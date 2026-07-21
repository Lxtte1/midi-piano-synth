#include <QResizeEvent>
#include <QPushButton>
#include <QKeyEvent>
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

        void pressKey(int key, double velocity = 1.0);
        void releaseKey(int key);

    protected:
        void resizeEvent(QResizeEvent* event) override;
        void keyPressEvent(QKeyEvent* event) override;
        void keyReleaseEvent(QKeyEvent* event) override;

    private:
        void redraw();

        AudioManager* audio;

        int whiteKeys = 0;
        std::vector<QPushButton*> keys;
        double defaultVelocity = 0.5;
};

#pragma once