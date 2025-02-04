#include "polysynth32.h"

template <typename T>
SynthRow<T>::SynthRow()
    // First layer: connect notes 0-3 to mixLeft1/mixRight1, notes 4-7 to mixLeft2/mixRight2
    : patchL1_0(notes[0].getOutputLeft(), 0, mixLeft1, 0), patchL1_1(notes[1].getOutputLeft(), 0, mixLeft1, 1), patchL1_2(notes[2].getOutputLeft(), 0, mixLeft1, 2), patchL1_3(notes[3].getOutputLeft(), 0, mixLeft1, 3), patchL2_0(notes[4].getOutputLeft(), 0, mixLeft2, 0), patchL2_1(notes[5].getOutputLeft(), 0, mixLeft2, 1), patchL2_2(notes[6].getOutputLeft(), 0, mixLeft2, 2), patchL2_3(notes[7].getOutputLeft(), 0, mixLeft2, 3), patchR1_0(notes[0].getOutputRight(), 0, mixRight1, 0), patchR1_1(notes[1].getOutputRight(), 0, mixRight1, 1), patchR1_2(notes[2].getOutputRight(), 0, mixRight1, 2), patchR1_3(notes[3].getOutputRight(), 0, mixRight1, 3), patchR2_0(notes[4].getOutputRight(), 0, mixRight2, 0), patchR2_1(notes[5].getOutputRight(), 0, mixRight2, 1), patchR2_2(notes[6].getOutputRight(), 0, mixRight2, 2), patchR2_3(notes[7].getOutputRight(), 0, mixRight2, 3)
      // Second layer: connect first layer mixers to final mixers
      ,
      patchLF1(mixLeft1, 0, finalLeft, 0), patchLF2(mixLeft2, 0, finalLeft, 1), patchRF1(mixRight1, 0, finalRight, 0), patchRF2(mixRight2, 0, finalRight, 1)
{
    // First layer mixers: 1.0 gain for each input
    // (will be divided among active voices)
    for (int i = 0; i < 4; i++)
    {
        mixLeft1.gain(i, 1.0);
        mixLeft2.gain(i, 1.0);
        mixRight1.gain(i, 1.0);
        mixRight2.gain(i, 1.0);
    }

    // Final mixers: 1.0 gain for each input
    finalLeft.gain(0, 1.0);
    finalLeft.gain(1, 1.0);
    finalRight.gain(0, 1.0);
    finalRight.gain(1, 1.0);
}

template <typename T>
void SynthRow<T>::noteOn(int index)
{
    if (index >= 0 && index < NOTES_PER_ROW)
    {
        notes[index].noteOn();
    }
}

template <typename T>
void SynthRow<T>::noteOff(int index)
{
    if (index >= 0 && index < NOTES_PER_ROW)
    {
        notes[index].noteOff();
    }
}

template <typename T>
void SynthRow<T>::begin()
{
    for (int i = 0; i < NOTES_PER_ROW; i++)
    {
        notes[i].begin();
    }
}

template <typename T>
void SynthRow<T>::setScale(const float *frequencies)
{
    for (int i = 0; i < NOTES_PER_ROW; i++)
    {
        notes[i].setFrequency(frequencies[i]);
    }
}

Polysynth32::Polysynth32()
{
    // Set mixer gains for final output
    for (int i = 0; i < 4; i++)
    {
        finalMixLeft.gain(i, 1.0);
        finalMixRight.gain(i, 1.0);
    }
}

void Polysynth32::setupScales()
{
    // C major pentatonic base frequencies for each row (C2, C3, C4, C5)
    constexpr float C2 = 65.41;  // C2
    constexpr float C3 = 130.81; // C3
    constexpr float C4 = 261.63; // C4
    constexpr float C5 = 523.25; // C5

    // Major pentatonic intervals: root, major second, major third, perfect fifth, major sixth
    // Then repeat first three notes up an octave
    constexpr int pentatonic[] = {0, 2, 4, 7, 9, 12, 14, 16};

    // Setup each row
    for (int i = 0; i < 8; i++)
    {
        scaleRow1[i] = makeNoteFreq(C2, pentatonic[i]); // C2 pentatonic
        scaleRow2[i] = makeNoteFreq(C3, pentatonic[i]); // C3 pentatonic
        scaleRow3[i] = makeNoteFreq(C4, pentatonic[i]); // C4 pentatonic
        scaleRow4[i] = makeNoteFreq(C5, pentatonic[i]); // C5 pentatonic
    }
}

void Polysynth32::begin()
{
    setupScales();

    // Set scales for each row
    synthRow1.setScale(scaleRow1);
    synthRow2.setScale(scaleRow2);
    synthRow3.setScale(scaleRow3);
    synthRow4.setScale(scaleRow4);

    // Initialize synth rows
    synthRow1.begin();
    synthRow2.begin();
    synthRow3.begin();
    synthRow4.begin();

    // Set up limiters with fast attack, medium release
    limiterLeft.threshold(-12.0); // Start limiting at -12 dB
    limiterLeft.attack(0.01);     // 10ms attack
    limiterLeft.release(60.0);    // 60ms release
    limiterLeft.ratio(4.0);       // 4:1 compression ratio

    limiterRight.threshold(-12.0);
    limiterRight.attack(0.01);
    limiterRight.release(60.0);
    limiterRight.ratio(4.0);
}

void Polysynth32::noteOn(int noteIndex)
{
    if (noteIndex < 0 || noteIndex >= 32)
        return;

    int row = noteIndex / 8;
    int note = noteIndex % 8;

    switch (row)
    {
    case 0:
        synthRow1.noteOn(note);
        break;
    case 1:
        synthRow2.noteOn(note);
        break;
    case 2:
        synthRow3.noteOn(note);
        break;
    case 3:
        synthRow4.noteOn(note);
        break;
    }
}

void Polysynth32::noteOff(int noteIndex)
{
    if (noteIndex < 0 || noteIndex >= 32)
        return;

    int row = noteIndex / 8;
    int note = noteIndex % 8;

    switch (row)
    {
    case 0:
        synthRow1.noteOff(note);
        break;
    case 1:
        synthRow2.noteOff(note);
        break;
    case 2:
        synthRow3.noteOff(note);
        break;
    case 3:
        synthRow4.noteOff(note);
        break;
    }
}