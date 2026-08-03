#include <QVBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QLabel>

#include "settings.h"

Settings::Settings(QWidget* parent) : QWidget(parent) {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setAlignment(Qt::AlignBottom);

    {
        QGridLayout* fileSettings = new QGridLayout(this);
        layout->addLayout(fileSettings);

        {
            QPushButton* button = new QPushButton(QString("Play File"), this);
            fileSettings->addWidget(button, 0, 0, 1, 2);

            QObject::connect(button, &QPushButton::pressed, this, [=]() {
                QString filename = QFileDialog::getOpenFileName(this, "Open MIDI file", QDir::homePath(), "MIDI Files (*.mid *.midi);;All Files (*)");
                if (!filename.isEmpty()) this->fileSelectCallback(filename);
            });
        }

        {
            QPushButton* button = new QPushButton(QString("Play"), this);
            fileSettings->addWidget(button, 1, 0);

            QObject::connect(button, &QPushButton::pressed, this, [=]() { this->filePlayCallback(true); });
        }

        {
            QPushButton* button = new QPushButton(QString("Pause"), this);
            fileSettings->addWidget(button, 1, 1);

            QObject::connect(button, &QPushButton::pressed, this, [=]() { this->filePlayCallback(false); });
        }
    }

    {
        QGridLayout* playerSettings = new QGridLayout(this);
        playerSettings->setAlignment(Qt::AlignLeft);
        layout->addLayout(playerSettings);

        {
            QCheckBox* box = new QCheckBox();
            playerSettings->addWidget(box, 0, 0);

            QLabel* label = new QLabel(QString("Training mode"), this);
            playerSettings->addWidget(label, 0, 1, Qt::AlignLeft);

            QObject::connect(box, &QCheckBox::stateChanged, this, [=]() { this->trainingCallback(box->isChecked()); });
        }
    }

    layout->addStretch(1);

    {
        QGridLayout* volumeSettings = new QGridLayout(this);
        layout->addLayout(volumeSettings);

        {
            QLabel* label = new QLabel(QString("Minimum\nVolume"), this);
            volumeSettings->addWidget(label, 0, 0);

            this->volume = new QSlider();
            this->volume->setMinimum(0);
            this->volume->setMaximum(100);
            this->volume->setSliderPosition(50);
            volumeSettings->addWidget(this->volume, 1, 0);

            QObject::connect(this->volume, &QSlider::valueChanged, this, &Settings::changeVolume);
        }

        {
            QLabel* label = new QLabel(QString("Minimum\nVelocity"), this);
            volumeSettings->addWidget(label, 0, 1);

            this->velocity = new QSlider();
            this->velocity->setMinimum(0);
            this->velocity->setMaximum(100);
            this->velocity->setSliderPosition(50);
            volumeSettings->addWidget(this->velocity, 1, 1);

            QObject::connect(this->velocity, &QSlider::valueChanged, this, &Settings::changeVolume);
        }
    }

    {
        QGridLayout* metaSettings = new QGridLayout(this);
        layout->addLayout(metaSettings);
        
        {
            QLabel* label = new QLabel(QString("Labels"), this);
            metaSettings->addWidget(label, 0, 0);

            this->labels = new QCheckBox();
            this->labels->setChecked(true);
            metaSettings->addWidget(this->labels, 1, 0);

            QObject::connect(this->labels, &QCheckBox::checkStateChanged, this, &Settings::changeKeyboard);
        }

        {
            QLabel* label = new QLabel(QString("Keys"), this);
            metaSettings->addWidget(label, 0, 1);

            this->keys = new QSpinBox();
            this->keys->setRange(0, 88);
            this->keys->setValue(25);
            metaSettings->addWidget(this->keys, 1, 1);

            QObject::connect(this->keys, &QSpinBox::valueChanged, this, &Settings::changeKeyboard);
        }

        {
            QLabel* label = new QLabel(QString("Octave"), this);
            metaSettings->addWidget(label, 0, 2);

            this->octave = new QSpinBox();
            this->octave->setRange(-1, 8);
            this->octave->setValue(3);
            metaSettings->addWidget(this->octave, 1, 2);

            QObject::connect(this->octave, &QSpinBox::valueChanged, this, &Settings::changeKeyboard);
        }
    }

    // {
    //     QGridLayout* deviceSettings = new QGridLayout(this);
    //     layout->addLayout(deviceSettings);

    //     QLabel* label = new QLabel(QString("Input Device"), this);
    //     deviceSettings->addWidget(label, 0, 0, 1, 2);

    //     this->dropdown = new QComboBox(this);
    //     deviceSettings->addWidget(this->dropdown, 1, 0);

    //     QObject::connect(this->dropdown, &QComboBox::currentIndexChanged, this, &Settings::changeDevice);

    //     QPushButton* refresh = new QPushButton("", this);
    //     refresh->setFixedWidth(refresh->height());
    //     deviceSettings->addWidget(refresh, 1, 1);

    //     QObject::connect(refresh, &QPushButton::pressed, [=]() {
    //         this->dropdown->clear();
    //         if (this->getDevices != nullptr) this->dropdown->addItems(this->getDevices());
    //     });
    // }
}

void Settings::onVolumeChange(std::function<void(double, double)> callback) {
    this->volumeCallback = callback;
}

void Settings::onKeyboardChange(std::function<void(bool, int, int)> callback) {
    this->keyboardCallback = callback;
}

void Settings::onDeviceChange(std::function<void(int)> callback) {
    this->deviceCallback = callback;
}

void Settings::onFileSelect(std::function<void(QString)> callback) {
    this->fileSelectCallback = callback;
}

void Settings::onFilePlay(std::function<void(bool)> callback) {
    this->filePlayCallback = callback;
}

void Settings::onTraningChange(std::function<void(bool)> callback) {
    this->trainingCallback = callback;
}

void Settings::getDevicesCallback(std::function<QStringList()> call) {
    this->getDevices = call;

    // this->dropdown->clear();
    // this->dropdown->addItems(this->getDevices());
}

void Settings::changeVolume() {
    if (this->volumeCallback == nullptr) return;
    this->volumeCallback(this->volume->value() / (double)this->volume->maximum(), this->velocity->value() / (double)this->velocity->maximum());
}

void Settings::changeKeyboard() {
    if (this->keyboardCallback == nullptr) return;
    this->keyboardCallback(this->labels->isChecked(), this->keys->value(), this->octave->value());
}

void Settings::changeDevice() {
    if (this->deviceCallback == nullptr) return;
    this->deviceCallback(this->dropdown->currentIndex());
}