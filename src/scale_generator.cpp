#include "layer.h"
#include "scale_generator.h"

// Base frequencies for different octaves of note C
const float C0 = 16.35;  // C0
const float C1 = 32.70;  // C1
const float C2 = 65.41;  // C2
const float C3 = 130.81; // C3 (starting point for our scale)
const float C4 = 261.63; // Middle C

// Frequency ratios for intervals within an octave
const float SEMITONE = 1.059463094359; // 2^(1/12)
const float WHOLE_TONE = SEMITONE * SEMITONE;

// Collection of common scale patterns
const ScalePattern SCALE_PATTERNS[] = {
    {
        "Major",
        {1.0, WHOLE_TONE, WHOLE_TONE *WHOLE_TONE,
         WHOLE_TONE *WHOLE_TONE *SEMITONE,
         WHOLE_TONE *WHOLE_TONE *WHOLE_TONE,
         WHOLE_TONE *WHOLE_TONE *WHOLE_TONE *WHOLE_TONE,
         WHOLE_TONE *WHOLE_TONE *WHOLE_TONE *WHOLE_TONE *WHOLE_TONE,
         2.0} // Octave
    },
    {"Natural Minor",
     {1.0, WHOLE_TONE, SEMITONE *WHOLE_TONE,
      WHOLE_TONE *WHOLE_TONE,
      WHOLE_TONE *WHOLE_TONE *SEMITONE,
      WHOLE_TONE *SEMITONE *WHOLE_TONE,
      WHOLE_TONE *WHOLE_TONE *SEMITONE,
      2.0}},
    {"Pentatonic Major",
     {1.0, WHOLE_TONE, WHOLE_TONE *WHOLE_TONE,
      WHOLE_TONE *WHOLE_TONE *WHOLE_TONE,
      WHOLE_TONE *WHOLE_TONE *WHOLE_TONE *WHOLE_TONE,
      2.0 * WHOLE_TONE,
      2.0 * WHOLE_TONE *WHOLE_TONE,
      2.0 * WHOLE_TONE *WHOLE_TONE *WHOLE_TONE}},
    {"Pentatonic Minor",
     {1.0, WHOLE_TONE *SEMITONE,
      WHOLE_TONE *WHOLE_TONE,
      WHOLE_TONE *WHOLE_TONE *SEMITONE,
      WHOLE_TONE *WHOLE_TONE *WHOLE_TONE,
      2.0 * SEMITONE,
      2.0 * WHOLE_TONE,
      2.0 * WHOLE_TONE *SEMITONE}},
    {"Blues",
     {1.0, WHOLE_TONE *SEMITONE,
      WHOLE_TONE *WHOLE_TONE,
      WHOLE_TONE *WHOLE_TONE *SEMITONE,
      WHOLE_TONE *WHOLE_TONE *WHOLE_TONE,
      WHOLE_TONE *WHOLE_TONE *WHOLE_TONE *SEMITONE,
      2.0 * WHOLE_TONE,
      2.0 * WHOLE_TONE *SEMITONE}},
    {"Harmonic Minor",
     {1.0, WHOLE_TONE, SEMITONE *WHOLE_TONE,
      WHOLE_TONE *WHOLE_TONE,
      WHOLE_TONE *WHOLE_TONE *SEMITONE,
      WHOLE_TONE *SEMITONE *WHOLE_TONE,
      WHOLE_TONE *WHOLE_TONE *WHOLE_TONE,
      2.0}},
    {"Half-Whole Dim",
     {1.0, SEMITONE,
      SEMITONE *WHOLE_TONE,
      WHOLE_TONE *SEMITONE,
      WHOLE_TONE *SEMITONE *WHOLE_TONE,
      WHOLE_TONE *WHOLE_TONE *SEMITONE,
      WHOLE_TONE *WHOLE_TONE *SEMITONE *WHOLE_TONE,
      2.0}},
    {"Whole-Half Dim",
     {1.0, WHOLE_TONE,
      WHOLE_TONE *SEMITONE,
      WHOLE_TONE *SEMITONE *WHOLE_TONE,
      WHOLE_TONE *WHOLE_TONE *SEMITONE,
      WHOLE_TONE *WHOLE_TONE *SEMITONE *WHOLE_TONE,
      WHOLE_TONE *WHOLE_TONE *WHOLE_TONE *SEMITONE,
      2.0}},
    {"Phrygian",
     {1.0, SEMITONE,
      WHOLE_TONE,
      WHOLE_TONE *WHOLE_TONE,
      WHOLE_TONE *WHOLE_TONE *SEMITONE,
      WHOLE_TONE *SEMITONE *WHOLE_TONE,
      WHOLE_TONE *WHOLE_TONE *SEMITONE,
      2.0}},
    {
        "Hungarian Minor",
        {1.0, WHOLE_TONE,                                // Root to 2nd
         SEMITONE *WHOLE_TONE,                           // to minor 3rd
         WHOLE_TONE *WHOLE_TONE *SEMITONE,               // to raised 4th
         WHOLE_TONE *WHOLE_TONE *WHOLE_TONE,             // to 5th
         WHOLE_TONE *WHOLE_TONE *WHOLE_TONE *SEMITONE,   // to minor 6th
         WHOLE_TONE *WHOLE_TONE *WHOLE_TONE *WHOLE_TONE, // to major 7th
         2.0}                                            // to octave
    },
    {
        "Locrian",
        {1.0, SEMITONE,                                // Root to minor 2nd
         WHOLE_TONE,                                   // to minor 3rd
         WHOLE_TONE *WHOLE_TONE,                       // to 4th
         WHOLE_TONE *WHOLE_TONE *SEMITONE,             // to diminished 5th
         WHOLE_TONE *WHOLE_TONE *WHOLE_TONE,           // to minor 6th
         WHOLE_TONE *WHOLE_TONE *WHOLE_TONE *SEMITONE, // to minor 7th
         2.0}                                          // to octave
    },
    {
        "Chromatic",
        {1.0, SEMITONE,                                              // Half step
         SEMITONE *SEMITONE,                                         // Whole step
         SEMITONE *SEMITONE *SEMITONE,                               // Step and a half
         SEMITONE *SEMITONE *SEMITONE *SEMITONE,                     // Two steps
         SEMITONE *SEMITONE *SEMITONE *SEMITONE *SEMITONE,           // Two and a half
         SEMITONE *SEMITONE *SEMITONE *SEMITONE *SEMITONE *SEMITONE, // Three steps
         2.0}                                                        // Octave
    }};

const char *ROOT_NOTE_NAMES[] = {
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

// Root note frequencies (C through B)
const float ROOT_NOTES[] = {
    C3,                                                                      // C
    C3 *SEMITONE,                                                            // C#/Db
    C3 *WHOLE_TONE,                                                          // D
    C3 *WHOLE_TONE *SEMITONE,                                                // D#/Eb
    C3 *WHOLE_TONE *WHOLE_TONE,                                              // E
    C3 *WHOLE_TONE *WHOLE_TONE *SEMITONE,                                    // F
    C3 *WHOLE_TONE *WHOLE_TONE *WHOLE_TONE,                                  // F#/Gb
    C3 *WHOLE_TONE *WHOLE_TONE *WHOLE_TONE *SEMITONE,                        // G
    C3 *WHOLE_TONE *WHOLE_TONE *WHOLE_TONE *WHOLE_TONE,                      // G#/Ab
    C3 *WHOLE_TONE *WHOLE_TONE *WHOLE_TONE *WHOLE_TONE *SEMITONE,            // A
    C3 *WHOLE_TONE *WHOLE_TONE *WHOLE_TONE *WHOLE_TONE *WHOLE_TONE,          // A#/Bb
    C3 *WHOLE_TONE *WHOLE_TONE *WHOLE_TONE *WHOLE_TONE *WHOLE_TONE *SEMITONE // B
};

const size_t NUM_ROOTS = sizeof(ROOT_NOTES) / sizeof(ROOT_NOTES[0]);
const size_t NUM_SCALES = sizeof(SCALE_PATTERNS) / sizeof(SCALE_PATTERNS[0]);
