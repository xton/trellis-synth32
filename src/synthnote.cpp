#include "synthnote.h"

SynthNote::SynthNote() : baseFreq(0), detune(0.1)
{
    // Set up wave mixer
    waveMixer.gain(0, 0.5); // Main oscillator
    waveMixer.gain(1, 0.5); // Detuned oscillator

    // Set up filter
    filter.frequency(1000);
    filter.resonance(0.7);
    filter.octaveControl(2.0);
}

void SynthNote::begin()
{
    // Set envelope parameters
    env.attack(9.2);
    env.hold(2.1);
    env.decay(31.4);
    env.sustain(0.6);
    env.release(85.5);

    // Start with silent oscillators
    waveform1.begin(0.0, baseFreq, WAVEFORM_SAWTOOTH);
    waveform2.begin(0.0, baseFreq * (1.0 + detune), WAVEFORM_SAWTOOTH);
}

void SynthNote::setFrequency(float freq)
{
    baseFreq = freq;
    waveform1.frequency(freq);
    waveform2.frequency(freq * (1.0 + detune));
}

void SynthNote::noteOn()
{
    env.noteOn();
}

void SynthNote::noteOff()
{
    env.noteOff();
}

void SynthNote::enable()
{
    AudioNoInterrupts();
    waveform1.amplitude(0.5);
    waveform2.amplitude(0.5);
    AudioInterrupts();
}

void SynthNote::disable()
{
    noteOff();
    AudioNoInterrupts();
    waveform1.amplitude(0);
    waveform2.amplitude(0);
    AudioInterrupts();
}
