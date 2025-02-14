#pragma once

#include <new>

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
    V initialValue;
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
          value(value_), initialValue(value_),
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

    void reset()
    {
        set(initialValue);
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
          slides{slides_...}
    {
    }

    void display()
    {
        if (!began)
            return;

        gfx.clearDisplay();
        gfx.setTextColor(SSD1306_WHITE); // Draw white text

        // title
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
        slides[currentSlide].right.display(gfx);
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

struct Preset_
{
    const char *title;
    void (*callback)();
};

#define Preset(title, impl) \
    Preset_ { title, []() impl }

template <size_t N, typename... Args>
class PresetSlide
{
protected:
    Preset_ presets[N];
    size_t selection = 0;

    unsigned long changeNow = 0;
    size_t countDown;

    void setSelection(size_t selection_)
    {
        selection = selection_;
        changeNow = 0;
        countDown = 0;
    }

    /// @brief selects which preset to use
    class LeftSetting : public ISetting
    {
    protected:
        friend class PresetSlide;
        PresetSlide *parent = NULL;

    public:
        LeftSetting() {}
        virtual void increment()
        {
            if (parent->selection < N - 1)
            {
                parent->selection++;
                parent->changeNow = 0;
                parent->countDown = 0;
            }
        }
        virtual void decrement()
        {
            if (parent->selection > 0)
            {
                parent->selection--;
                parent->changeNow = 0;
                parent->countDown = 0;
            }
        }
        // display the setting on the given display with
        // a preconfigured cursor and text style
        virtual void display(Adafruit_GFX &d) { d.printf("%s", parent->presets[parent->selection].title); }
        virtual void begin() {}
    };

    /// @brief confirms the selection
    class RightSetting : public ISetting
    {
        friend class PresetSlide;
        PresetSlide *parent = NULL;

    public:
        RightSetting() {}

        virtual void increment()
        {
            // change in 3 seconds
            parent->changeNow = millis() + 3000;
            parent->tick(); // side effect: updates countdown
        }
        virtual void decrement()
        {
            // cancel change
            parent->changeNow = 0;
            parent->countDown = 0;
        }

        // display the setting on the given display with
        // a preconfigured cursor and text style
        virtual void display(Adafruit_GFX &d)
        {
            if (parent->changeNow == 0)
            {
                d.print("  yes? (up)");
            }
            else
            {
                d.printf("...%d", parent->countDown);
            }
        }
        virtual void begin() {}
    };

    friend class LeftSetting;
    friend class RightSetting;

    LeftSetting leftSetting;
    RightSetting rightSetting;
    Slide slide{leftSetting, rightSetting, "presets"};

public:
    PresetSlide(Args... presets_)
        : presets{presets_...}
    {
        leftSetting.parent = this;
        rightSetting.parent = this;
    }

    Slide getSlide() { return slide; }

    /// @brief Poll this
    /// @return true if display should be updated.
    bool tick()
    {
        auto now = millis();
        if (changeNow == 0)
        {
            return false;
        }
        else if (now > changeNow)
        {
            // do it!
            presets[selection].callback();
            changeNow = 0;
            countDown = 0;
            return true;
        }
        else
        {
            size_t newCountDown = (changeNow - now + 999) / 1000;
            bool shouldUpdate = newCountDown == countDown;
            countDown = newCountDown;
            return shouldUpdate;
        }
    }
};

// Deduction guide
template <typename... Args>
PresetSlide(Args...) -> PresetSlide<sizeof...(Args), Args...>;
