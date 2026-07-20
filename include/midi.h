#include <alsa/asoundlib.h>
#include "piano.h"
#include <vector>
#include <thread>

class MIDIInput {
    public:
        MIDIInput(Piano& piano);
        ~MIDIInput();

        const std::vector<snd_seq_client_info_t*> getClients();

    private:
        void run();

        Piano* piano;

        std::thread runner;
        snd_seq_t* seq;
        bool running;
};

#pragma once