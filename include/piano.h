#include <QResizeEvent>
#include <QPushButton>
#include <functional>
#include <QKeyEvent>
#include <QWidget>
#include "audio.h"
#include <vector>

static const bool KEY_COLOUR[] = { false, true, false, true, false, false, true, false, true, false, true, false };
static const char KEYS[] = { 'C', 'C', 'D', 'D', 'E', 'F', 'F', 'G', 'G', 'A', 'A', 'B' };
static const int KEYS_BEFORE[] = { 0, 0, 1, 1, 2, 3, 2, 4, 3, 5, 4, 6 };

class Piano : public QWidget {
    public:
        Piano(AudioManager& audioManager, QWidget* parent = nullptr);
        Piano(AudioManager&, int numKeys, int octave, QWidget* parent = nullptr);
        void initiate(int numKeys, int octave);

        void pressKey(int key, double velocity = 1.0);
        void releaseKey(int key);
        void setDefaultVelocity(double velocity);
        int getOctave();
        int getWhiteKeysCount();
        bool isReady();
        static const int codeToKey(int code);
        const int codeToColourIndex(int code);
        std::vector<int> getPressedKeys();

        void onKeyPressed(std::function<void(int)> callback);
        void onKeyReleased(std::function<void(int)> callback);

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
        bool initiated = false;
        std::vector<int> pressedKeys;
        std::vector<std::function<void(int)>> keyPressCallbacks;
        std::vector<std::function<void(int)>> keyReleaseCallbacks;
};

#pragma once