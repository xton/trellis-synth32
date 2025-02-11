#pragma once

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include "polysynth32.h"

class ISetting
{
public:
    virtual ~ISetting() = default;

    virtual void increment(Polysynth32 &synth) = 0;
    virtual void decrement(Polysynth32 &synth) = 0;

    // display the setting on the given display with
    // a preconfigured cursor and text style
    virtual void display(Adafruit_GFX &d) = 0;

protected:
    // Protected constructor prevents instantiation of interface
    ISetting() = default;

    // Prevent copying of interface
    ISetting(const ISetting &) = delete;
    ISetting &operator=(const ISetting &) = delete;
};

class Slide
{
public:
    const ISetting &left;
    const ISetting &right;
    Slide(const ISetting &left_, const ISetting &right_) : left(left_), right(right_) {}
};

template <typename V>
class Setting : public ISetting
{
    using Mutator = V (*)(V oldValue);
    using Publisher = void (*)(Polysynth32 &, V);

private:
    const char *fmt;

    V value;
    V minValue;
    V maxValue;

    Mutator incrementor;
    Mutator decrementor;
    Publisher publisher;

public:
    Setting(const char *fmt_,
            V value_,
            V minValue_,
            V maxValue_,
            Mutator incrementor_,
            Mutator decrementor_,
            Publisher publisher_)
        : fmt(fmt_),
          value(value_),
          minValue(minValue_),
          maxValue(maxValue_),
          incrementor(incrementor_),
          decrementor(decrementor_),
          publisher(publisher_) {}

    void increment(Polysynth32 &synth)
    {
        value = incrementor(value);
        if (value > maxValue)
            value = maxValue;

        publisher(synth, value);
    }
    void decrement(Polysynth32 &synth)
    {
        value = decrementor(value);
        if (value < minValue)
            value = minValue;

        publisher(synth, value);
    }

    // display the setting on the given display with
    // a preconfigured cursor and text style
    void display(Adafruit_GFX &d)
    {
        d.printf(fmt, value);
    }
};

template <typename V>
Setting(const char *, V, V, V, V (*)(V), V (*)(V), void (*)(Polysynth32 &, V)) -> Setting<V>;

template <size_t N, typename... Args>
class Menu
{
    const Slide slides[N];

public:
    Menu(const Args... slides_) : slides{slides_...} {}
};

// Deduction guide
template <typename... Args>
Menu(Args...) -> Menu<sizeof...(Args), Args...>;

#define SIMPLE_LAMBDA(decl, expr) (+[](decl) { return expr; })
#define PUBLISH_METHOD(meth) (+[](Polysynth32 &s, int i) { return s.meth(i); })