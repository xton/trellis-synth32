#include "guitarnote.h"

GuitarNote::GuitarNote() : baseFreq(0)
{
    // Configure mixer levels
    // mixer.gain(0, 0.5); // Pluck level
    // mixer.gain(1, 0.3); // String 1 level
    // mixer.gain(2, 0.3); // String 2 level

    // // Configure filter
    // filter.frequency(1200);  // Initial cutoff
    // filter.resonance(0.7);   // Moderate resonance
    // filter.octaveControl(7); // Allow wide frequency range
}

void GuitarNote::begin()
{
    // // Set envelope for pluck shape
    // env.attack(1.0); // Very fast attack
    // env.hold(2.0);
    // env.decay(100.0);   // Medium decay
    // env.sustain(0.3);   // Moderate sustain
    // env.release(300.0); // Long release
}

void GuitarNote::setFrequency(float freq)
{
    baseFreq = freq;

    // // Pluck uses the base frequency
    // pluck.frequency(freq);

    // // Strings are slightly detuned
    // string1.frequency(freq * (1.0 - detune));
    // string2.frequency(freq * (1.0 + detune));

    // Set filter cutoff relative to note frequency
    // filter.frequency(freq * 4);
}

void GuitarNote::noteOn()
{
    AudioNoInterrupts();

    // Sharp attack for pluck
    note.noteOn(baseFreq, 0.7);
    // string2.noteOn(baseFreq * (1.0 + detune), 0.5);

    // pluck.amplitude(0.8);
    // string1.amplitude(0.4);
    // string2.amplitude(0.4);

    // Higher filter frequency at attack
    // filter.frequency(baseFreq * 8);

    // env.noteOn();
    AudioInterrupts();
}

void GuitarNote::noteOff()
{
    AudioNoInterrupts();
    note.noteOff(0.5);

    // // pluck.amplitude(0);
    // // string1.amplitude(0);
    // // string2.amplitude(0);
    // string1.noteOff(0.5);
    // string2.noteOff(0.5);
    // env.noteOff();
    AudioInterrupts();
}