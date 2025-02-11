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
    Polysynth32 &synth;
    Adafruit_SSD1306 &gfx;

    const Slide slides[N];
    int currentSlide = 0;

public:
    Menu(Polysynth32 &synth_,
         Adafruit_SSD1306 &gfx_,
         const Args... slides_)
        : synth(synth_),
          gfx(gfx_),
          slides{slides_...} {}

    void display()
    {
        gfx.clearDisplay();
        gfx.setTextSize(1);
        gfx.setFont(&FreeSansBold12pt7b);
        gfx.setTextColor(SSD1306_WHITE); // Draw white text
        gfx.setCursor(0, 32);
        slides[currentSlide].left.display(gfx);
        gfx.setCursor(0, 64);
        slides[currentSlide].right.display(gfx);
        gfx.display();
    }

    void leftClick()
    {
        currentSlide--;
        if (currentSlide < 0)
            currentSlide = 0;

        display();
    }

    void rightClick()
    {
        currentSlide++;
        if (currentSlide >= N)
            currentSlide = N - 1;

        display();
    }

    void leftInc()
    {
        slides[currentSlide].left.increment(synth);
        display();
    }
    void leftDec()
    {
        slides[currentSlide].left.decrement(synth);
        display();
    }
    void rightInc()
    {
        slides[currentSlide].right.increment(synth);
        display();
    }
    void rightDec()
    {
        slides[currentSlide].right.decrement(synth);
        display();
    }
};

// Deduction guide
template <typename... Args>
Menu(Polysynth32 &, Adafruit_SSD1306 &, Args...) -> Menu<sizeof...(Args), Args...>;

#define SIMPLE_LAMBDA(decl, expr) (+[](decl) { return expr; })
#define PUBLISH_METHOD(meth, tpe) (+[](Polysynth32 &s, tpe i) { return s.meth(i); })