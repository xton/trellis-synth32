#pragma once
#include "inote.h"
#include "debug.h"
#include "play_memory_variable.h"

class SampleNote : public INote
{
private:
    float baseFreq;

    const int16_t *buffer;
    size_t buffer_len;
    float referenceFreq;

    AudioPlayPlayMemoryVariable player;
    AudioMixer4 finalMix;
    AudioConnection patch{player, finalMix};

public:
    SampleNote() {}

    virtual void begin() override
    {
        setGain(1.0);
    }

    void noteOn() override
    {
        AudioNoInterrupts();
        player.setSpeed(baseFreq / referenceFreq);
        player.play(buffer, buffer_len);
        AudioInterrupts();
    }
    // void noteOff(){ player.stop(); }
    // presently we're letting the whole sample play always
    void noteOff() {}

    void enable() override {}
    void disable() override
    {
        player.stop();
    }

    void setFrequency(float freq) override { baseFreq = freq; }
    void setSample(const int16_t *new_buffer, size_t new_buffer_len, float new_referenceFreq)
    {
        buffer = new_buffer;
        buffer_len = new_buffer_len;
        referenceFreq = new_referenceFreq;
    }
    void setGain(float g) { finalMix.gain(0, g); }

    AudioStream &getOutputLeft() override
    {
        return finalMix;
    }
    AudioStream &getOutputRight() override { return finalMix; }
};