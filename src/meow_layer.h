#pragma once

#include <Arduino.h>

#include "layer.h"
#include "sample_note.h"

#include "AudioSampleMeowsic_a0.h"
#include "AudioSampleMeowsic_a1.h"
#include "AudioSampleMeowsic_a2.h"
#include "AudioSampleMeowsic_c3.h"

class MeowLayer : public Layer<SampleNote>
{
public:
    virtual void begin()
    {
        Layer::begin();

        for (size_t j = 0; j < ROW_COUNT; j++)
            for (size_t i = 0; i < NOTES_PER_ROW; i++)
            {
                switch (j)
                {
                    // note: two octaves higher than stated
                case 0:
                    rows[j].notes[i].setSample((int16_t *)AudioSampleMeowsic_a0, sizeof(AudioSampleMeowsic_a0) / 2, 110.0);
                    break;
                case 1:
                    rows[j].notes[i].setSample((int16_t *)AudioSampleMeowsic_a1, sizeof(AudioSampleMeowsic_a1) / 2, 220.0);
                    break;
                case 2:
                    rows[j].notes[i].setSample((int16_t *)AudioSampleMeowsic_a2, sizeof(AudioSampleMeowsic_a2) / 2, 440.0);
                    break;
                default:
                    rows[j].notes[i].setSample((int16_t *)AudioSampleMeowsic_c3, sizeof(AudioSampleMeowsic_c3) / 2, 523.25);
                    break;
                }
            }
    }
};