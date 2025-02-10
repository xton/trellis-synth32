#pragma once
#include "synthrow.h"
#include "simplesynthnote.h"
#include "synthnote.h"
#include "guitarnote.h"
#include "cheapguitarnote.h"
#include "delayrow.h"
#include "inote.h"
#include "meow_note.h"

#include "debug.h"

class MultiRow
{
public:
    static const int VOICE_COUNT = 3;

private:
    SynthRow<SimpleSynthNote> synthRow0;
    // SynthRow<MeowNote> synthRow1;
    SynthRow<GuitarNote> synthRow2;
    // SynthRow<CheapGuitarNote> synthRow3;
    SynthRow<SimpleSynthNote> synthRow3;

    // ISynthRow *rows[VOICE_COUNT] = {&synthRow0, &synthRow1, &synthRow2, &synthRow3};
    ISynthRow *rows[VOICE_COUNT] = {&synthRow0, &synthRow2, &synthRow3};

    // PRINT_SIZE_ERROR(synthRow3);

    // SynthRow<GuitarNote> synthRow0;      // Row 0: Guitar sounds
    // SynthRow<SimpleSynthNote> synthRow1; // Row 1: Synth sounds
    // SynthRow<CheapGuitarNote> synthRow2; // Row 2: More synth sounds for now
    // DelayRow<SimpleSynthNote> synthRow3; // Row 3: More synth sounds for now

    uint8_t idx = 0; // current note idx

    AudioMixer4 demuxL;
    AudioMixer4 demuxR;

    AudioConnection p0L{rows[0]->getOutputLeft(), 0, demuxL, 0};
    AudioConnection p0R{rows[0]->getOutputRight(), 0, demuxR, 0};

    AudioConnection p1L{rows[1]->getOutputLeft(), 0, demuxL, 1};
    AudioConnection p1R{rows[1]->getOutputRight(), 0, demuxR, 1};

    AudioConnection p2L{rows[2]->getOutputLeft(), 0, demuxL, 2};
    AudioConnection p2R{rows[2]->getOutputRight(), 0, demuxR, 2};

    // AudioConnection p3L{rows[3]->getOutputLeft(), 0, demuxL, 3};
    // AudioConnection p3R{rows[3]->getOutputRight(), 0, demuxR, 3};

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