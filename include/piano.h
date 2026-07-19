#include <pipewire/pipewire.h>
#include <QResizeEvent>
#include <QPushButton>
#include <QWidget>
#include <vector>
#include <thread>

const int NUM_KEYS = 25;
const bool KEY_COLOUR[] = { false, true, false, true, false, false, true, false, true, false, true, false };

struct Data {
    struct pw_main_loop* loop;
    struct pw_stream* stream;
    double accumulator;

    double frequency;
};

static void processPipewire(void* userdata);
static void runPipewire(pw_main_loop* loop);

class Piano : public QWidget {
    public:
        Piano(QWidget* parent = nullptr);
        ~Piano();

    protected:
        void resizeEvent(QResizeEvent* event) override;

    private:
        void redraw();

        Data data;
        std::thread pipewire;
        std::vector<QPushButton*> whiteKeys;
        std::vector<QPushButton*> blackKeys;
};

#pragma once