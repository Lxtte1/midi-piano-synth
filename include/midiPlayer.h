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

        struct Event {
            double time;
            bool active;
            int key;
        };

    private:
        void advance();

        double speed = 100.0;

        Piano* piano;
        std::vector<Note> notes;
        std::vector<Event> events;
        QGraphicsScene* scene;
        QGraphicsView* view;
        QTimer* runner = nullptr;
        std::chrono::_V2::system_clock::time_point startTime;
};

#pragma once