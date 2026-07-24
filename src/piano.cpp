#include <QVBoxLayout>
#include <QLabel>

#include "piano.h"

Piano::Piano(AudioManager& audioManager, QWidget* parent) : Piano(audioManager, 25, 3, parent) {}

Piano::Piano(AudioManager& audioManager, int numKeys, int octave, QWidget* parent) : QWidget(parent), audio(&audioManager) {
    this->initiate(numKeys, octave);
}

void Piano::pressKey(int key, double velocity) {
    this->audio->playNote(key, this->defaultVelocity + velocity * (1 - this->defaultVelocity));

    int index = key - (this->octave + 1) * 12;
    if (index < 0 || index >= this->keys.size()) return;
    QPushButton* button = this->keys[index];
    button->setDown(true);
}

void Piano::releaseKey(int key) {
    this->audio->stopNote(key);

    int index = key - (this->octave + 1) * 12;
    if (index < 0 || index >= this->keys.size()) return;
    QPushButton* button = this->keys[index];
    button->setDown(false);
}

void Piano::setDefaultVelocity(double velocity) {
    this->defaultVelocity = velocity;
}

int Piano::getOctave() {
    return this->octave;
}

int Piano::getWhiteKeysCount() {
    return this->whiteKeys;
}

bool Piano::isReady() {
    return this->initiated;
}

const int Piano::codeToKey(int code) {
    return ((int)(code / 12) - 1) * (KEY_COLOUR[code % 12] ? 5 : 7) + KEYS_BEFORE[code % 12];
}

const int Piano::codeToColourIndex(int code) {
    return Piano::codeToKey(code) - this->octave * (KEY_COLOUR[code % 12] ? 5 : 7);
}

void Piano::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    this->redraw();
}

void Piano::keyPressEvent(QKeyEvent* event) {
    if (event->isAutoRepeat() || event->key() < 48 || event->key() > 57) return;
    this->pressKey(event->key());
}

void Piano::keyReleaseEvent(QKeyEvent* event) {
    if (event->isAutoRepeat() || event->key() < 48 || event->key() > 57) return;
    this->releaseKey(event->key());
}

void Piano::initiate(int numKeys, int octave) {
    this->initiated = false;

    this->setMinimumSize(numKeys * 50, 400);
    this->octave = octave;
    this->whiteKeys = 0;

    while (this->keys.size() > 0) {
        delete this->keys[0];
        this->keys.erase(this->keys.begin());
    }

    for (unsigned int i = 0; i < numKeys; i++) {
        QPushButton* button = new QPushButton("", this);
        button->setStyleSheet(QString("QPushButton { background-color: %1; border: none; %2; }").arg(KEY_COLOUR[i % 12] ? "black" : "white").arg(i < numKeys - 1 ? "border-right: 2px solid black;" : "")
        .append(" QPushButton:hover { background-color: %1; }").arg(KEY_COLOUR[i % 12] ? "#111" : "#ddd")
        .append(" QPushButton:pressed { background-color: %1; }").arg(KEY_COLOUR[i % 12] ? "#222" : "#bbb"));

        if (this->showLabels) {
            QVBoxLayout* layout = new QVBoxLayout(button);
            layout->addStretch();

            QLabel* label = new QLabel(QString("%1\%2%3").arg(KEYS[i % 12]).arg(KEY_COLOUR[i % 12] ? "#" : "").arg(octave + (int)(i / 12)));
            label->setAlignment(Qt::AlignCenter | Qt::AlignBottom);
            label->setStyleSheet(QString("color: %1;").arg(KEY_COLOUR[i % 12] ? "white" : "black"));
            layout->addWidget(label);
        }

        this->keys.push_back(button);
        if (!KEY_COLOUR[i % 12]) this->whiteKeys++;

        QObject::connect(button, &QPushButton::pressed, [=]() {
            this->pressKey(i + (octave + 1) * 12);
        });

        QObject::connect(button, &QPushButton::released, [=]() {
            this->releaseKey(i + (octave + 1) * 12);
        });
    }

    this->redraw();
    this->initiated = true;
}

void Piano::redraw() {
    unsigned int whiteKeyWidth = this->width() / this->whiteKeys;
    unsigned int blackKeyWidth = whiteKeyWidth / 2;

    unsigned int whiteIndex = 0;
    unsigned int blackIndex = 0;

    for (unsigned int i = 0; i < this->keys.size(); i++) {
        QPushButton* button = this->keys[i];

        if (KEY_COLOUR[i % 12]) {
            button->setGeometry(whiteIndex * whiteKeyWidth - blackKeyWidth / 2, 0, blackKeyWidth, this->height() * 0.55);
            button->setMaximumHeight(400);
            button->raise();

            blackIndex++;
        } else {
            button->setGeometry(whiteIndex * whiteKeyWidth, 0, (i == this->keys.size() - 1 ? this->width() - whiteKeyWidth * whiteIndex : whiteKeyWidth), this->height());
            
            whiteIndex++;
        }

        button->show();
    }
}