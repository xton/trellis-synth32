#pragma once
#include "inote.h"

class SimpleSynthNote : public INote
{
private:
    AudioSynthWaveform wav;
    AudioEffectEnvelope env;

    AudioConnection patchMF{wav, 0, env, 0};

    float baseFreq;

public:
    SimpleSynthNote();
    void begin() override;
    void noteOn() override;
    void noteOff() override;
    void enable() override;
    void disable() override;
    void setFrequency(float freq) override;
    AudioStream &getOutputLeft() override { return env; }
    AudioStream &getOutputRight() override { return env; }
};