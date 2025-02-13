#pragma once

#include <Arduino.h>
#include <math.h>

#include "layer.h"
#include "sample_note.h"

#include "AudioSampleMeowsic_a0.h"
#include "AudioSampleMeowsic_a1.h"
#include "AudioSampleMeowsic_a2.h"
#include "AudioSampleMeowsic_c3.h"

struct SampleData
{
    int16_t *data;
    size_t length;
    float referenceFrequency;
};

/*

| Octave  | Note | Frequency (Hz)|
|---------|------|--------------|
| 0       | A0   | 27.50        |
| 0       | C1   | 32.70        |
| 1       | A1   | 55.00        |
| 1       | C2   | 65.41        |
| 2       | A2   | 110.00       |
| 2       | C3   | 130.81       |
| 3       | A3   | 220.00       |
| 3       | C4   | 261.63       |
| 4       | A4   | 440.00       |
| 4       | C5   | 523.25       |
| 5       | A5   | 880.00       |
| 5       | C6   | 1046.50      |
| 6       | A6   | 1760.00      |
| 6       | C7   | 2093.00      |
| 7       | A7   | 3520.00      |
| 7       | C8   | 4186.01      |
|-------------------------------|


Note: i'm transposing these 3 octaves to make them fit our scale better
*/

const SampleData MeowSamples[] = {
    {(int16_t *)AudioSampleMeowsic_a0, sizeof(AudioSampleMeowsic_a0) / 2, 220.0},
    {(int16_t *)AudioSampleMeowsic_a1, sizeof(AudioSampleMeowsic_a1) / 2, 440.0},
    {(int16_t *)AudioSampleMeowsic_a2, sizeof(AudioSampleMeowsic_a2) / 2, 880.0},
    {(int16_t *)AudioSampleMeowsic_c3, sizeof(AudioSampleMeowsic_c3) / 2, 1046.5}};

const size_t MEOW_SAMPLE_COUNT = sizeof(MeowSamples) / sizeof(MeowSamples[0]);

class MeowLayer : public Layer<SampleNote>
{

public:
    virtual void setScale(float const *frequencies)
    {
        for (size_t j = 0; j < ROW_COUNT; j++)
            for (size_t i = 0; i < NOTES_PER_ROW; i++)
            {
                SampleNote &note = rows[j].notes[i];
                float freq = *(frequencies++);

                // set the frequency
                note.setFrequency(freq);

                // now set the best sample
                const SampleData *bestSample = &MeowSamples[0];
                float bestDelta = std::fabs(bestSample->referenceFrequency - freq);
                for (size_t k = 1; k < MEOW_SAMPLE_COUNT; k++)
                {
                    const SampleData *thisSample = &MeowSamples[k];
                    float thisDelta = std::fabs(thisSample->referenceFrequency - freq);

                    if (thisDelta < bestDelta)
                    {
                        bestDelta = thisDelta;
                        bestSample = thisSample;
                    }

                    note.setSample(bestSample->data, bestSample->length, bestSample->referenceFrequency);
                }

                // these meows is quiet!
                note.setGain(4.0);
            }
    }
};