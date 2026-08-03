#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QWidget>
#include "piano.h"
#include <QTimer>
#include <vector>
#include <chrono>

static const QColor NOTE_COLOURS[12] =
{
    QColor("#ff5555"),
    QColor("#ff8844"),
    QColor("#ffaa33"),
    QColor("#ffdd33"),
    QColor("#aadd33"),
    QColor("#55dd55"),
    QColor("#33dddd"),
    QColor("#3399ff"),
    QColor("#6666ff"),
    QColor("#aa66ff"),
    QColor("#dd55dd"),
    QColor("#ff66aa")
};

class MidiPlayer : public QWidget {
    public:
        MidiPlayer(Piano& piano, QWidget* parent = nullptr);
        ~MidiPlayer();

        bool loadFile(QString path);
        void start();
        void stop();
        void resume();
        void pause();
        void setTraining(bool training = true);
        std::vector<int> getTrainingKeys();

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
            double velocity;
        };

    private:
        void advance();

        double speed = 200.0;
        bool playing = false;
        double time = 0.0;
        bool training = false;

        Piano* piano;
        std::vector<int> trainingKeys;
        std::vector<Note> notes;
        std::vector<Event> events;
        QGraphicsScene* scene;
        QGraphicsView* view;
        QTimer* runner = nullptr;
        std::chrono::system_clock::time_point startTime;
};

#pragma once