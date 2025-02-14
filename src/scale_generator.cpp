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

// Base C3 frequency
#define FREQ_C3 130.81

// All frequency ratios from C3 to C4 (including accidentals)
#define RATIO_C3 1.0             // C3  (130.81 Hz)
#define RATIO_CS3 1.059463094359 // C#3 (138.59 Hz)
#define RATIO_D3 1.122462048309  // D3  (146.83 Hz)
#define RATIO_DS3 1.189207115002 // D#3 (155.56 Hz)
#define RATIO_E3 1.259921049895  // E3  (164.81 Hz)
#define RATIO_F3 1.334839854170  // F3  (174.61 Hz)
#define RATIO_FS3 1.414213562373 // F#3 (185.00 Hz)
#define RATIO_G3 1.498307076877  // G3  (196.00 Hz)
#define RATIO_GS3 1.587401051968 // G#3 (207.65 Hz)
#define RATIO_A3 1.681792830507  // A3  (220.00 Hz)
#define RATIO_AS3 1.781797436281 // A#3 (233.08 Hz)
#define RATIO_B3 1.887748625363  // B3  (246.94 Hz)
#define RATIO_C4 2.0             // C4  (261.63 Hz)

// Collection of common scale patterns
const ScalePattern SCALE_PATTERNS[] = {
    {
        "Original",
        {RATIO_C3,           // C3
         RATIO_D3,           // D3
         RATIO_E3,           // E3
         RATIO_G3,           // G3
         RATIO_A3,           // A3
         RATIO_C4,           // C4
         RATIO_C4 *RATIO_D3, // D4
         RATIO_C4 *RATIO_E3} // E4
    },
    {
        "Major",
        {RATIO_C3, // C3
         RATIO_D3, // D3
         RATIO_E3, // E3
         RATIO_F3, // F3
         RATIO_G3, // G3
         RATIO_A3, // A3
         RATIO_B3, // B3
         RATIO_C4} // C4
    },
    {
        "Natural Minor",
        {RATIO_C3,  // C3
         RATIO_D3,  // D3
         RATIO_DS3, // Eb3
         RATIO_F3,  // F3
         RATIO_G3,  // G3
         RATIO_GS3, // Ab3
         RATIO_AS3, // Bb3
         RATIO_C4}  // C4
    },
    {
        "Pentatonic Major",
        {RATIO_C3,           // C3
         RATIO_D3,           // D3
         RATIO_E3,           // E3
         RATIO_G3,           // G3
         RATIO_A3,           // A3
         RATIO_C4,           // C4
         RATIO_C4 *RATIO_D3, // D4
         RATIO_C4 *RATIO_E3} // E4
    },
    {
        "Pentatonic Minor",
        {RATIO_C3,            // C3
         RATIO_DS3,           // Eb3
         RATIO_F3,            // F3
         RATIO_G3,            // G3
         RATIO_AS3,           // Bb3
         RATIO_C4,            // C4
         RATIO_C4 *RATIO_DS3, // Eb4
         RATIO_C4 *RATIO_F3}  // F4
    },
    {
        "Blues",
        {RATIO_C3,            // C3
         RATIO_DS3,           // Eb3
         RATIO_F3,            // F3
         RATIO_FS3,           // F#3
         RATIO_G3,            // G3
         RATIO_AS3,           // Bb3
         RATIO_C4,            // C4
         RATIO_C4 *RATIO_DS3} // Eb4
    },
    {
        "Half-Whole Dim",
        {RATIO_C3,  // C3
         RATIO_CS3, // C#3
         RATIO_DS3, // D#3
         RATIO_E3,  // E3
         RATIO_FS3, // F#3
         RATIO_G3,  // G3
         RATIO_A3,  // A3
         RATIO_AS3} // A#3
    },
    {
        "Whole-Half Dim",
        {RATIO_C3,  // C3
         RATIO_D3,  // D3
         RATIO_DS3, // D#3
         RATIO_F3,  // F3
         RATIO_FS3, // F#3
         RATIO_GS3, // G#3
         RATIO_A3,  // A3
         RATIO_B3}  // B3
    },
    {
        "Phrygian",
        {RATIO_C3,  // C3
         RATIO_CS3, // Db3
         RATIO_DS3, // Eb3
         RATIO_F3,  // F3
         RATIO_G3,  // G3
         RATIO_GS3, // Ab3
         RATIO_AS3, // Bb3
         RATIO_C4}  // C4
    },
    {
        "Hungarian Minor",
        {RATIO_C3,  // C3
         RATIO_D3,  // D3
         RATIO_DS3, // Eb3
         RATIO_FS3, // F#3
         RATIO_G3,  // G3
         RATIO_GS3, // Ab3
         RATIO_B3,  // B3
         RATIO_C4}  // C4
    },
    {
        "Locrian",
        {RATIO_C3,  // C3
         RATIO_CS3, // Db3
         RATIO_DS3, // Eb3
         RATIO_F3,  // F3
         RATIO_FS3, // Gb3
         RATIO_GS3, // Ab3
         RATIO_AS3, // Bb3
         RATIO_C4}  // C4
    },
    {
        "Chromatic",
        {RATIO_C3,  // C3
         RATIO_CS3, // C#3
         RATIO_D3,  // D3
         RATIO_DS3, // D#3
         RATIO_E3,  // E3
         RATIO_F3,  // F3
         RATIO_FS3, // F#3
         RATIO_G3}  // G3
    } // end
};

const char *ROOT_NOTE_NAMES[] = {
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

// Root note frequencies (C through B)
const float ROOT_NOTES[] = {
    FREQ_C3,            // C
    FREQ_C3 *RATIO_CS3, // C#/Db
    FREQ_C3 *RATIO_D3,  // D
    FREQ_C3 *RATIO_DS3, // D#/Eb
    FREQ_C3 *RATIO_E3,  // E
    FREQ_C3 *RATIO_F3,  // F
    FREQ_C3 *RATIO_FS3, // F#/Gb
    FREQ_C3 *RATIO_G3,  // G
    FREQ_C3 *RATIO_GS3, // G#/Ab
    FREQ_C3 *RATIO_A3,  // A
    FREQ_C3 *RATIO_AS3, // A#/Bb
    FREQ_C3 *RATIO_B3   // B
};

const size_t NUM_ROOTS = sizeof(ROOT_NOTES) / sizeof(ROOT_NOTES[0]);
const size_t NUM_SCALES = sizeof(SCALE_PATTERNS) / sizeof(SCALE_PATTERNS[0]);
