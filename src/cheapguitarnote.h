#pragma once
#include "inote.h"
#include "debug.h"

class CheapGuitarNote : public INote
{
private:
    AudioSynthWaveform waveform; // Main oscillator

    // AudioSynthWaveform lfo;      // Low frequency oscillator for modulation
    // AudioFilterStateVariable filter;

    AudioFilterBiquad filter; // Simpler IIR filter
    AudioEffectEnvelope envelope;

    // Audio connections as fixed members
    AudioConnection patchCord1{waveform, 0, filter, 0};
    // AudioConnection patchCord2{lfo, 0, filter, 1}; // Modulate filter
    AudioConnection patchCord3{filter, 0, envelope, 0};

    // AudioConnection patchCord1{waveform, 0, envelope, 0};

    float baseFreq;

public:
    CheapGuitarNote();
    void begin() override;
    void noteOn() override;
    void noteOff() override;
    void setFrequency(float freq) override;
    AudioStream &getOutputLeft() override { return envelope; }
    AudioStream &getOutputRight() override { return envelope; }
};