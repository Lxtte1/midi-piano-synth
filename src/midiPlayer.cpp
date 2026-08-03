#include <QLinearGradient>
#include <QVBoxLayout>
#include <MidiFile.h>
#include <algorithm>
#include <map>

#include "midiPlayer.h"

MidiPlayer::MidiPlayer(Piano& piano, QWidget* parent) : QWidget(parent), piano(&piano) {
    this->setStyleSheet("background-color: white; border-bottom: 2px solid black");
    this->setMinimumHeight(400);
    this->show();

    
    this->scene = new QGraphicsScene(this);
    this->scene->setSceneRect(0, 0, this->piano->width(), 400);
    this->scene->setBackgroundBrush(Qt::white);

    this->view = new QGraphicsView(this->scene, this);
    this->view->setScene(this->scene);
    this->view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->view->show();

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 2);
    layout->addWidget(this->view);

    piano.onKeyPressed([=](int key) {
    });
}

MidiPlayer::~MidiPlayer() {
}

bool MidiPlayer::loadFile(QString path) {
    smf::MidiFile file;
    file.read(path.toStdString());
    file.doTimeAnalysis();
    file.linkNotePairs();

    this->events.clear();

    for (unsigned int i = 0; i < file.getTrackCount(); i++)
        for (unsigned int j = 0; j < file[i].getEventCount(); j++) {
            smf::MidiEvent event = file[i][j];

            if (event.isNote()) this->events.push_back({ event.seconds, event.isNoteOn(), event.getKeyNumber(), event.getVelocity() / 127.0 });
        }

    return true;
}

void MidiPlayer::start() {
    this->stop();
    if (this->events.size() <= 0) return;

    std::map<int, Note> key;
    for (unsigned int i = 0; i < this->events.size(); i++) {
        Event& event = this->events[i];

        if (event.active) key[event.key] = {event.time, 0.0, event.key};
        else {
            key[event.key].duration = event.time - key[event.key].time;
            this->notes.push_back(key[event.key]);
        }
    }

    for (unsigned int i = 0; i < this->notes.size(); i++) {
        Note& note = this->notes[i];

        QGraphicsRectItem* item = new QGraphicsRectItem(100, 100, 100, 100);
        item->setBrush(NOTE_COLOURS[note.key % 12]);
        item->setPen(Qt::NoPen);

        this->scene->addItem(item);
        note.rect = item;
    }

    this->runner = new QTimer(this);
    QObject::connect(this->runner, &QTimer::timeout, this, &MidiPlayer::advance);

    this->startTime = std::chrono::high_resolution_clock::now();
    this->playing = true;
    this->time = -1.0;
    this->runner->start(16);
    this->trainingKeys.clear();
}

void MidiPlayer::stop() {
    if (this->runner && this->runner->isActive()) this->runner->stop();
    this->playing = false;

    while (this->notes.size() > 0) {
        this->scene->removeItem(this->notes[0].rect);
        delete this->notes[0].rect;
        this->notes.erase(this->notes.begin());
    }
}

void MidiPlayer::resume() {
    this->playing = true;
}

void MidiPlayer::pause() {
    this->playing = false;
}

void MidiPlayer::setTraining(bool training) {
    this->training = training;
}

std::vector<int> MidiPlayer::getTrainingKeys() {
    return this->trainingKeys;
}

void MidiPlayer::advance() {
    std::chrono::system_clock::time_point now = std::chrono::high_resolution_clock::now();
    double delta = std::chrono::duration<double>(now - this->startTime).count();
    this->startTime = now;
    this->scene->setSceneRect(0, 0, this->width(), this->height());
    if (this->playing) this->time += delta;

    if (!this->piano->isReady() || this->piano->getWhiteKeysCount() <= 0) return;

    double width = this->width() / this->piano->getWhiteKeysCount();

    for (unsigned int i = 0; i < this->notes.size(); i++) {
        Note& note = this->notes[i];
        QGraphicsRectItem* rect = note.rect;
        if (rect == nullptr) continue;
        int noteIndex = KEY_COLOUR[note.key % 12] ? this->piano->codeToColourIndex(note.key - 1) : this->piano->codeToColourIndex(note.key);

        int y = this->height() - (note.time * this->speed) - note.duration * this->speed + this->time * this->speed;

        if (y > this->height()) {
            this->scene->removeItem(rect);
            delete rect;

            this->notes.erase(this->notes.begin() + i);
            i--;
        } else
            if (KEY_COLOUR[note.key % 12]) rect->setRect((noteIndex + 0.75) * width, y, width / 2, note.duration * this->speed);
            else rect->setRect(noteIndex * width, y, width, note.duration * this->speed);
    }

    for (unsigned int i = 0; i < this->events.size(); i++) {
        Event& event = this->events[i];
        if (this->time < event.time) continue;

        if (this->training)
            if (event.active) { this->pause(); this->trainingKeys.push_back(event.key); }
            else { auto it = std::find(this->trainingKeys.begin(), this->trainingKeys.end(), event.key); if (it != this->trainingKeys.end()) this->trainingKeys.erase(it); } 
        else
            if (event.active) this->piano->pressKey(event.key, event.velocity);
            else this->piano->releaseKey(event.key);

        this->events.erase(this->events.begin() + i);
        i--;
    }

    if (this->training) {
        std::vector<int> pressedKeys = this->piano->getPressedKeys();
        bool allKeysPressed = true;

        for (unsigned int i = 0; i < this->trainingKeys.size(); i++) {
            int key = this->trainingKeys[i];
            auto it = std::find(pressedKeys.begin(), pressedKeys.end(), key);
            if (it == pressedKeys.end()) {
                allKeysPressed = false;
                break;
            }
        }

        if (allKeysPressed && pressedKeys.size() >= this->trainingKeys.size()) this->resume();
    }

    if (this->notes.size() <= 0 && this->events.size() <= 0) this->stop();
}