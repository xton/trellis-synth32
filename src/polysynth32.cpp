#include "polysynth32.h"

Polysynth32::Polysynth32() {}

void Polysynth32::setupScales()
{

    // Scale configuration
    float scale[ILayer::NOTES_PER_ROW * ILayer::ROW_COUNT];

    scaleGen.generateFullScale(scale);
    // // Row 1: C3 pentatonic
    // scale[0] = 130.81; // C3
    // scale[1] = 146.83; // D3
    // scale[2] = 164.81; // E3
    // scale[3] = 196.00; // G3
    // scale[4] = 220.00; // A3
    // scale[5] = 261.63; // C4
    // scale[6] = 293.66; // D4
    // scale[7] = 329.63; // E4

    // for (int i = 0; i < 8; i++)
    // {
    //     // Row 2: Same pattern one octave up (C4 pentatonic)
    //     scale[8 + i] = scale[i] * 2.0;
    //     // Row 3: Two octaves up
    //     scale[16 + i] = scale[i] * 4.0;
    //     // Row 4: Three octaves up
    //     scale[24 + i] = scale[i] * 8.0;
    // }

    // Set scales for each row
    for (int j = 0; j < LAYER_COUNT; j++)
        layers[j]->setScale(scale);
}

void Polysynth32::begin()
{
    setupScales();

    // Initialize synth layers
    for (int i = 0; i < LAYER_COUNT; i++)
        layers[i]->begin();

    selectVoice(currentLayer);
}

void Polysynth32::selectVoice(uint8_t idx)
{
    currentLayer = idx;
    for (int i = 0; i < LAYER_COUNT; i++)
    {
        if (i == idx)
        {
            finalMixLeft.gain(i, 1.0);
            finalMixRight.gain(i, 1.0);
            layers[i]->enable();
        }
        else
        {
            finalMixLeft.gain(i, 0);
            finalMixRight.gain(i, 0);
            layers[i]->disable();
        }
    }
}
