#include <alsa/asoundlib.h>
#include <functional>
// #include <MidiFile.h>
#include "piano.h"
#include <vector>
#include <thread>

class MIDIInput {
    public:
        MIDIInput(Piano& piano);
        ~MIDIInput();

        const std::vector<snd_seq_client_info_t*> getClients();
        const QStringList getClientNames();

        void pickClient(int index);
        // void playFile(QString file);

    private:
        void run();

        Piano* piano;

        unsigned char port;
        std::thread runner;
        snd_seq_t* seq;
        bool running;
        // smf::MidiEventList events;
};

#pragma once