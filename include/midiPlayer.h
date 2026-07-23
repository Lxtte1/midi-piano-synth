#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QWidget>
#include "piano.h"
#include <QTimer>
#include <vector>
#include <chrono>

class MidiPlayer : public QWidget {
    public:
        MidiPlayer(Piano& piano, QWidget* parent = nullptr);
        ~MidiPlayer();

        void start();
        void stop();

        struct Note {
            double time;
            double duration;
            int key;

            QGraphicsRectItem* rect;
        };

    private:
        void advance();

        double speed = 100.0;

        Piano* piano;
        std::vector<Note> notes;
        QGraphicsScene* scene;
        QGraphicsView* view;
        QTimer* runner = nullptr;
        std::chrono::_V2::system_clock::time_point startTime;
};

#pragma once