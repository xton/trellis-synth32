#pragma once
#include "inote.h"

class GuitarNote : public INote
{
private:
    AudioSynthKarplusStrong note;

    // Initial attack waveform for the pluck
    // AudioSynthWaveform pluck;

    // A pair of slightly detuned sawtooths for sustained string tone
    // AudioSynthWaveform string1;
    // AudioSynthWaveform string2;
    // AudioSynthKarplusStrong string1;
    // AudioSynthKarplusStrong string2;

    // // Mixer to combine the pluck and string tones
    // AudioMixer4 mixer;

    // // Filter to shape the tone
    // AudioFilterStateVariable filter;

    // // Envelope for overall amplitude shaping
    // AudioEffectEnvelope env;

    // // Audio routing
    // // AudioConnection patchPluck{pluck, 0, mixer, 0};
    // AudioConnection patchStr1{string1, 0, mixer, 1};
    // AudioConnection patchStr2{string2, 0, mixer, 2};
    // AudioConnection patchMixFilt{mixer, 0, filter, 0};
    // AudioConnection patchFiltEnv{filter, 0, env, 0};

    float baseFreq;
    // const float detune = 0.02f; // 2 cents detune between strings

public:
    GuitarNote();
    void begin() override;
    void noteOn() override;
    void noteOff() override;
    void setFrequency(float freq) override;
    AudioStream &getOutputLeft() override { return note; }
    AudioStream &getOutputRight() override { return note; }
};