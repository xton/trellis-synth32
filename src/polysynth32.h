#pragma once
#include <Arduino.h>
#include <Audio.h>
#include "inote.h"
#include "synthnote.h"
#include "guitarnote.h"
#include "effect_dynamics.h"
#include "simplesynthnote.h"
#include "synthrow.h"
#include "delayrow.h"

class Polysynth32
{
private:
    // Note: GuitarNote and DrumNote still need to be implemented
    SynthRow<GuitarNote> synthRow1;      // Row 0: Guitar sounds
    DelayRow<SimpleSynthNote> synthRow2; // Row 1: Synth sounds
    SynthRow<SynthNote> synthRow3;       // Row 2: More synth sounds for now
    SynthRow<SynthNote> synthRow4;       // Row 3: More synth sounds for now

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