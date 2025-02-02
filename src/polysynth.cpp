#include "polysynth.h"

void PolySynth::begin()
{
    // Initialize envelopes
    for (int i = 0; i < NUM_VOICES; i++)
    {
        envs[i].attack(9.2);
        envs[i].hold(2.1);
        envs[i].decay(31.4);
        envs[i].sustain(0.6);
        envs[i].release(84.5);
    }

    // Set mixer gains
    for (int i = 0; i < 4; i++)
    {
        mixer1.gain(i, 0.25);
        mixer2.gain(i, 0.25);
    }
    mixerFinal.gain(0, 0.5);
    mixerFinal.gain(1, 0.5);
}

int PolySynth::findVoiceForNote(int noteNumber) const
{
    // First check if this note is already playing
    for (int i = 0; i < NUM_VOICES; i++)
    {
        if (voices[i].active && voices[i].noteNumber == noteNumber)
        {
            return i;
        }
    }
    return -1;
}

int PolySynth::findFreeVoice() const
{
    for (int i = 0; i < NUM_VOICES; i++)
    {
        if (!voices[i].active)
        {
            return i;
        }
    }
    return -1;
}

int PolySynth::findOldestVoice() const
{
    uint32_t oldestTime = 0xFFFFFFFF;
    int oldestIndex = 0;

    for (int i = 0; i < NUM_VOICES; i++)
    {
        if (voices[i].startTime < oldestTime)
        {
            oldestTime = voices[i].startTime;
            oldestIndex = i;
        }
    }
    return oldestIndex;
}

void PolySynth::noteOn(int noteNumber, float frequency, int waveform)
{
    int voiceIndex = findVoiceForNote(noteNumber);

    // If note isn't already playing, find a free voice
    if (voiceIndex == -1)
    {
        voiceIndex = findFreeVoice();
    }

    // If no free voice, steal the oldest one
    if (voiceIndex == -1)
    {
        voiceIndex = findOldestVoice();
    }

    AudioNoInterrupts();
    waves[voiceIndex].begin(0.5, frequency, waveform);
    envs[voiceIndex].noteOn();
    AudioInterrupts();

    voices[voiceIndex].active = true;
    voices[voiceIndex].noteNumber = noteNumber;
    voices[voiceIndex].startTime = millis();
}

void PolySynth::noteOff(int noteNumber)
{
    int voiceIndex = findVoiceForNote(noteNumber);
    if (voiceIndex != -1)
    {
        envs[voiceIndex].noteOff();
        voices[voiceIndex].active = false;
        voices[voiceIndex].noteNumber = -1;
    }
}