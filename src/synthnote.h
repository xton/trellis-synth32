#pragma once
#include "inote.h"

class SynthNote : public INote
{
private:
    AudioSynthWaveform waveform1;
    AudioSynthWaveform waveform2; // For detuning
    AudioMixer4 waveMixer;        // Combine waveforms
    AudioFilterStateVariable filter;
    AudioEffectEnvelope env;

    // Audio routing
    AudioConnection patchW1{waveform1, 0, waveMixer, 0};
    AudioConnection patchW2{waveform2, 0, waveMixer, 1};

    AudioConnection patchE{waveMixer, 0, env, 0};

    // AudioConnection patchMF{waveMixer, 0, filter, 0};
    // AudioConnection patchFE{filter, 0, env, 0};

    float baseFreq;
    float detune;

public:
    SynthNote();
    void begin() override;
    void noteOn() override;
    void noteOff() override;
    void setFrequency(float freq) override;
    AudioStream &getOutputLeft() override { return env; }
    AudioStream &getOutputRight() override { return env; }
};