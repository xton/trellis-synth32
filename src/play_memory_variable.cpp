#include "play_memory_variable.h"

void AudioPlayPlayMemoryVariable::update(void)
{
    audio_block_t *block;
    int16_t *out;

    if (!playing)
        return;

    block = allocate();
    if (block == NULL)
        return;

    out = block->data;
    float pos = sample_pos;

    int i;
    for (i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
    {
        if ((uint32_t)pos >= sample_length - 1)
        { // Need an extra sample for interpolation
            break;
        }

        // Get the two samples we're between
        uint32_t pos_int = (uint32_t)pos;
        float frac = pos - pos_int;
        int16_t sample1 = sample_data[pos_int];
        int16_t sample2 = sample_data[pos_int + 1];

        // Linear interpolation
        out[i] = sample1 + (sample2 - sample1) * frac;

        pos += speed;
    }

    while (i < AUDIO_BLOCK_SAMPLES)
    {
        out[i++] = 0;
    }

    if ((uint32_t)pos >= sample_length - 1)
    {
        playing = false;
    }

    sample_pos = pos;
    transmit(block);
    release(block);
}
