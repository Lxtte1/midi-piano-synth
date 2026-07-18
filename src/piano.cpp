#include "piano.h"

#include <QPushButton>
#include <cstdio>

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