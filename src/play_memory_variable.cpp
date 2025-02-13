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

void AudioPlayPlayMemoryVariable::play(const int16_t *data, uint32_t length)
{
    AudioNoInterrupts();
    sample_data = data;
    sample_length = length;
    sample_pos = 0;
    findZeroCrossing();
    playing = true;
    AudioInterrupts();
}

/// @brief fast forward to the first zero crossing to avoid a starting click.
void AudioPlayPlayMemoryVariable::findZeroCrossing()
{
    const static size_t MAX_SKIP = 600;
    float limit = sample_length < MAX_SKIP ? sample_length : MAX_SKIP;
    int16_t previous, current = sample_data[(size_t)sample_pos];

    do
    {
        previous = current;
        current = sample_data[(size_t)sample_pos];

        if (current == 0 || (previous < 0 && current > 0) || (current < 0 && previous > 0))
            break;

        sample_pos += 1.0;
    } while (sample_pos < limit);
}
