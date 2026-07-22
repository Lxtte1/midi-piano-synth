#include <QApplication>
#include <QHBoxLayout>
#include <QWidget>

#include "settings.h"
#include "piano.h"
#include "audio.h"
#include "midi.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    
    QWidget window;
    window.resize(800, 600);
    window.setWindowTitle("Digital Piano");
    window.show();

    QHBoxLayout* mainLayout = new QHBoxLayout(&window);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(5);
    
    Settings settings(&window);
    settings.show();
    mainLayout->addWidget(&settings);

    AudioManager manager;
    Piano piano(manager, &window);
    piano.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    piano.show();
    mainLayout->addWidget(&piano);

    settings.onKeyboardChange([&piano](bool labels, int keys, int octave) {
        piano.showLabels = labels;
        piano.initiate(keys, octave);
    });

    settings.onVolumeChange([&piano, &manager](double volume, double velocity) {
        piano.setDefaultVelocity(velocity);
        manager.setMaximumVolume(volume);
    });
    
    
    MIDIInput midi(piano);

    settings.getDevicesCallback([&midi]() {
        return midi.getClientNames();
    });

    settings.onDeviceChange([&midi](int index) {
        midi.pickClient(index);
    });
    
    return app.exec();
}