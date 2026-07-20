#include <pipewire/pipewire.h>
#include <thread>
#include <map>

struct Note {
    double frequency;
    double phase;
    double age;

    bool active;
    double released;
};

struct Data {
    struct pw_main_loop* loop;
    struct pw_stream* stream;

    double targetReduction;
    double smoothReduction;
    std::map<int, Note> notes;
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
        static const double envelope(double age, double releasedAt);
        static const double amplitudeToDecibles(float sample);
        static const double signedAmplitudeToDecibles(float sample);
        static const float deciblesToAmplitude(double db);
        static const double applyVolumeCurve(float input, double threshold, double smoothWidth, double ratio);
        static const double quadraticBezier1D(double a, double b, double c, double t);

        static void compress(float* samples, double& targetReduction, double& smoothReduction, int frames);

        void playNote(int note);
        void stopNote(int note = -1);
    private:
        static void process(void* userdata);
        static void run(Data* data);

        std::thread runner;
        Data data;
};

#pragma once