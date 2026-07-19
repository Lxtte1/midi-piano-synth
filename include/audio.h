#include <pipewire/pipewire.h>
#include <thread>

struct Data {
    struct pw_main_loop* loop;
    struct pw_stream* stream;

    double phase;
    double frequency;
    double age;
};

class AudioManager {
    public:
        AudioManager();
        ~AudioManager();

        static const int rate = 44100;
        static const int channels = 2;

        static const double getFrequency(int key);
        static const double harmonics(double phase, int n);
        static const double samplePhase(double phase);
        static const double envelope(double age);

        void setTone(double tone);
        void stop();
    private:
        static void process(void* userdata);
        static void run(Data* data);

        std::thread runner;
        Data data;
};

#pragma once