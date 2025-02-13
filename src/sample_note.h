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
    AudioEffectEnvelope env;
    AudioConnection patch{player, env};

public:
    SampleNote() {}

    virtual void begin() override
    {
        env.attack(10.0);
        env.hold(0.0);
        env.decay(0.0);
        env.sustain(1.0);
        env.release(200.0);
    }

    void noteOn() override
    {
        AudioNoInterrupts();
        player.setSpeed(baseFreq / referenceFreq);
        player.play(buffer, buffer_len);
        env.noteOn();
        AudioInterrupts();
    }
    // void noteOff() { player.stop(); }
    void noteOff()
    {
        env.noteOff();
    }

    void enable() override {}
    void disable() override
    {
        player.stop();
        env.noteOff();
    }

    void setFrequency(float freq) override { baseFreq = freq; }
    void setSample(const int16_t *new_buffer, size_t new_buffer_len, float new_referenceFreq)
    {
        buffer = new_buffer;
        buffer_len = new_buffer_len;
        referenceFreq = new_referenceFreq;
    }

    AudioStream &getOutputLeft() override { return env; }
    AudioStream &getOutputRight() override { return env; }
};