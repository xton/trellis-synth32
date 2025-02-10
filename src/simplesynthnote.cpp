#include "simplesynthnote.h"

SimpleSynthNote::SimpleSynthNote() : baseFreq(0) {}

void SimpleSynthNote::begin()
{
    // Set envelope parameters
    env.attack(9.2);
    env.hold(2.1);
    env.decay(31.4);
    env.sustain(0.6);
    env.release(84.5);

    // Start with silent oscillators
    wav.begin(0.0, baseFreq, WAVEFORM_TRIANGLE);
}

void SimpleSynthNote::setFrequency(float freq)
{
    baseFreq = freq;
    wav.frequency(freq);
}

void SimpleSynthNote::noteOn()
{
    env.noteOn();
}

void SimpleSynthNote::noteOff()
{
    env.noteOff();
}

void SimpleSynthNote::enable()
{
    wav.amplitude(0.5);
}

void SimpleSynthNote::disable()
{
    noteOff();
    wav.amplitude(0);
}
