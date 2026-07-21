#include <spa/param/audio/format-utils.h>

#include "audio.h"

AudioManager::AudioManager() {
    static const struct pw_stream_events stream_events = {
        .version = PW_VERSION_STREAM_EVENTS,
        .process = this->process
    };

    pw_init(nullptr, nullptr);
    
    this->data = { nullptr, nullptr, 0.0, 0.0, std::map<int, Note>() };
    this->data.loop = pw_main_loop_new(NULL);
    pw_properties* properties = pw_properties_new(PW_KEY_MEDIA_TYPE, "Audio", PW_KEY_MEDIA_CATEGORY, "Playback", PW_KEY_MEDIA_ROLE, "Music",
                                                  PW_KEY_APP_NAME, "Digital Piano", PW_KEY_APP_ICON_NAME, "keyboard", NULL);
    this->data.stream = pw_stream_new_simple(pw_main_loop_get_loop(this->data.loop), "audio-src", properties, &stream_events, &this->data);

    const struct spa_pod* params[1];
    unsigned char buffer[1024];
    struct spa_pod_builder builder = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));
    spa_audio_info_raw info = { .format = SPA_AUDIO_FORMAT_F32, .rate = this->rate, .channels = this->channels };
    params[0] = spa_format_audio_raw_build(&builder, SPA_PARAM_EnumFormat, &info);

    pw_stream_flags flags = static_cast<pw_stream_flags>(PW_STREAM_FLAG_AUTOCONNECT | PW_STREAM_FLAG_MAP_BUFFERS | PW_STREAM_FLAG_RT_PROCESS);
    pw_stream_connect(this->data.stream, PW_DIRECTION_OUTPUT, PW_ID_ANY, flags, params, 1);

    this->runner = std::thread(&this->run, &this->data);
}

AudioManager::~AudioManager() {
    pw_main_loop_quit(this->data.loop);
    pw_stream_destroy(this->data.stream);

    if (this->runner.joinable()) this->runner.join();
}

const double AudioManager::getFrequency(int key) {
    return 440.0 * pow(2.0, (key - 69) / 12.0);
}

const float AudioManager::getAmplitude(Note& note) {
    double value = 0.0;
    
    for (unsigned int i = 0; i < note.phases.size(); i++) {
        double frequency = note.frequency * (i + 1);
        double damping = 0.15 + 0.35 * frequency / 4000.0;
        double amplitude = pow(i + 1, -1.25) * exp(-damping * (i + 1));
        amplitude *= AudioManager::envelope(note, i + 1) * 0.5 * note.velocity;

        note.phases[i] += frequency / AudioManager::rate;
        if (note.phases[i] > 1) note.phases[i] -= 1;

        value += amplitude * sin(note.phases[i] * 2 * M_PI);
    }

    return value;
}

const double AudioManager::envelope(double age) {
    double attack = 1.0 - exp(-80.0 * age);
    double decay = exp(-3.0 * age);

    return attack * decay;
}

const double AudioManager::envelope(double age, double releasedAt) {
    double attack = 1.0 - exp(-80.0 * age);
    double decay = exp(-3.0 * age);
    double release = exp(-7.0 * (age - releasedAt));

    return attack * decay * release;
}

const double AudioManager::envelope(Note& note, int decayRate) {
    // Config
    const double attackTime = 0.004; // 4 ms
    const double attackCoefficient = 3;
    const double decayTime = 10; // 10 s
    const double decayCoefficient = 12;
    const double releaseTime = 0.12; // 120 ms

    double value = 1;

    if (note.age < attackTime) value = (1 - exp(-attackCoefficient * note.age / attackTime)) / (1 - exp(-attackCoefficient));
    else value = (exp(-decayCoefficient * (note.age - attackTime) * decayRate / decayTime) - exp(-decayCoefficient)) / (1 - exp(-decayCoefficient));

    if (!note.active) value *= 1 - (note.age - note.released) / releaseTime;

    return std::max(value, 0.0);
}

const double AudioManager::amplitudeToDecibles(float sample) {
    return 20.0 * log10(sample);
}

const double AudioManager::signedAmplitudeToDecibles(float sample) {
    return 10.0 * log10(sample * sample);
}

const float AudioManager::deciblesToAmplitude(double db) {
    return pow(10.0, db / 20.0);
}

const double AudioManager::applyVolumeCurve(float input, double threshold, double smoothWidth, double ratio) {
    double start = threshold - smoothWidth / 2;
    double excess = input - start;

    if (excess <= 0) return input;
    if (excess >= smoothWidth) return threshold + (input - threshold) / ratio;

    double end = threshold + smoothWidth / (2 * ratio);
    double smoothT = excess / smoothWidth;

    return AudioManager::quadraticBezier1D(start, threshold, end, smoothT);
}

const double AudioManager::quadraticBezier1D(double a, double b, double c, double t) {
    return t * t * (a - 2 * b + c) + t * 2 * (b - a) + a;
}

void AudioManager::compress(float* samples, double& targetReduction, double& smoothReduction, int frames) {
    // Config
    const double dbMin = -80;
    const double threshold = -12.97596;
    const double smoothWidth = 17.9;
    const double ratio = 3.4;

    for (unsigned int i = 0; i < frames * AudioManager::channels; i++) {
        if (std::abs(samples[i]) <= 0.0001) continue;

        double inputDB = AudioManager::signedAmplitudeToDecibles(samples[i]);
        double reductionDB = inputDB - AudioManager::applyVolumeCurve(inputDB, threshold, smoothWidth, ratio);

        targetReduction = std::max(targetReduction, std::max(0.0, reductionDB));
        targetReduction -= targetReduction * (1 / AudioManager::rate) * 100;
        smoothReduction += (targetReduction - smoothReduction) * (1 / AudioManager::rate) * 100;

        double outputDB = inputDB - smoothReduction;
        if (samples[i] > 0) samples[i] = AudioManager::deciblesToAmplitude(outputDB);
        else samples[i] = -AudioManager::deciblesToAmplitude(outputDB); 
    }
}

void AudioManager::playNote(int note, double velocity) {
    this->data.notes[note] = { this->getFrequency(note), velocity, std::vector<double>(AudioManager::harmonics), 0.0, true, 0.0 };
    
    this->cleanupNotes();
}

void AudioManager::stopNote(int note) {
    Note& _note = this->data.notes[note];
    _note.active = false;
    _note.released = _note.age;
    
    this->cleanupNotes();
}


void AudioManager::process(void* userdata) {
    struct Data* data = (Data*)userdata;
    struct pw_buffer* b = pw_stream_dequeue_buffer(data->stream);
    struct spa_buffer* buffer = b->buffer;
    
    float* dst = (float*)buffer->datas[0].data;

    int stride = sizeof(float) * AudioManager::channels;
    int frames = buffer->datas[0].maxsize / stride;
    if (b->requested) frames = SPA_MIN(b->requested, frames);

    for (unsigned int i = 0; i < frames; i++) {
        float sample = 0.0;

        for (auto j = data->notes.begin(); j != data->notes.end(); j++) {
            Note& note = j->second;
            float value = AudioManager::getAmplitude(note);

            sample += value * 0.5;
            note.age += 1.0 / AudioManager::rate;
        };

        for (unsigned int c = 0; c < AudioManager::channels; c++) dst[i * AudioManager::channels + c] = sample; // * 32767.0;
    }

    AudioManager::compress(dst, data->targetReduction, data->smoothReduction, frames);

    buffer->datas[0].chunk->offset = 0;
    buffer->datas[0].chunk->stride = stride;
    buffer->datas[0].chunk->size = frames * stride;

    pw_stream_queue_buffer(data->stream, b);

    for (auto i = data->notes.begin(); i != data->notes.end();)
        if (!i->second.active && (i->second.age - i->second.released) > 0.2) i = data->notes.erase(i);
        else i++;
}

void AudioManager::run(Data* data) {
    pw_main_loop_run(data->loop);
}

void AudioManager::cleanupNotes() {
    // for (auto i = this->data.notes.begin(); i != this->data.notes.end();)
    //     if (i->second.age >= 1) this->data.notes.erase(i);
    //     else i++;
}