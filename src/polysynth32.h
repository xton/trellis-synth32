#pragma once
#include <Arduino.h>
#include <Audio.h>
#include "inote.h"
#include "synthnote.h"
#include "guitarnote.h"
#include "effect_dynamics.h"

// Will need to include other note types when we create them
// #include "guitarnote.h"
// #include "drumnote.h"

// Forward declaration for the row template
template <typename T>
class SynthRow;

// Each row manages 8 notes of the same type
template <typename T>
class SynthRow
{
    // static_assert(std::is_base_of<INote, T>::value, "T must implement INote interface");

private:
    static const int NOTES_PER_ROW = 8;
    T notes[NOTES_PER_ROW];
    AudioMixer4 mixLeft1;   // Mixes notes 0-3
    AudioMixer4 mixLeft2;   // Mixes notes 4-7
    AudioMixer4 finalLeft;  // Combines left mixers
    AudioMixer4 mixRight1;  // Mixes notes 0-3
    AudioMixer4 mixRight2;  // Mixes notes 4-7
    AudioMixer4 finalRight; // Combines right mixers

    // Audio connections for note outputs to first layer mixers
    AudioConnection patchL1_0;
    AudioConnection patchL1_1;
    AudioConnection patchL1_2;
    AudioConnection patchL1_3;
    AudioConnection patchL2_0;
    AudioConnection patchL2_1;
    AudioConnection patchL2_2;
    AudioConnection patchL2_3;
    AudioConnection patchR1_0;
    AudioConnection patchR1_1;
    AudioConnection patchR1_2;
    AudioConnection patchR1_3;
    AudioConnection patchR2_0;
    AudioConnection patchR2_1;
    AudioConnection patchR2_2;
    AudioConnection patchR2_3;

    // Audio connections for mixer routing
    AudioConnection patchLF1;
    AudioConnection patchLF2;
    AudioConnection patchRF1;
    AudioConnection patchRF2;

public:
    SynthRow();
    void begin();
    void noteOn(int index);
    void noteOff(int index);
    AudioStream &getOutputLeft() { return finalLeft; }
    AudioStream &getOutputRight() { return finalRight; }
    void setScale(const float *frequencies);
};

class Polysynth32
{
private:
    // Note: GuitarNote and DrumNote still need to be implemented
    SynthRow<GuitarNote> synthRow1; // Row 0: Guitar sounds
    SynthRow<SynthNote> synthRow2;  // Row 1: Synth sounds
    SynthRow<SynthNote> synthRow3;  // Row 2: More synth sounds for now
    SynthRow<SynthNote> synthRow4;  // Row 3: More synth sounds for now

    // Final mixing and limiting stage
    AudioMixer4 finalMixLeft;
    AudioMixer4 finalMixRight;
    AudioEffectDynamics limiterLeft;
    AudioEffectDynamics limiterRight;

    // Audio connections for final mix stage
    AudioConnection patchL1{synthRow1.getOutputLeft(), 0, finalMixLeft, 0};
    AudioConnection patchL2{synthRow2.getOutputLeft(), 0, finalMixLeft, 1};
    AudioConnection patchL3{synthRow3.getOutputLeft(), 0, finalMixLeft, 2};
    AudioConnection patchL4{synthRow4.getOutputLeft(), 0, finalMixLeft, 3};

    AudioConnection patchR1{synthRow1.getOutputRight(), 0, finalMixRight, 0};
    AudioConnection patchR2{synthRow2.getOutputRight(), 0, finalMixRight, 1};
    AudioConnection patchR3{synthRow3.getOutputRight(), 0, finalMixRight, 2};
    AudioConnection patchR4{synthRow4.getOutputRight(), 0, finalMixRight, 3};

    // Limiter connections
    AudioConnection patchLimL{finalMixLeft, 0, limiterLeft, 0};
    AudioConnection patchLimR{finalMixRight, 0, limiterRight, 0};

    // Scale configuration
    void setupScales();
    static constexpr float makeNoteFreq(float baseFreq, int semitones)
    {
        return baseFreq * pow(2.0f, semitones / 12.0f);
    }

    // Frequency tables for each row (8 notes per row)
    float scaleRow1[8];
    float scaleRow2[8];
    float scaleRow3[8];
    float scaleRow4[8];

public:
    Polysynth32();
    void begin();
    void noteOn(int noteIndex);
    void noteOff(int noteIndex);
    AudioStream &getOutputLeft() { return limiterLeft; }
    AudioStream &getOutputRight() { return limiterRight; }
};