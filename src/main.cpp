#include <QApplication>
#include <QGridLayout>
#include <QWidget>

#include "midiPlayer.h"
#include "settings.h"
#include "piano.h"
#include "audio.h"
#include "midi.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    
    QWidget window;
    window.setWindowTitle("Digital Piano");
    window.show();

    QGridLayout* mainLayout = new QGridLayout(&window);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setVerticalSpacing(0);
    mainLayout->setHorizontalSpacing(5);

    
    Settings settings(&window);
    settings.show();
    mainLayout->addWidget(&settings, 0, 0, 2, 1);

    AudioManager manager;
    Piano piano(manager, &window);
    piano.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    piano.show();
    mainLayout->addWidget(&piano, 1, 1);

    settings.onKeyboardChange([&piano](bool labels, int keys, int octave) {
        piano.showLabels = labels;
        piano.initiate(keys, octave);
    });

    settings.onVolumeChange([&piano, &manager](double volume, double velocity) {
        piano.setDefaultVelocity(velocity);
        manager.setMaximumVolume(volume);
    });
    
    
    MIDIInput midi(piano);
    settings.getDevicesCallback([&midi]() { return midi.getClientNames(); });
    settings.onDeviceChange([&midi](int index) { midi.pickClient(index); });

    MidiPlayer player(piano, &window);
    mainLayout->addWidget(&player, 0, 1);
    player.start();

    // midi.playFile("temp/music.mid");
    
    window.resize(mainLayout->minimumSize());
    return app.exec();
}