#pragma once
#include "inote.h"
#include "debug.h"

class GuitarNote : public INote
{
private:
    AudioSynthKarplusStrong note;
    float baseFreq;

public:
    GuitarNote();
    void begin() override;
    void noteOn() override;
    void noteOff() override;
    void setFrequency(float freq) override;
    AudioStream &getOutputLeft() override { return note; }
    AudioStream &getOutputRight() override { return note; }
};