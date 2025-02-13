#pragma once
#include "sample_note.h"
#include "AudioSampleMeowsic_c1.h"

class MeowNote : public SampleNote
{
private:
    const int16_t *MeowSample16 = (int16_t *)AudioSampleMeowsic_c1;
    const size_t MeowSample16Length = sizeof(AudioSampleMeowsic_c1) / 2;

public:
    virtual void begin() override
    {
        // actual frequency for c4, but i want to transpose
        setSample(MeowSample16, MeowSample16Length, 261.6);
    }
};