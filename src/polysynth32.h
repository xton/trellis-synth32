#pragma once
#include <Arduino.h>
#include <Audio.h>
#include "inote.h"
#include "effect_dynamics.h"

#include "simplesynthnote.h"
#include "guitarnote.h"
#include "cheapguitarnote.h"
#include "meow_note.h"

#include "layer.h"
#include "filters.h"
#include "debug.h"

class Polysynth32
{
public:
    static const int LAYER_COUNT = 4;

private:
    Layer<SimpleSynthNote> layer1;
    Layer<MeowNote> layer2;
    Layer<GuitarNote> layer3;
    Layer<CheapGuitarNote> layer4;

    ILayer *layers[LAYER_COUNT] = {&layer1, &layer2, &layer3, &layer4};
    size_t currentLayer = 0;

    // Final mixing
    AudioMixer4 finalMixLeft;
    AudioMixer4 finalMixRight;

    // Audio connections for final mix stage
    AudioConnection patchL1{layer1.getOutputLeft(), 0, finalMixLeft, 0};
    AudioConnection patchL2{layer2.getOutputLeft(), 0, finalMixLeft, 1};
    AudioConnection patchL3{layer3.getOutputLeft(), 0, finalMixLeft, 2};
    AudioConnection patchL4{layer4.getOutputLeft(), 0, finalMixLeft, 3};

    AudioConnection patchR1{layer1.getOutputRight(), 0, finalMixRight, 0};
    AudioConnection patchR2{layer2.getOutputRight(), 0, finalMixRight, 1};
    AudioConnection patchR3{layer3.getOutputRight(), 0, finalMixRight, 2};
    AudioConnection patchR4{layer4.getOutputRight(), 0, finalMixRight, 3};

public:
    Polysynth32();
    void begin();
    void noteOn(int noteIndex) { layers[currentLayer]->noteOn(noteIndex); }
    void noteOff(int noteIndex) { layers[currentLayer]->noteOff(noteIndex); }

    // effects and postprocessing
    GainFilter gain{finalMixLeft, finalMixRight};
    DelayFilter delay{&gain};
    BitCrusherFilter bitCrusher{&delay};
    LimiterFilter limiter{&bitCrusher};

    AudioStream &getOutputLeft() { return limiter.outL(); }
    AudioStream &getOutputRight() { return limiter.outR(); }
    void setupScales();
    void selectVoice(uint8_t idx);
};
