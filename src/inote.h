#pragma once
#include <Audio.h>

// Interface for all note types
class INote
{
public:
    virtual ~INote() = default; // Virtual destructor for proper cleanup

    // Pure virtual methods define the interface

    // Initialize the note's components. All oscillators must start with amplitude=0
    virtual void begin() = 0;

    // Start playing the note. Implementations should use fixed amplitude values
    virtual void noteOn() = 0;

    // Implementations must ensure oscillators are turned off (amplitude=0)
    // when note is released to save CPU cycles
    virtual void noteOff() = 0;

    // restore amplitude to 1 (or otherwise enable note to produce data)
    virtual void enable() = 0;

    // prevent note from using cpu
    virtual void disable() = 0;

    virtual void setFrequency(float freq) = 0;
    virtual AudioStream &getOutputLeft() = 0;
    virtual AudioStream &getOutputRight() = 0;

protected:
    // Protected constructor prevents instantiation of interface
    INote() = default;

    // Prevent copying of interface
    INote(const INote &) = delete;
    INote &operator=(const INote &) = delete;
};