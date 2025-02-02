#pragma once
#include <Arduino.h>
#include <Audio.h>

#define NUM_VOICES 8
#define MAX_NOTES 32 // Total number of keys on NeoTrellis

class Voice
{
public:
    bool active;
    int noteNumber;
    uint32_t startTime;

    Voice() : active(false), noteNumber(-1), startTime(0) {}
};

class PolySynth
{
private:
    // Static allocation of all audio components
    AudioSynthWaveform waves[NUM_VOICES];
    AudioEffectEnvelope envs[NUM_VOICES];
    Voice voices[NUM_VOICES];
    AudioMixer4 mixer1;     // First layer mixer (voices 0-3)
    AudioMixer4 mixer2;     // First layer mixer (voices 4-7)
    AudioMixer4 mixerFinal; // Final output mixer

    // Static allocation of all audio connections
    AudioConnection patchCord0{waves[0], envs[0]};
    AudioConnection patchCord1{waves[1], envs[1]};
    AudioConnection patchCord2{waves[2], envs[2]};
    AudioConnection patchCord3{waves[3], envs[3]};
    AudioConnection patchCord4{waves[4], envs[4]};
    AudioConnection patchCord5{waves[5], envs[5]};
    AudioConnection patchCord6{waves[6], envs[6]};
    AudioConnection patchCord7{waves[7], envs[7]};

    AudioConnection patchCord8{envs[0], 0, mixer1, 0};
    AudioConnection patchCord9{envs[1], 0, mixer1, 1};
    AudioConnection patchCord10{envs[2], 0, mixer1, 2};
    AudioConnection patchCord11{envs[3], 0, mixer1, 3};
    AudioConnection patchCord12{envs[4], 0, mixer2, 0};
    AudioConnection patchCord13{envs[5], 0, mixer2, 1};
    AudioConnection patchCord14{envs[6], 0, mixer2, 2};
    AudioConnection patchCord15{envs[7], 0, mixer2, 3};

    AudioConnection patchCord16{mixer1, 0, mixerFinal, 0};
    AudioConnection patchCord17{mixer2, 0, mixerFinal, 1};

    int findOldestVoice() const;
    int findVoiceForNote(int noteNumber) const;
    int findFreeVoice() const;

public:
    PolySynth() {} // Empty constructor since we're using static allocation
    void begin();
    void noteOn(int noteNumber, float frequency, int waveform);
    void noteOff(int noteNumber);
    AudioMixer4 &getOutputMixer() { return mixerFinal; }
};
