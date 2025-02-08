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
#include "multirow.h"

#include "debug.h"

class Polysynth32
{
public:
    static const int ROW_COUNT = 4;

private:
    // Note: GuitarNote and DrumNote still need to be implemented
    MultiRow synthRow1; // Row 0: Guitar sounds
    MultiRow synthRow2; // Row 1: Synth sounds
    MultiRow synthRow3; // Row 2: More synth sounds for now
    MultiRow synthRow4; // Row 3: More synth sounds for now

    MultiRow *rows[4] = {&synthRow1, &synthRow2, &synthRow3, &synthRow4};

    // Final mixing and limiting stage
    AudioMixer4 finalMixLeft;
    AudioMixer4 finalMixRight;
    AudioEffectBitcrusher bitcrusherLeft;
    AudioEffectBitcrusher bitcrusherRight;
    AudioEffectDynamics limiterLeft;
    AudioEffectDynamics limiterRight;
    AudioMixer4 gainMixerLeft;
    AudioMixer4 gainMixerRight;

    // Audio connections for final mix stage
    AudioConnection patchL1{synthRow1.getOutputLeft(), 0, finalMixLeft, 0};
    AudioConnection patchL2{synthRow2.getOutputLeft(), 0, finalMixLeft, 1};
    AudioConnection patchL3{synthRow3.getOutputLeft(), 0, finalMixLeft, 2};
    AudioConnection patchL4{synthRow4.getOutputLeft(), 0, finalMixLeft, 3};

    AudioConnection patchR1{synthRow1.getOutputRight(), 0, finalMixRight, 0};
    AudioConnection patchR2{synthRow2.getOutputRight(), 0, finalMixRight, 1};
    AudioConnection patchR3{synthRow3.getOutputRight(), 0, finalMixRight, 2};
    AudioConnection patchR4{synthRow4.getOutputRight(), 0, finalMixRight, 3};

    // filter connections
    AudioConnection pf0{finalMixLeft, 0, bitcrusherLeft, 0};
    AudioConnection pf1{finalMixRight, 0, bitcrusherRight, 0};
    AudioConnection pf2{bitcrusherLeft, 0, gainMixerLeft, 0};
    AudioConnection pf3{bitcrusherRight, 0, gainMixerRight, 0};
    AudioConnection pf4{gainMixerLeft, 0, limiterLeft, 0};
    AudioConnection pf5{gainMixerRight, 0, limiterRight, 0};

    // AudioConnection patchLimL{finalMixLeft, 0, limiterLeft, 0};
    // AudioConnection patchLimR{finalMixRight, 0, limiterRight, 0};

    // Scale configuration
    void setupScales();

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

    void setCrusherBits(uint8_t b)
    {
        bitcrusherRight.bits(b);
        bitcrusherLeft.bits(b);
    }

    void setCrusherSampleRate(float r)
    {
        bitcrusherLeft.sampleRate(r);
        bitcrusherRight.sampleRate(r);
    }

    void setGain(float g)
    {
        gainMixerLeft.gain(0, g);
        gainMixerRight.gain(0, g);
    }

    void selectVoice(uint8_t idx)
    {
        for (int i = 0; i < ROW_COUNT; i++)
            rows[i]->selectVoice(idx);
    }
};

// PRINT_SIZE_ERROR(Polysynth32);