#pragma once
#include "synthrow.h"
#include "simplesynthnote.h"
#include "synthnote.h"
#include "guitarnote.h"
#include "delayrow.h"
#include "inote.h"

class MultiRow
{
public:
    static const int VOICE_COUNT = 4;

private:
    SynthRow<GuitarNote> synthRow0;      // Row 0: Guitar sounds
    SynthRow<SimpleSynthNote> synthRow1; // Row 1: Synth sounds
    SynthRow<GuitarNote> synthRow2;      // Row 2: More synth sounds for now
    SynthRow<SimpleSynthNote> synthRow3; // Row 3: More synth sounds for now

    ISynthRow *rows[VOICE_COUNT] = {&synthRow0, &synthRow1, &synthRow2, &synthRow3};
    uint8_t idx = 0; // current note idx

    AudioMixer4 demuxL;
    AudioMixer4 demuxR;

    AudioConnection p0L{synthRow0.getOutputLeft(), 0, demuxL, 0};
    AudioConnection p0R{synthRow0.getOutputRight(), 0, demuxR, 0};

    AudioConnection p1L{synthRow1.getOutputLeft(), 0, demuxL, 1};
    AudioConnection p1R{synthRow1.getOutputRight(), 0, demuxR, 1};

    AudioConnection p2L{synthRow2.getOutputLeft(), 0, demuxL, 2};
    AudioConnection p2R{synthRow2.getOutputRight(), 0, demuxR, 2};

    AudioConnection p3L{synthRow3.getOutputLeft(), 0, demuxL, 3};
    AudioConnection p3R{synthRow3.getOutputRight(), 0, demuxR, 3};

public:
    MultiRow() {}

    void selectVoice(uint8_t newIdx)
    {
        idx = newIdx;
        for (uint8_t i = 0; i < VOICE_COUNT; i++)
        {
            // stop all running voices (maybe remove)
            for (uint8_t j = 0; j < ISynthRow::NOTES_PER_ROW; j++)
                rows[i]->noteOff(j);

            demuxL.gain(i, i == idx ? 1.0 : 0.0);
            demuxR.gain(i, i == idx ? 1.0 : 0.0);
        }
    }

    void begin()
    {
        for (uint8_t i = 0; i < VOICE_COUNT; i++)
            rows[i]->begin();

        // initialize mixer
        selectVoice(idx);
    }

    void noteOn(int index) { rows[idx]->noteOn(index); }
    void noteOff(int index) { rows[idx]->noteOff(index); }

    AudioStream &getOutputLeft() { return demuxL; }
    AudioStream &getOutputRight() { return demuxR; }

    void setScale(const float *frequencies)
    {
        for (uint8_t i = 0; i < VOICE_COUNT; i++)
            rows[i]->setScale(frequencies);
    }
};