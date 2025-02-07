#pragma once
#include <Audio.h>
#include "inote.h"

class ISynthRow
{
public:
    static const int NOTES_PER_ROW = 8;
    virtual ~ISynthRow() = default;
    virtual void begin() = 0;
    virtual void noteOn(int index) = 0;
    virtual void noteOff(int index) = 0;
    virtual AudioStream &getOutputLeft() = 0;
    virtual AudioStream &getOutputRight() = 0;
    virtual void setScale(const float *frequencies) = 0;

protected:
    // Protected constructor prevents instantiation of interface
    ISynthRow() = default;

    // Prevent copying of interface
    ISynthRow(const ISynthRow &) = delete;
    ISynthRow &operator=(const ISynthRow &) = delete;
};

// Each row manages 8 notes of the same type
template <typename T>
class SynthRow : public ISynthRow
{
    // static_assert(std::is_base_of<INote, T>::value, "T must implement INote interface");

private:
    T notes[NOTES_PER_ROW];
    AudioMixer4 mixLeft1;   // Mixes notes 0-3
    AudioMixer4 mixLeft2;   // Mixes notes 4-7
    AudioMixer4 finalLeft;  // Combines left mixers
    AudioMixer4 mixRight1;  // Mixes notes 0-3
    AudioMixer4 mixRight2;  // Mixes notes 4-7
    AudioMixer4 finalRight; // Combines right mixers

    // Audio connections for note outputs to first layer mixers
    AudioConnection patchL1_0;
    AudioConnection patchL1_1;
    AudioConnection patchL1_2;
    AudioConnection patchL1_3;
    AudioConnection patchL2_0;
    AudioConnection patchL2_1;
    AudioConnection patchL2_2;
    AudioConnection patchL2_3;
    AudioConnection patchR1_0;
    AudioConnection patchR1_1;
    AudioConnection patchR1_2;
    AudioConnection patchR1_3;
    AudioConnection patchR2_0;
    AudioConnection patchR2_1;
    AudioConnection patchR2_2;
    AudioConnection patchR2_3;

    // Audio connections for mixer routing
    AudioConnection patchLF1;
    AudioConnection patchLF2;
    AudioConnection patchRF1;
    AudioConnection patchRF2;

public:
    SynthRow();
    void begin();
    void noteOn(int index);
    void noteOff(int index);
    AudioStream &getOutputLeft() { return finalLeft; }
    AudioStream &getOutputRight() { return finalRight; }
    void setScale(const float *frequencies);
};