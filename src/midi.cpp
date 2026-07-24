#include <alsa/asoundlib.h>
#include <cstdio>
#include <chrono>

#include "midi.h"

MIDIInput::MIDIInput(Piano& piano) {
    this->piano = &piano;

    snd_seq_open(&this->seq, "default", SND_SEQ_OPEN_INPUT, 1);
    this->port = (unsigned char)snd_seq_create_simple_port(this->seq, "Input", SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE, SND_SEQ_PORT_TYPE_MIDI_GENERIC);

    this->pickClient(0);
}

MIDIInput::~MIDIInput() {
    this->running = false;

    if (this->runner.joinable()) this->runner.join();
}

const std::vector<snd_seq_client_info_t*> MIDIInput::getClients() {
    snd_seq_client_info_t* clientInfo;
    snd_seq_client_info_alloca(&clientInfo);
    snd_seq_client_info_set_client(clientInfo, -1);

    std::vector<snd_seq_client_info_t*> clients;

    while (snd_seq_query_next_client(this->seq, clientInfo) >= 0) {
        int id = snd_seq_client_info_get_client(clientInfo);
        int type = snd_seq_client_info_get_type(clientInfo);
        int card = snd_seq_client_info_get_card(clientInfo);

        if (type == -1 || card != 4) continue;

        snd_seq_client_info_t* client;
        snd_seq_client_info_malloc(&client);
        snd_seq_client_info_copy(client, clientInfo);

        clients.push_back(client);
    }

    return clients;
}

const QStringList MIDIInput::getClientNames() {
    const std::vector<snd_seq_client_info_t*> clients = this->getClients();
    QStringList names(clients.size());

    for (unsigned int i = 0; i < clients.size(); i++) {
        names[i] = "Client " + i;
    }

    return names;
}

void MIDIInput::pickClient(int index) {
    this->running = false;
    if (this->runner.joinable()) this->runner.join();

    std::vector<snd_seq_client_info_t*> clients = this->getClients();
    if (clients.size() <= 0) return;
    snd_seq_client_info_t* client = clients[index];
    
    snd_seq_addr_t sender{ (unsigned char)snd_seq_client_info_get_client(client), this->port };
    snd_seq_addr_t destination{ (unsigned char)snd_seq_client_id(this->seq), this->port };

    snd_seq_port_subscribe_t* subscriber;
    snd_seq_port_subscribe_alloca(&subscriber);
    snd_seq_port_subscribe_set_sender(subscriber, &sender);
    snd_seq_port_subscribe_set_dest(subscriber, &destination);

    snd_seq_subscribe_port(this->seq, subscriber);

    this->runner = std::thread(&MIDIInput::run, this);
}

void MIDIInput::run() {
    this->running = true;

    while (this->running) {
        snd_seq_event_t* event = nullptr;
        if (snd_seq_event_input(this->seq, &event) <= 0) continue;
        
        switch (event->type) {
            case SND_SEQ_EVENT_NOTEON:
                this->piano->pressKey(event->data.note.note, event->data.note.velocity / 127.0);
                break;
            case SND_SEQ_EVENT_NOTEOFF:
                this->piano->releaseKey(event->data.note.note);
                break;
        }
    }
}