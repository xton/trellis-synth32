#pragma once

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/TomThumb.h>

class ISetting
{
public:
    virtual ~ISetting() = default;

    virtual void increment() = 0;
    virtual void decrement() = 0;

    // display the setting on the given display with
    // a preconfigured cursor and text style
    virtual void display(Adafruit_GFX &d) = 0;
    virtual void begin() = 0;

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
    ISetting &left;
    ISetting &right;
    const char *title = NULL;
    Slide(ISetting &left_,
          ISetting &right_,
          const char *title_ = NULL)
        : left(left_),
          right(right_),
          title(title_) {}
};

template <typename V>
class Setting : public ISetting
{
    using Mutator = V (*)(V oldValue);
    using Publisher = void (*)(V);
    using IntConverter = int (*)(V);

private:
    const char *fmt;

    V value;
    V minValue;
    V maxValue;

    Mutator incrementor;
    Mutator decrementor;
    Publisher publisher;

public:
    IntConverter intConverter = NULL;

    void setIntConverter(IntConverter f) { intConverter = f; }

    Setting(const char *fmt_,
            V value_,
            V minValue_,
            V maxValue_,
            Mutator incrementor_,
            Mutator decrementor_,
            Publisher publisher_,
            IntConverter intConverter_ = NULL)
        : fmt(fmt_),
          value(value_),
          minValue(minValue_),
          maxValue(maxValue_),
          incrementor(incrementor_),
          decrementor(decrementor_),
          publisher(publisher_),
          intConverter(intConverter_)
    {
    }

    void set(V newValue)
    {
        value = newValue;
        publisher(value);
    }

    void increment()
    {
        value = incrementor(value);
        if (value > maxValue)
            value = maxValue;

        publisher(value);
    }
    void decrement()
    {
        value = decrementor(value);
        if (value < minValue)
            value = minValue;

        publisher(value);
    }

    // display the setting on the given display with
    // a preconfigured cursor and text style
    void display(Adafruit_GFX &d)
    {
        if (intConverter)
        {
            d.printf(fmt, intConverter(value));
        }
        else
        {
            d.printf(fmt, value);
        }
    }

    void begin()
    {
        // align actual state with setting's
        publisher(value);
    }
};

template <typename V>
Setting(const char *, V, V, V, V (*)(V), V (*)(V), void (*)(V), int (*)(V)) -> Setting<V>;

template <size_t N, typename... Args>
class Menu
{
    Adafruit_SSD1306 &gfx;

    Slide slides[N];
    int currentSlide = 0;
    bool leftIsDown = false;
    bool rightIsDown = false;
    bool began = false;

public:
    Menu(Adafruit_SSD1306 &gfx_,
         Args... slides_)
        : gfx(gfx_),
          slides{slides_...} {}

    void display()
    {
        if (!began)
            return;

        gfx.clearDisplay();
        gfx.setTextColor(SSD1306_WHITE); // Draw white text

        // title
        slides[currentSlide].right.display(gfx);
        if (slides[currentSlide].title)
        {
            gfx.setTextSize(2);
            gfx.setCursor(10, 10);
            gfx.setFont(&TomThumb);
            gfx.print(slides[currentSlide].title);
        }

        // settings
        gfx.setTextSize(1);
        gfx.setFont(&FreeSansBold9pt7b);
        gfx.setCursor(0, 31);
        gfx.print("< ");
        slides[currentSlide].left.display(gfx);
        gfx.setCursor(0, 53);
        gfx.print("> ");
        gfx.display();
    }

    void nextSlide()
    {
        currentSlide++;
        if (currentSlide >= (int)N)
            currentSlide = N - 1;
        display();
    }

    void prevSlide()
    {
        currentSlide--;
        if (currentSlide < 0)
            currentSlide = 0;
        display();
    }

    void leftDown()
    {
        leftIsDown = true;
        prevSlide();
    }

    void leftUp()
    {
        leftIsDown = false;
    }

    void rightDown()
    {
        rightIsDown = true;
        nextSlide();
    }

    void rightUp()
    {
        rightIsDown = false;
    }

    void leftInc()
    {
        if (leftIsDown || rightIsDown)
        {
            nextSlide();
        }
        else
        {
            slides[currentSlide].left.increment();
            display();
        }
    }
    void leftDec()
    {
        if (leftIsDown || rightIsDown)
        {
            prevSlide();
        }
        else
        {
            slides[currentSlide].left.decrement();
            display();
        }
    }
    void rightInc()
    {
        if (leftIsDown || rightIsDown)
        {
            nextSlide();
        }
        else
        {
            slides[currentSlide].right.increment();
            display();
        }
    }
    void rightDec()
    {
        if (leftIsDown || rightIsDown)
        {
            prevSlide();
        }
        else
        {
            slides[currentSlide].right.decrement();
            display();
        }
    }

    void begin()
    {
        for (size_t i = 0; i < N; i++)
        {
            slides[i].right.begin();
            slides[i].left.begin();
        }
        began = true;
        display();
    }
};

// Deduction guide
template <typename... Args>
Menu(Adafruit_SSD1306 &, Args...) -> Menu<sizeof...(Args), Args...>;

#define SIMPLE_LAMBDA(decl, expr) (+[](decl) { return expr; })
#define PUBLISH_METHOD(func, tpe) (+[](tpe i) { return func(i); })