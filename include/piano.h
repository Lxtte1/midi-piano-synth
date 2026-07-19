#include <QResizeEvent>
#include <QPushButton>
#include <QWidget>
#include <vector>

const int NUM_KEYS = 25;
const bool KEY_COLOUR[] = { false, true, false, true, false, false, true, false, true, false, true, false };

static void processPipewire(void* userdata);

class Piano : public QWidget {
    public:
        Piano(QWidget* parent = nullptr);

    protected:
        void resizeEvent(QResizeEvent* event) override;

    private:
        void redraw();

        std::vector<QPushButton*> whiteKeys;
        std::vector<QPushButton*> blackKeys;
};

#pragma once