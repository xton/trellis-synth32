#include <Audio.h>

class AudioPlayPlayMemoryVariable : public AudioStream
{
public:
    AudioPlayPlayMemoryVariable(void) : AudioStream(0, NULL) {}

    void play(const int16_t *data, uint32_t length);

    void stop(void)
    {
        playing = false;
        sample_data = NULL;
    }
    bool isPlaying(void) { return playing; }

    void setSpeed(float new_speed) { speed = new_speed; }

    virtual void update(void);

private:
    volatile bool playing = false;
    const int16_t *sample_data = NULL;
    uint32_t sample_length = 0;
    float speed = 1.0;
    volatile float sample_pos = 0.0;

    /// @brief fast forward to the first zero crossing to avoid a starting click.
    void findZeroCrossing();
};