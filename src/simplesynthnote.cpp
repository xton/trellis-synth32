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
    AudioNoInterrupts();
    wav.amplitude(0.5);
    env.noteOn();
    AudioInterrupts();
}

void SimpleSynthNote::noteOff()
{
    AudioNoInterrupts();
    env.noteOff();
    // wav.amplitude(0); // let the envelope stop us since there's some decay here.
    AudioInterrupts();
}