#include <QVBoxLayout>

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
}

MidiPlayer::~MidiPlayer() {
}

void MidiPlayer::start() {
    this->stop();

    // Temporary (Mary had a little lamb)
    this->notes.push_back({ 0.0, 1.0, 62 });
    this->notes.push_back({ 1.0, 1.0, 61 });
    this->notes.push_back({ 2.0, 1.0, 60 });
    this->notes.push_back({ 3.0, 1.0, 61 });
    this->notes.push_back({ 4.0, 1.0, 62 });
    this->notes.push_back({ 5.0, 1.0, 62 });
    this->notes.push_back({ 6.0, 1.0, 62 });
    this->notes.push_back({ 7.0, 1.0, 61 });
    this->notes.push_back({ 8.0, 1.0, 61 });
    this->notes.push_back({ 9.0, 1.0, 61 });
    this->notes.push_back({ 10.0, 1.0, 62 });
    this->notes.push_back({ 11.0, 1.0, 64 });
    this->notes.push_back({ 12.0, 1.0, 64 });
    this->notes.push_back({ 13.0, 1.0, 62 });
    this->notes.push_back({ 14.0, 1.0, 61 });
    this->notes.push_back({ 15.0, 1.0, 60 });
    this->notes.push_back({ 16.0, 1.0, 61 });
    this->notes.push_back({ 17.0, 1.0, 62 });
    this->notes.push_back({ 18.0, 1.0, 62 });
    this->notes.push_back({ 19.0, 1.0, 62 });
    this->notes.push_back({ 20.0, 1.0, 62 });
    this->notes.push_back({ 21.0, 1.0, 62 });
    this->notes.push_back({ 22.0, 1.0, 62 });
    this->notes.push_back({ 23.0, 1.0, 61 });
    this->notes.push_back({ 24.0, 1.0, 61 });
    this->notes.push_back({ 25.0, 1.0, 60 });

    // int startKey = (this->piano->getOctave() + 1) * 12;
    // double width = this->width() / this->piano->getWhiteKeysCount();

    for (unsigned int i = 0; i < this->notes.size(); i++) {
        Note& note = this->notes[i];

        QGraphicsRectItem* item = new QGraphicsRectItem(100, 100, 100, 100);
        item->setBrush(i % 2 == 0 ? Qt::blue : Qt::red);
        item->setPen(Qt::NoPen);

        // QWidget* rect = new QWidget(this);
        // rect->setStyleSheet("background-color: blue; border: none");
        // rect->setGeometry((note.key - startKey) * width, this->height() - (note.time * speed) - note.duration * this->speed, width, note.duration * this->speed);
        // rect->show();

        this->scene->addItem(item);
        note.rect = item;
    }

    this->runner = new QTimer(this);
    QObject::connect(this->runner, &QTimer::timeout, this, &MidiPlayer::advance);

    this->startTime = std::chrono::high_resolution_clock::now();
    this->runner->start(16);
}

void MidiPlayer::stop() {
    if (this->runner && this->runner->isActive()) this->runner->stop();
}

void MidiPlayer::advance() {
    auto now = std::chrono::high_resolution_clock::now();
    double delta = std::chrono::duration<double>(now - this->startTime).count();
    this->scene->setSceneRect(0, 0, this->width(), this->height());

    if (!this->piano->isReady() || this->piano->getWhiteKeysCount() <= 0) return;

    int startKey = (this->piano->getOctave() + 1) * 12;
    double width = this->width() / this->piano->getWhiteKeysCount();

    for (unsigned int i = 0; i < this->notes.size(); i++) {
        Note& note = this->notes[i];
        QGraphicsRectItem* rect = note.rect;
        if (rect == nullptr) continue;

        int y = this->height() - (note.time * this->speed) - note.duration * this->speed + delta * this->speed;

        if (y > this->height()) {;
            this->scene->removeItem(rect);
            delete rect;

            this->notes.erase(this->notes.begin() + i);
            i--;
        } else rect->setRect((note.key - startKey) * width, y, width, note.duration * this->speed);
    }

    if (this->notes.size() <= 0) this->stop();
}