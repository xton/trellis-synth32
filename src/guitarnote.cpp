#include "guitarnote.h"

GuitarNote::GuitarNote() : baseFreq(0) {}

void GuitarNote::begin() {}

void GuitarNote::setFrequency(float freq) { baseFreq = freq; }

void GuitarNote::noteOn() { note.noteOn(baseFreq, 0.7); }

void GuitarNote::noteOff() { note.noteOff(0.5); }

void GuitarNote::enable() {}

void GuitarNote::disable() { note.noteOff(0); }
