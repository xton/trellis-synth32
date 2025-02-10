#include "cheapguitarnote.h"

CheapGuitarNote::CheapGuitarNote() : baseFreq(440) {}

void CheapGuitarNote::begin()
{

    waveform.begin(WAVEFORM_SAWTOOTH);
    waveform.amplitude(0);
    waveform.frequency(baseFreq);

    // // Set up LFO for subtle pitch/filter modulation
    // lfo.begin(WAVEFORM_SINE);
    // lfo.frequency(6.0);  // 6 Hz modulation
    // lfo.amplitude(0.02); // Subtle amount

    // Configure filter with modulation input
    // filter.frequency(baseFreq * 2);
    // filter.resonance(0.7);
    // filter.octaveControl(2);
    filter.setLowpass(0, 1000, 0.7);

    // Set up envelope for pluck shape
    envelope.attack(0);
    envelope.decay(100);
    envelope.sustain(0.3);
    envelope.release(200);
}

void CheapGuitarNote::setFrequency(float freq)
{
    baseFreq = freq;
    AudioNoInterrupts();
    waveform.frequency(baseFreq);
    // // Increase modulation rate for higher notes
    // lfo.frequency(6.0 + (baseFreq / 440.0) * 2.0);
    // filter.frequency(baseFreq * 2);
    filter.setLowpass(0, freq * 2, 0.7); // Adjust filter with note
    AudioInterrupts();
}

void CheapGuitarNote::noteOn() { envelope.noteOn(); }

void CheapGuitarNote::noteOff() { envelope.noteOff(); }

void CheapGuitarNote::enable() { waveform.amplitude(0.5); }

void CheapGuitarNote::disable()
{
    noteOff();
    waveform.amplitude(0);
}
