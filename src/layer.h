#pragma once

#include <Arduino.h>
#include <Audio.h>
#include "inote.h"

class ILayer
{
public:
    static const int NOTES_PER_ROW = 8;
    static const size_t ROW_COUNT = 4;
    virtual ~ILayer() = default;

    virtual void begin() = 0;
    virtual void enable() = 0;
    virtual void disable() = 0;
    virtual void noteOn(int index) = 0;
    virtual void noteOff(int index) = 0;
    virtual AudioStream &getOutputLeft() = 0;
    virtual AudioStream &getOutputRight() = 0;
    virtual void setScale(const float *frequencies) = 0;

protected:
    ILayer() = default;
    ILayer(const ILayer &) = delete;
    ILayer &operator=(const ILayer &) = delete;
};

template <typename T>
class Layer;

template <typename T>
class Row
{
public:
    AudioStream &getOutputLeft() { return finalLeft; }
    AudioStream &getOutputRight() { return finalRight; }
    T notes[ILayer::NOTES_PER_ROW];

protected:
    friend class Layer<T>;

private:
    AudioMixer4 mixLeft1;   // Mixes notes 0-3
    AudioMixer4 mixLeft2;   // Mixes notes 4-7
    AudioMixer4 finalLeft;  // Combines left mixers
    AudioMixer4 mixRight1;  // Mixes notes 0-3
    AudioMixer4 mixRight2;  // Mixes notes 4-7
    AudioMixer4 finalRight; // Combines right mixers

    // Audio connections for note outputs to first layer mixers
    AudioConnection patchL1_0{notes[0].getOutputLeft(), 0, mixLeft1, 0};
    AudioConnection patchL1_1{notes[1].getOutputLeft(), 0, mixLeft1, 1};
    AudioConnection patchL1_2{notes[2].getOutputLeft(), 0, mixLeft1, 2};
    AudioConnection patchL1_3{notes[3].getOutputLeft(), 0, mixLeft1, 3};
    AudioConnection patchL2_0{notes[4].getOutputLeft(), 0, mixLeft2, 0};
    AudioConnection patchL2_1{notes[5].getOutputLeft(), 0, mixLeft2, 1};
    AudioConnection patchL2_2{notes[6].getOutputLeft(), 0, mixLeft2, 2};
    AudioConnection patchL2_3{notes[7].getOutputLeft(), 0, mixLeft2, 3};

    AudioConnection patchR1_0{notes[0].getOutputRight(), 0, mixRight1, 0};
    AudioConnection patchR1_1{notes[1].getOutputRight(), 0, mixRight1, 1};
    AudioConnection patchR1_2{notes[2].getOutputRight(), 0, mixRight1, 2};
    AudioConnection patchR1_3{notes[3].getOutputRight(), 0, mixRight1, 3};
    AudioConnection patchR2_0{notes[4].getOutputRight(), 0, mixRight2, 0};
    AudioConnection patchR2_1{notes[5].getOutputRight(), 0, mixRight2, 1};
    AudioConnection patchR2_2{notes[6].getOutputRight(), 0, mixRight2, 2};
    AudioConnection patchR2_3{notes[7].getOutputRight(), 0, mixRight2, 3};

    // second layers
    AudioConnection patchLF1{mixLeft1, 0, finalLeft, 0};
    AudioConnection patchLF2{mixLeft2, 0, finalLeft, 1};
    AudioConnection patchRF1{mixRight1, 0, finalRight, 0};
    AudioConnection patchRF2{mixRight2, 0, finalRight, 1};
};

template <typename T>
class Layer : public ILayer
{
public:
    template <typename Func>
    inline void for_all_notes(Func operation)
    {
        for (size_t j = 0; j < ROW_COUNT; j++)
            for (size_t i = 0; i < NOTES_PER_ROW; i++)
                operation(rows[j].notes[i]);
    }

    virtual void begin()
    {
        for_all_notes([](INote &note)
                      { note.begin(); });
    }
    void enable()
    {
        for_all_notes([](INote &note)
                      { note.enable(); });
    }

    void disable()
    {
        for_all_notes([](INote &note)
                      { note.disable(); });
    }

    void noteOn(int index)
    {
        rows[index / NOTES_PER_ROW].notes[index % NOTES_PER_ROW].noteOn();
    }

    void noteOff(int index)
    {
        rows[index / NOTES_PER_ROW].notes[index % NOTES_PER_ROW].noteOff();
    }

    AudioStream &getOutputLeft() { return finalLeft; }
    AudioStream &getOutputRight() { return finalRight; }

    void setScale(float const *frequencies)
    {
        for_all_notes([&frequencies](INote &note)
                      { note.setFrequency(*(frequencies++)); });
    }

protected:
    Row<T> rows[ROW_COUNT];
    AudioMixer4 finalLeft;  // Combines left mixers
    AudioMixer4 finalRight; // Combines right mixers

    AudioConnection patchLF1{rows[0].getOutputLeft(), 0, finalLeft, 0};
    AudioConnection patchLF2{rows[1].getOutputLeft(), 0, finalLeft, 1};
    AudioConnection patchLF3{rows[2].getOutputLeft(), 0, finalLeft, 2};
    AudioConnection patchLF4{rows[3].getOutputLeft(), 0, finalLeft, 3};

    AudioConnection patchRF1{rows[0].getOutputRight(), 0, finalRight, 0};
    AudioConnection patchRF2{rows[1].getOutputRight(), 0, finalRight, 1};
    AudioConnection patchRF3{rows[2].getOutputRight(), 0, finalRight, 2};
    AudioConnection patchRF4{rows[3].getOutputRight(), 0, finalRight, 3};
};
