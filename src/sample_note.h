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

public:
    SampleNote() {}

    /// @brief override this in subclasses to set the sample!
    virtual void begin() override {}
    void noteOn() override
    {
        player.setSpeed(baseFreq / referenceFreq);
        player.play(buffer, buffer_len);
    }
    void noteOff() { player.stop(); }

    void enable() override {}
    void disable() override { player.stop(); }

    void setFrequency(float freq) override { baseFreq = freq; }
    void setSample(const int16_t *new_buffer, size_t new_buffer_len, float new_referenceFreq)
    {
        buffer = new_buffer;
        buffer_len = new_buffer_len;
        referenceFreq = new_referenceFreq;
    }

    AudioStream &getOutputLeft() override { return player; }
    AudioStream &getOutputRight() override { return player; }
};