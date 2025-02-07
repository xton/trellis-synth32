#pragma once
#include "inote.h"
#include "simplesynthnote.h"
#include "synthnote.h"
#include "guitarnote.h"

#define NOTE_COUNT 3

class MultiNote : public INote
{
private:
    GuitarNote note0;
    SimpleSynthNote note1;
    SynthNote note2;

    INote *notes[3] = {&note0, &note1, &note2};
    uint8_t idx = 0; // current note idx

    AudioMixer4 demuxL;
    AudioMixer4 demuxR;

    AudioConnection p0L{note0.getOutputLeft(), 0, demuxL, 0};
    AudioConnection p0R{note0.getOutputRight(), 0, demuxR, 0};

    AudioConnection p1L{note1.getOutputLeft(), 0, demuxL, 1};
    AudioConnection p1R{note1.getOutputRight(), 0, demuxR, 1};

    AudioConnection p2L{note2.getOutputLeft(), 0, demuxL, 2};
    AudioConnection p2R{note2.getOutputRight(), 0, demuxR, 2};

public:
    void selectNote(uint8_t newIdx)
    {
        idx = newIdx;
        for (uint8_t i = 0; i < NOTE_COUNT; i++)
        {
            notes[i]->noteOff();
            demuxL.gain(i, i == idx ? 1.0 : 0.0);
            demuxR.gain(i, i == idx ? 1.0 : 0.0);
        }
    }

    void begin() override
    {
        for (uint8_t i = 0; i < NOTE_COUNT; i++)
            notes[i]->begin();

        // noop to initi state
        selectNote(idx);
    }

    // Start playing the note. Implementations should use fixed amplitude values
    void noteOn()
    {
        notes[idx]->noteOn();
    }

    // Implementations must ensure oscillators are turned off (amplitude=0)
    // when note is released to save CPU cycles
    void noteOff()
    {
        notes[idx]->noteOff();
    }

    void setFrequency(float freq)
    {
        for (uint8_t i = 0; i < NOTE_COUNT; i++)
        {
            notes[idx]->setFrequency(freq);
        }
    }

    AudioStream &getOutputLeft() { return demuxL; }
    AudioStream &getOutputRight() { return demuxR; }
};