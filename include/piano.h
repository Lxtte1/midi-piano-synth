#include <QResizeEvent>
#include <QPushButton>
#include <QKeyEvent>
#include <QWidget>
#include "audio.h"
#include <vector>

const bool KEY_COLOUR[] = { false, true, false, true, false, false, true, false, true, false, true, false };
const char KEYS[] = { 'C', 'C', 'D', 'D', 'E', 'F', 'F', 'G', 'G', 'A', 'A', 'B' };

class Piano : public QWidget {
    public:
        Piano(AudioManager& audioManager, QWidget* parent = nullptr);
        Piano(AudioManager&, int numKeys, int octave, QWidget* parent = nullptr);
        void initiate(int numKeys, int octave);

        void pressKey(int key, double velocity = 1.0);
        void releaseKey(int key);
        void setDefaultVelocity(double velocity);

        bool showLabels = true;

    protected:
        void resizeEvent(QResizeEvent* event) override;
        void keyPressEvent(QKeyEvent* event) override;
        void keyReleaseEvent(QKeyEvent* event) override;

    private:
        void redraw();

        AudioManager* audio;

        int whiteKeys = 0;
        int octave;
        std::vector<QPushButton*> keys;
        double defaultVelocity = 0.5;
};

#pragma once