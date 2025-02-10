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
    size_t currentLayer = 0;

    ILayer *layers[LAYER_COUNT] = {&layer1, &layer2, &layer3, &layer4};

    // Final mixing
    AudioMixer4 finalMixLeft;
    AudioMixer4 finalMixRight;

    // effects and postprocessing
    AudioEffectBitcrusher bitcrusherLeft;
    AudioEffectBitcrusher bitcrusherRight;
    AudioEffectDynamics limiterLeft;
    AudioEffectDynamics limiterRight;
    AudioMixer4 gainMixerLeft;
    AudioMixer4 gainMixerRight;

    // Audio connections for final mix stage
    AudioConnection patchL1{layer1.getOutputLeft(), 0, finalMixLeft, 0};
    AudioConnection patchL2{layer2.getOutputLeft(), 0, finalMixLeft, 1};
    AudioConnection patchL3{layer3.getOutputLeft(), 0, finalMixLeft, 2};
    AudioConnection patchL4{layer4.getOutputLeft(), 0, finalMixLeft, 3};

    AudioConnection patchR1{layer1.getOutputRight(), 0, finalMixRight, 0};
    AudioConnection patchR2{layer2.getOutputRight(), 0, finalMixRight, 1};
    AudioConnection patchR3{layer3.getOutputRight(), 0, finalMixRight, 2};
    AudioConnection patchR4{layer4.getOutputRight(), 0, finalMixRight, 3};

    // filter connections
    AudioConnection pf0{finalMixLeft, 0, bitcrusherLeft, 0};
    AudioConnection pf1{finalMixRight, 0, bitcrusherRight, 0};
    AudioConnection pf2{bitcrusherLeft, 0, gainMixerLeft, 0};
    AudioConnection pf3{bitcrusherRight, 0, gainMixerRight, 0};
    AudioConnection pf4{gainMixerLeft, 0, limiterLeft, 0};
    AudioConnection pf5{gainMixerRight, 0, limiterRight, 0};

public:
    Polysynth32();
    void begin();
    void noteOn(int noteIndex) { layers[currentLayer]->noteOn(noteIndex); }
    void noteOff(int noteIndex) { layers[currentLayer]->noteOff(noteIndex); }
    AudioStream &getOutputLeft() { return limiterLeft; }
    AudioStream &getOutputRight() { return limiterRight; }
    void setupScales();
    void selectVoice(uint8_t idx);

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
};
