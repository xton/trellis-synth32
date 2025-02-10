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

    ILayer *layers[LAYER_COUNT] = {&layer1, &layer2, &layer3, &layer4};
    size_t currentLayer = 0;

    // Final mixing
    AudioMixer4 finalMixLeft;
    AudioMixer4 finalMixRight;

    // effects and postprocessing
    AudioMixer4 gainMixerLeft;
    AudioMixer4 gainMixerRight;
    AudioEffectDelay delayL;
    AudioEffectDelay delayR;
    AudioMixer4 delayMixerLeft;
    AudioMixer4 delayMixerRight;
    AudioEffectBitcrusher bitcrusherLeft;
    AudioEffectBitcrusher bitcrusherRight;
    AudioEffectDynamics limiterLeft;
    AudioEffectDynamics limiterRight;

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

    // master gain
    AudioConnection pf0{finalMixLeft, gainMixerLeft};
    AudioConnection pf1{finalMixRight, gainMixerRight};

    // Delay routing with cross-feedback
    AudioConnection patchDryL{gainMixerLeft, delayMixerLeft};
    AudioConnection patchDryR{gainMixerRight, delayMixerRight};
    AudioConnection patchDelayInL{gainMixerLeft, delayL};
    AudioConnection patchDelayInR{gainMixerRight, delayR};
    AudioConnection patchDelayOutL{delayL, 0, delayMixerLeft, 1};
    AudioConnection patchDelayOutR{delayR, 0, delayMixerRight, 1};
    AudioConnection patchCrossL{delayL, 0, delayMixerRight, 2}; // Cross-feed L->R
    AudioConnection patchCrossR{delayR, 0, delayMixerLeft, 2};  // Cross-feed R->L

    // bitcrusher
    AudioConnection pf2{delayMixerLeft, bitcrusherLeft};
    AudioConnection pf3{delayMixerRight, bitcrusherRight};

    // finally, limiter
    AudioConnection pf4{bitcrusherLeft, limiterLeft};
    AudioConnection pf5{bitcrusherRight, limiterRight};

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

    void setDelay(bool isOn)
    {
        if (isOn)
        {
            // setup delay
            // Set up initial mixer gains
            delayMixerLeft.gain(0, 1.0); // Dry signal
            delayMixerLeft.gain(1, 0.4); // Delay feedback
            delayMixerLeft.gain(2, 0.2); // Cross-feed from right

            delayMixerRight.gain(0, 1.0); // Dry signal
            delayMixerRight.gain(1, 0.4); // Delay feedback
            delayMixerRight.gain(2, 0.2); // Cross-feed from left

            // Set up initial delay parameters
            delayL.delay(0, 220); // ~1/16th note at 120bpm
            delayR.delay(0, 660); // Golden ratio offset for pseudo-random feel
        }
        else
        {
            // disable
            delayMixerLeft.gain(0, 1.0);
            delayMixerLeft.gain(1, 0);
            delayMixerLeft.gain(2, 0);
            delayMixerRight.gain(0, 1.0);
            delayMixerRight.gain(1, 0);
            delayMixerRight.gain(2, 0);
            delayL.delay(0, 0);
            delayR.delay(0, 0);
        }
    }
};
