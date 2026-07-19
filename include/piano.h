#include <QResizeEvent>
#include <QPushButton>
#include <QWidget>
#include "audio.h"
#include <vector>

const int NUM_KEYS = 25;
const bool KEY_COLOUR[] = { false, true, false, true, false, false, true, false, true, false, true, false };
const int START_OCTAVE = 3;
const char KEYS[] = { 'C', 'C', 'D', 'D', 'E', 'F', 'F', 'G', 'G', 'A', 'A', 'B' };

class Piano : public QWidget {
    public:
        Piano(AudioManager& audioManager, QWidget* parent = nullptr);

    protected:
        void resizeEvent(QResizeEvent* event) override;

    private:
        void redraw();

        AudioManager* audio;

        std::vector<QPushButton*> whiteKeys;
        std::vector<QPushButton*> blackKeys;
};

#pragma once