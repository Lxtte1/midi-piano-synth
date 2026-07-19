#include <spa/param/audio/format-utils.h>
#include <spa/interfaces/audio/aec.h>
#include <pipewire/pipewire.h>
#include "piano.h"

static void processPipewire(void* userdata) {
    printf("PipeWire process\n");
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
    }

    this->redraw();

    // Temporary
    // https://docs.pipewire.org/page_tutorial4.html
    struct Data {
        struct pw_main_loop* loop;
        struct pw_stream* stream;
        double accumulator;
    };

    static const struct pw_stream_events stream_events = {
        .version = PW_VERSION_STREAM_EVENTS,
        .process = processPipewire,
    };

    struct Data data = { 0, };
    const struct spa_pod* params[1];
    uint8_t buffer[1024];
    struct spa_pod_builder builder = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));

    pw_init(nullptr, nullptr);

    data.loop = pw_main_loop_new(NULL);
    pw_properties* properties = pw_properties_new(PW_KEY_MEDIA_TYPE, "Audio", PW_KEY_MEDIA_CATEGORY, "Playback", PW_KEY_MEDIA_ROLE, "Music", PW_KEY_APP_NAME, "Test", PW_KEY_APP_ICON_NAME, "addressbook", NULL);
    data.stream = pw_stream_new_simple(pw_main_loop_get_loop(data.loop), "audio-src", properties, &stream_events, &data);

    spa_audio_info_raw info = { .format = SPA_AUDIO_FORMAT_S16, .rate = 44100, .channels = 2 };
    params[0] = spa_format_audio_raw_build(&builder, SPA_PARAM_EnumFormat, &info);

    pw_stream_flags flags = static_cast<pw_stream_flags>(PW_STREAM_FLAG_AUTOCONNECT | PW_STREAM_FLAG_MAP_BUFFERS | PW_STREAM_FLAG_RT_PROCESS);
    pw_stream_connect(data.stream, PW_DIRECTION_OUTPUT, PW_ID_ANY, flags, params, 1);

    pw_main_loop_run(data.loop);
    pw_stream_destroy(data.stream);
    pw_main_loop_destroy(data.loop);
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