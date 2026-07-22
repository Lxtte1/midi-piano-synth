#include <functional>
#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>
#include <QWidget>
#include <QSlider>

class Settings : public QWidget {
    public:
        Settings(QWidget* parent = nullptr);

        void onVolumeChange(std::function<void(double, double)> callback);
        void onKeyboardChange(std::function<void(bool, int, int)> callback);
        void onDeviceChange(std::function<void(int)> callback);

        void getDevicesCallback(std::function<QStringList()> call);

    private:
        void changeVolume();
        void changeKeyboard();
        void changeDevice();

        std::function<void(double, double)> volumeCallback;
        std::function<void(bool, int, int)> keyboardCallback;
        std::function<void(int)> deviceCallback;

        std::function<QStringList()> getDevices;

        QSlider* volume;
        QSlider* velocity;

        QCheckBox* labels;
        QSpinBox* keys;
        QSpinBox* octave;

        QComboBox* dropdown;
};

#pragma once