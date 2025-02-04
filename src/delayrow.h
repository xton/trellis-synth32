#pragma once
#include "synthrow.h"

template <typename T>
class DelayRow : public SynthRow<T>
{
private:
    AudioEffectDelay delayL;
    AudioEffectDelay delayR;
    AudioMixer4 finalMixL; // Mix dry and delayed signals
    AudioMixer4 finalMixR;

    // Connections from base class outputs to delays
    AudioConnection patchDryL{SynthRow<T>::getOutputLeft(), 0, finalMixL, 0};
    AudioConnection patchDryR{SynthRow<T>::getOutputRight(), 0, finalMixR, 0};

    // Delay routing with cross-feedback
    AudioConnection patchDelayInL{SynthRow<T>::getOutputLeft(), 0, delayL, 0};
    AudioConnection patchDelayInR{SynthRow<T>::getOutputRight(), 0, delayR, 0};
    AudioConnection patchDelayOutL{delayL, 0, finalMixL, 1};
    AudioConnection patchDelayOutR{delayR, 0, finalMixR, 1};
    AudioConnection patchCrossL{delayL, 0, finalMixR, 2}; // Cross-feed L->R
    AudioConnection patchCrossR{delayR, 0, finalMixL, 2}; // Cross-feed R->L

public:
    DelayRow() : SynthRow<T>()
    {
        // Set up initial mixer gains
        finalMixL.gain(0, 1.0); // Dry signal
        finalMixL.gain(1, 0.4); // Delay feedback
        finalMixL.gain(2, 0.2); // Cross-feed from right

        finalMixR.gain(0, 1.0); // Dry signal
        finalMixR.gain(1, 0.4); // Delay feedback
        finalMixR.gain(2, 0.2); // Cross-feed from left

        // Set up initial delay parameters
        delayL.delay(0, 220); // ~1/16th note at 120bpm
        delayR.delay(0, 660); // Golden ratio offset for pseudo-random feel
    }

    // Override the base class output methods to return our final mix
    AudioStream &getOutputLeft() { return finalMixL; }
    AudioStream &getOutputRight() { return finalMixR; }

    void setDelayTimes(float leftMs, float rightMs)
    {
        delayL.delay(0, leftMs);
        delayR.delay(0, rightMs);
    }

    void setFeedback(float amount)
    { // 0.0 to 1.0
        finalMixL.gain(1, amount);
        finalMixR.gain(1, amount);
    }

    void setCrossfeed(float amount)
    { // 0.0 to 1.0
        finalMixL.gain(2, amount);
        finalMixR.gain(2, amount);
    }
};
