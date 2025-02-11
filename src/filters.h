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
    T left() { return _left; }
    T right() { return _right; }

protected:
    T _left;
    T _right;

public:
    AudioStream &outR() { return _right.out(); }
    AudioStream &outL() { return _left.out(); }
    AudioStream &inR() { return _right.in(); }
    AudioStream &inL() { return _left.in(); }
};

class GainFilter : public MonoFilter<SimpleMonoFilterChannel<AudioMixer4>>
{

public:
    void gain(float g)
    {
        _left.filter.gain(0, g);
        _right.filter.gain(0, g);
    }
};

class BitCrusherFilter : public MonoFilter<SimpleMonoFilterChannel<AudioEffectBitcrusher>>
{

public:
    void bits(uint8_t bits_)
    {
        _left.filter.bits(bits_);
        _left.filter.bits(bits_);
    }

    void sampleRate(float sampleRate_)
    {
        _left.filter.sampleRate(sampleRate_);
        _left.filter.sampleRate(sampleRate_);
    }
};

class LimiterFilter : public MonoFilter<SimpleMonoFilterChannel<AudioEffectDynamics>>
{

public:
    void begin()
    {
        _left.filter.compression(-12.0, 0.01, 0.06, 4.0);
        _right.filter.compression(-12.0, 0.01, 0.06, 4.0);
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