#include <spa/param/audio/format-utils.h>

#include "audio.h"

AudioManager::AudioManager() {
    static const struct pw_stream_events stream_events = {
        .version = PW_VERSION_STREAM_EVENTS,
        .process = this->process
    };

    pw_init(nullptr, nullptr);
    
    this->data = { nullptr, nullptr, 0.0, this->getFrequency(48), 0.0 };
    this->data.loop = pw_main_loop_new(NULL);
    pw_properties* properties = pw_properties_new(PW_KEY_MEDIA_TYPE, "Audio", PW_KEY_MEDIA_CATEGORY, "Playback", PW_KEY_MEDIA_ROLE, "Music",
                                                  PW_KEY_APP_NAME, "Digital Piano", PW_KEY_APP_ICON_NAME, "keyboard", NULL);
    this->data.stream = pw_stream_new_simple(pw_main_loop_get_loop(this->data.loop), "audio-src", properties, &stream_events, &this->data);

    const struct spa_pod* params[1];
    unsigned char buffer[1024];
    struct spa_pod_builder builder = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));
    spa_audio_info_raw info = { .format = SPA_AUDIO_FORMAT_S16, .rate = this->rate, .channels = this->channels };
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

const double AudioManager::harmonics(double phase, int n) {
    double stretch = sqrt(1.0 + 0.0004 * n * n);
    return sin(phase * n * stretch);
}

const double AudioManager::samplePhase(double phase) {
    // return ( sin(phase)
    //        + sin(phase * 2.0) * 0.5
    //        + sin(phase * 3.0) * 0.25
    //        + sin(phase * 4.0) * 0.125
    //        + sin(phase * 5.0) * 0.0625 ) / 1.9375;
    return (AudioManager::harmonics(phase, 1)
            + AudioManager::harmonics(phase, 2) * 0.5
            + AudioManager::harmonics(phase, 3) * 0.25
            + AudioManager::harmonics(phase, 4) * 0.125
            + AudioManager::harmonics(phase, 5) * 0.0625) / 1.9375;
}

const double AudioManager::envelope(double age) {
    double attack = 1.0 - exp(-80.0 * age);
    double decay = exp(-3.0 * age);

    return attack * decay;
}

void AudioManager::setTone(double tone) {
    this->data.phase = 0.0;
    this->data.frequency = tone;
    this->data.age = 0.0;
}

void AudioManager::stop() {
    this->data.frequency = 0.0;
}

void AudioManager::process(void* userdata) {
    struct Data* data = (Data*)userdata;
    struct pw_buffer* b = pw_stream_dequeue_buffer(data->stream);
    struct spa_buffer* buffer = b->buffer;
    
    short* dst = (short*)buffer->datas[0].data;

    int stride = sizeof(short) * 2;
    int frames = buffer->datas[0].maxsize / stride;
    if (b->requested) frames = SPA_MIN(b->requested, frames);

    for (unsigned int i = 0; i < frames; i++) {
        data->phase += 2 * M_PI * data->frequency / AudioManager::rate;
        if (data->phase >= 2 * M_PI) data->phase -= 2 * M_PI;

        double value = AudioManager::samplePhase(data->phase) * AudioManager::envelope(data->age);
        short sample = (short)(value * 32767.0 * 0.5);

        for (unsigned int c = 0; c < AudioManager::channels; c++) *dst++ = sample;

        data->age += 1.0 / AudioManager::rate;
    }

    buffer->datas[0].chunk->offset = 0;
    buffer->datas[0].chunk->stride = stride;
    buffer->datas[0].chunk->size = frames * stride;

    pw_stream_queue_buffer(data->stream, b);
}

void AudioManager::run(Data* data) {
    pw_main_loop_run(data->loop);
}