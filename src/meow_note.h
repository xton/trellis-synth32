#pragma once
#include "sample_note.h"
#include "meow_sample.h"

class MeowNote : public SampleNote
{
public:
    virtual void begin() override
    {
        setSample(MeowSample16, MeowSample16Length, 440.0);
    }
};