#include "play_memory_variable.h"

void AudioPlayPlayMemoryVariable::update(void)
{
    audio_block_t *block;
    int16_t *out;

    // Return if not playing
    if (!playing)
        return;

    // Allocate output block
    block = allocate();
    if (block == NULL)
        return;

    // Fill the output block
    out = block->data;
    float pos = sample_pos;

    int i;
    for (i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
    {
        if ((uint32_t)pos >= sample_length)
            break; // Don't return immediately - transmit what we have
        out[i] = sample_data[(uint32_t)pos];
        pos += speed;
    }

    // If we didn't fill the whole block, zero the rest
    while (i < AUDIO_BLOCK_SAMPLES)
    {
        out[i++] = 0;
    }

    if ((uint32_t)pos >= sample_length)
    {
        playing = false; // Mark as done after transmitting final block
    }

    sample_pos = pos;
    transmit(block);
    release(block);
}
