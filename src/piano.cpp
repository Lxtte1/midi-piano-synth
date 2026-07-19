#include <spa/param/audio/format-utils.h>
#include <thread>

#include "piano.h"

static void processPipewire(void* userdata) {
    struct Data* data = (Data*)userdata;
    struct pw_buffer* b;
    struct spa_buffer* buf;
    int i, c, n_frames, stride;
    int16_t* dst, val;

    if ((b = pw_stream_dequeue_buffer(data->stream)) == NULL) {
        pw_log_warn("Out of buffers: %m");
        return;
    }

    buf = b->buffer;
    if ((dst = (int16_t*)buf->datas[0].data) == NULL) return;

    stride = sizeof(int16_t) * 2;
    n_frames = buf->datas[0].maxsize / stride;
    if (b->requested) n_frames = SPA_MIN(b->requested, n_frames);

    for (unsigned int i = 0; i < n_frames; i++) {
        data->accumulator += 6.2832 * data->frequency / 44100; // 2 * pi * 440 / 44100
        if (data->accumulator >= 6.2832) data->accumulator -= 6.2832; // 2 * pi

        val = sin(data->accumulator) * 32767.0;
        for (c = 0; c < 2; c++) *dst++ = val * 0.5;
    }

    buf->datas[0].chunk->offset = 0;
    buf->datas[0].chunk->stride = stride;
    buf->datas[0].chunk->size = n_frames * stride;

    pw_stream_queue_buffer(data->stream, b);
}

static void runPipewire(pw_main_loop* loop) {
    printf("Hello!\n");
    pw_main_loop_run(loop);
}

Piano::Piano(QWidget* parent) : QWidget(parent) {
    this->setMinimumSize(NUM_KEYS * 50, 400);

    for (unsigned int i = 0; i < NUM_KEYS; i++) {
        QPushButton* button = new QPushButton("", this);
        button->setStyleSheet(QString("QPushButton { background-color: %1; border: none; %2; }").arg(KEY_COLOUR[i % 12] ? "black" : "white").arg(i < NUM_KEYS - 1 ? "border-right: 2px solid black;" : "")
        .append(" QPushButton:hover { background-color: %1; }").arg(KEY_COLOUR[i % 12] ? "#111" : "#ddd")
        .append(" QPushButton:pressed { background-color: %1; }").arg(KEY_COLOUR[i % 12] ? "#222" : "#bbb"));

        if (KEY_COLOUR[i % 12]) this->blackKeys.push_back(button);
        else this->whiteKeys.push_back(button);

        QObject::connect(button, &QPushButton::pressed, [=]() {
            this->data.frequency = 100 * i;
        });
    }

    this->redraw();

    // Temporary
    // https://docs.pipewire.org/page_tutorial4.html
    static const struct pw_stream_events stream_events = {
        .version = PW_VERSION_STREAM_EVENTS,
        .process = processPipewire,
    };

    this->data = { nullptr, nullptr, 0, 600 };
    const struct spa_pod* params[1];
    uint8_t buffer[1024];
    struct spa_pod_builder builder = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));

    pw_init(nullptr, nullptr);

    this->data.loop = pw_main_loop_new(NULL);
    pw_properties* properties = pw_properties_new(PW_KEY_MEDIA_TYPE, "Audio", PW_KEY_MEDIA_CATEGORY, "Playback", PW_KEY_MEDIA_ROLE, "Music", PW_KEY_APP_NAME, "Test", PW_KEY_APP_ICON_NAME, "addressbook", NULL);
    this->data.stream = pw_stream_new_simple(pw_main_loop_get_loop(this->data.loop), "audio-src", properties, &stream_events, &this->data);

    spa_audio_info_raw info = { .format = SPA_AUDIO_FORMAT_S16, .rate = 44100, .channels = 2 };
    params[0] = spa_format_audio_raw_build(&builder, SPA_PARAM_EnumFormat, &info);

    pw_stream_flags flags = static_cast<pw_stream_flags>(PW_STREAM_FLAG_AUTOCONNECT | PW_STREAM_FLAG_MAP_BUFFERS | PW_STREAM_FLAG_RT_PROCESS);
    pw_stream_connect(this->data.stream, PW_DIRECTION_OUTPUT, PW_ID_ANY, flags, params, 1);

    this->pipewire = std::thread(&runPipewire, this->data.loop);

    // pw_main_loop_run(this->data.loop);
    // pw_stream_destroy(this->data.stream);
    // pw_main_loop_destroy(this->data.loop);
}

Piano::~Piano() {
    pw_main_loop_quit(this->data.loop);

    if (this->pipewire.joinable())
        this->pipewire.join();
}

void Piano::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    this->redraw();
}

void Piano::redraw() {
    unsigned int whiteKeyWidth = this->width() / this->whiteKeys.size();
    unsigned int blackKeyWidth = whiteKeyWidth / 2;

    unsigned int whiteIndex = 0;
    unsigned int blackIndex = 0;

    for (unsigned int i = 0; i < NUM_KEYS; i++) {
        if (KEY_COLOUR[i % 12]) {
            QPushButton* button = this->blackKeys[blackIndex];
            button->setGeometry(whiteIndex * whiteKeyWidth - blackKeyWidth / 2, 0, blackKeyWidth, this->height() * 0.55);
            button->setMaximumHeight(400);
            button->raise();

            blackIndex++;
        } else {
            QPushButton* button = this->whiteKeys[whiteIndex];
            button->setGeometry(whiteIndex * whiteKeyWidth, 0, whiteKeyWidth, this->height());
            
            whiteIndex++;
        }
    }
}
