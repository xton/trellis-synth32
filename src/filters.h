#pragma once

#include <Audio.h>

class Filter
{
private:
    Filter *parent;
    AudioConnection patchLeft;
    AudioConnection patchRight;

public:
    // first layer on filter stack must use this
    Filter(AudioStream &l, AudioStream &r)
        : parent(NULL),
          patchLeft(AudioConnection(l, inL())),
          patchRight(AudioConnection(r, inR()))
    {
    }

    // subsequent layers use this
    Filter(Filter &parent_)
        : parent(&parent_),
          patchLeft(AudioConnection(parent->outL(), inL())),
          patchRight(AudioConnection(parent->outR(), inR()))
    {
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
class SimpleMonoFilterChannel
{
public:
    T filter;
    virtual AudioStream &out() { return filter; }
    virtual AudioStream &in() { return filter; }
};

template <typename T>
class MonoFilter
{
public:
    T left() { return _left; }
    T right() { return _right; }

protected:
    T _left;
    T _right;

private:
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