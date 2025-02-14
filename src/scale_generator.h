#pragma once
#include "layer.h"

// Base frequencies for different octaves of note C
extern const float C0; // C0
extern const float C1; // C1
extern const float C2; // C2
extern const float C3; // C3 (starting point for our scale)
extern const float C4; // Middle C

// Frequency ratios for intervals within an octave
extern const float SEMITONE;
extern const float WHOLE_TONE;

// Scale interval patterns (steps between notes)
struct ScalePattern
{
    const char *name;
    const float intervals[8]; // 8 notes per row
};

extern const size_t NUM_ROOTS;
extern const size_t NUM_SCALES;

// Collection of common scale patterns
extern const ScalePattern SCALE_PATTERNS[];

// Root note frequencies (C through B)
extern const char *ROOT_NOTE_NAMES[];
extern const float ROOT_NOTES[];

class ScaleGenerator
{
private:
    float rootFreq;
    const ScalePattern *pattern;

public:
    ScaleGenerator(float rootFreq_ = C3, const ScalePattern *pattern_ = &SCALE_PATTERNS[0])
        : rootFreq(rootFreq_), pattern(pattern_) {}

    void setRoot(float root) { rootFreq = root; }
    void setPattern(const ScalePattern *newPattern) { pattern = newPattern; }

    // Fills the provided array with frequencies for the full 32-note grid
    void generateFullScale(float *frequencies)
    {
        Serial.printf("Gen for pattern: %s", pattern->name);

        // For each row
        for (size_t row = 0; row < ILayer::ROW_COUNT; row++)
        {
            Serial.printf("\nR%d: ", row);
            float octaveMult = (1 << row); // 1, 2, 4, 8 for each row

            // For each column in the row
            for (size_t col = 0; col < ILayer::NOTES_PER_ROW; col++)
            {
                size_t index = row * ILayer::NOTES_PER_ROW + col;
                frequencies[index] = rootFreq * pattern->intervals[col] * octaveMult;
                Serial.printf(" %d", (int)frequencies[index]);
            }
        }

        Serial.println("");
    }

    const char *getCurrentScaleName() const
    {
        return pattern->name;
    }
};