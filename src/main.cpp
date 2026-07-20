#include <QApplication>
#include <QVBoxLayout>
#include <QWidget>

#include "piano.h"
#include "audio.h"
#include "midi.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    
    QWidget window;
    window.resize(800, 600);
    window.setWindowTitle("Digital Piano");
    window.show();

    QVBoxLayout* layout = new QVBoxLayout(&window);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    
    AudioManager manager;
    
    Piano piano(manager, &window);
    piano.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    piano.show();
    layout->addWidget(&piano);
    
    MIDIInput midi(piano);

    return app.exec();
}