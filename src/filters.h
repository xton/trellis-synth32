#pragma once

#include <Audio.h>
#include <new>
#include "effect_dynamics.h"

class Filter
{
private:
    // we're doing delayed initialization
    alignas(AudioConnection) byte plbuf[sizeof(AudioConnection)];
    alignas(AudioConnection) byte prbuf[sizeof(AudioConnection)];
    AudioConnection *patchLeft = NULL;
    AudioConnection *patchRight = NULL;

public:
    void connect(AudioStream &left_, AudioStream &right_)
    {
        patchLeft = new (plbuf) AudioConnection(left_, inL());
        patchRight = new (prbuf) AudioConnection(right_, inR());
    }

    ~Filter()
    {
        if (patchLeft)
            patchLeft->~AudioConnection();
        if (patchRight)
            patchRight->~AudioConnection();
    }

    virtual void begin() {}
    virtual AudioStream &outR() = 0;
    virtual AudioStream &outL() = 0;
    virtual AudioStream &inR() = 0;
    virtual AudioStream &inL() = 0;
};

class MonoFilterChannel
{
public:
    virtual AudioStream &out() = 0;
    virtual AudioStream &in() = 0;
};

template <typename T>
class SimpleMonoFilterChannel : public MonoFilterChannel
{
public:
    T filter;
    virtual AudioStream &out() { return filter; }
    virtual AudioStream &in() { return filter; }
};

template <typename T>
class MonoFilter : public Filter
{
public:
    T left;
    T right;

    AudioStream &outR() { return right.out(); }
    AudioStream &outL() { return left.out(); }
    AudioStream &inR() { return right.in(); }
    AudioStream &inL() { return left.in(); }
};

class GainFilter : public MonoFilter<SimpleMonoFilterChannel<AudioMixer4>>
{

public:
    void gain(float g)
    {
        left.filter.gain(0, g);
        right.filter.gain(0, g);
    }
};

class BitCrusherFilter : public MonoFilter<SimpleMonoFilterChannel<AudioEffectBitcrusher>>
{

public:
    void bits(uint8_t bits_)
    {
        left.filter.bits(bits_);
        left.filter.bits(bits_);
    }

    void sampleRate(float sampleRate_)
    {
        left.filter.sampleRate(sampleRate_);
        left.filter.sampleRate(sampleRate_);
    }
};

class LimiterFilter : public MonoFilter<SimpleMonoFilterChannel<AudioEffectDynamics>>
{

public:
    void begin()
    {
        left.filter.compression(-12.0, 0.01, 0.06, 4.0);
        right.filter.compression(-12.0, 0.01, 0.06, 4.0);
    }
};

class DelayFilter : public Filter
{

public:
    AudioMixer4 dryL;
    AudioMixer4 dryR;
    AudioEffectDelay delayL;
    AudioEffectDelay delayR;
    AudioMixer4 finalL;
    AudioMixer4 finalR;

    AudioConnection patchDryL{dryL, finalL};
    AudioConnection patchDryR{dryR, finalR};
    AudioConnection patchDelayInL{dryL, delayL};
    AudioConnection patchDelayInR{dryR, delayR};
    AudioConnection patchDelayOutL{delayL, 0, finalL, 1};
    AudioConnection patchDelayOutR{delayR, 0, finalR, 1};
    AudioConnection patchCrossL{delayL, 0, finalR, 2}; // Cross-feed L->R
    AudioConnection patchCrossR{delayR, 0, finalL, 2}; // Cross-feed R->L

    AudioStream &outR() { return finalR; }
    AudioStream &outL() { return finalL; }
    AudioStream &inR() { return dryR; }
    AudioStream &inL() { return dryL; }

    void begin()
    {
        setActive(false);
    }

    void setActive(bool active)
    {
        if (active)
        {
            // setup delay
            // Set up initial mixer gains
            finalL.gain(0, 1.0); // Dry signal
            finalL.gain(1, 0.4); // Delay feedback
            finalL.gain(2, 0.2); // Cross-feed from right

            finalR.gain(0, 1.0); // Dry signal
            finalR.gain(1, 0.4); // Delay feedback
            finalR.gain(2, 0.2); // Cross-feed from left

            // Set up initial delay parameters
            delayL.delay(0, 220); // ~1/16th note at 120bpm
            delayR.delay(0, 660); // Golden ratio offset for pseudo-random feel
        }
        else
        {
            // disable
            finalL.gain(0, 1.0);
            finalL.gain(1, 0);
            finalL.gain(2, 0);
            finalR.gain(0, 1.0);
            finalR.gain(1, 0);
            finalR.gain(2, 0);
            delayL.delay(0, 0);
            delayR.delay(0, 0);
        }
    }
};

class FeedbackMonoFilterChannel : public MonoFilterChannel
{
public:
    // Audio objects
    AudioMixer4 baseMix;  // Mix input with feedback
    AudioMixer4 driveMix; // Mix input with feedback
    AudioEffectDelay delay;
    AudioFilterStateVariable filter;
    AudioMixer4 fbMix;    // Feedback mix stage
    AudioMixer4 finalMix; // Final output mix

    // patch cables

    // Input to drive stage
    AudioConnection patchInDrive{baseMix, driveMix};     // Input signal
    AudioConnection patchFbDrive{fbMix, 0, driveMix, 1}; // Feedback signal

    // Drive to delay
    AudioConnection patchDriveDelay{driveMix, delay};

    // Delay to filter
    AudioConnection patchDelayFilter{delay, filter};

    // Filter to feedback mix
    AudioConnection patchFilterFb{filter, fbMix};

    // Final outputs
    AudioConnection patchDry{baseMix, finalMix};     // Dry path
    AudioConnection patchWet{fbMix, 0, finalMix, 1}; // Wet path from feedback mix

    virtual AudioStream &in() { return baseMix; }
    virtual AudioStream &out() { return finalMix; }
};

class FeeedbackFilter : public Filter
{
public:
    FeedbackMonoFilterChannel left;
    FeedbackMonoFilterChannel right;

    // Cross-feedback to drive mix
    AudioConnection patchXFbDriveL{right.fbMix, 0, left.driveMix, 2};
    AudioConnection patchXFbDriveR{left.fbMix, 0, right.driveMix, 2};

    AudioStream &outR() { return right.out(); }
    AudioStream &outL() { return left.out(); }
    AudioStream &inR() { return right.in(); }
    AudioStream &inL() { return left.in(); }

    void setDelayRight(float d)
    {
        right.delay.delay(0, d);
    }

    void setDelayLeft(float d)
    {
        left.delay.delay(0, d);
    }

    void setDrive(float amount)
    {
        left.driveMix.gain(0, amount);
        right.driveMix.gain(0, amount);
    }

    void setFeedback(float fb)
    {
        // Clamp feedback to 0.9 for safety
        left.driveMix.gain(1, fb);
        right.driveMix.gain(1, fb);
    }

    void setCrossFeedback(float xfb)
    {
        // Clamp cross-feedback to 0.5 for safety
        left.driveMix.gain(2, xfb);
        right.driveMix.gain(2, xfb);
    }

    void setFilterFreq(float freq)
    {
        left.filter.frequency(freq);
        right.filter.frequency(freq);
    }

    void setFilterRes(float q)
    {
        left.filter.resonance(q);
        right.filter.resonance(q);
    }

    void setWetDryMix(float wet)
    {
        float dry = 1.0f - wet;
        left.finalMix.gain(0, dry);
        left.finalMix.gain(1, wet);
        right.finalMix.gain(0, dry);
        right.finalMix.gain(1, wet);
    }

    void begin()
    {
        setDelayRight(266);
        setDelayLeft(399);

        setDrive(1.0);

        setFeedback(0.4);
        setCrossFeedback(0.2);

        setFilterFreq(1000);
        setFilterRes(0.7);

        setWetDryMix(0.7);
    }
};