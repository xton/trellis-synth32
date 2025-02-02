/* Simple Audio library demonstration - pocket synth with C major scale and 4 wave types */

#include <Arduino.h>

#include <Audio.h>
#include <Adafruit_NeoTrellisM4.h>
#include "polysynth.h"

#ifdef __SAMD51__
// Define strong symbols for these handlers to avoid them being clobbered by the weak symbols in coretex_handlers.c
extern "C" void DMAC_0_Handler();
void DMAC_1_Handler(void) { DMAC_0_Handler(); }
void DMAC_2_Handler(void) { DMAC_0_Handler(); }
void DMAC_3_Handler(void) { DMAC_0_Handler(); }
void DMAC_4_Handler(void) { DMAC_0_Handler(); }
#endif

uint32_t Wheel(byte WheelPos);

PolySynth synth;

short wave_type[4] = {
    WAVEFORM_SINE,
    WAVEFORM_SQUARE,
    WAVEFORM_SAWTOOTH,
    WAVEFORM_TRIANGLE,
};

float cmaj_low[8] = {130.81, 146.83, 164.81, 174.61, 196.00, 220.00, 246.94, 261.63};
float cmaj_high[8] = {261.6, 293.7, 329.6, 349.2, 392.0, 440.0, 493.9, 523.3};

AudioEffectDelay delay1;
AudioMixer4 mixerLeft;
AudioMixer4 mixerRight;
AudioOutputAnalogStereo audioOut;

// Connect synth output to delay and stereo mix
AudioConnection patchCord1(synth.getOutputMixer(), 0, delay1, 0);
AudioConnection patchCord2(synth.getOutputMixer(), 0, mixerLeft, 0);
AudioConnection patchCord3(synth.getOutputMixer(), 0, mixerRight, 0);
AudioConnection patchCord4(delay1, 0, mixerLeft, 1);
AudioConnection patchCord5(delay1, 1, mixerLeft, 2);
AudioConnection patchCord6(delay1, 2, mixerRight, 1);
AudioConnection patchCord7(delay1, 3, mixerRight, 2);
AudioConnection patchCord8(mixerLeft, 0, audioOut, 0);
AudioConnection patchCord9(mixerRight, 0, audioOut, 1);

Adafruit_NeoTrellisM4 trellis = Adafruit_NeoTrellisM4();

void setup()
{
  Serial.begin(115200);

  trellis.begin();
  trellis.setBrightness(255);

  AudioMemory(180); // Increased for 8-voice polyphony

  // Initialize processor and memory measurements
  AudioProcessorUsageMaxReset();
  AudioMemoryUsageMaxReset();

  synth.begin();

  // Set up stereo mix
  mixerLeft.gain(1, 0.36);
  mixerLeft.gain(3, 0.36);
  mixerRight.gain(0, 0.36);
  mixerRight.gain(2, 0.36);

  // Set up delay
  delay1.delay(0, 110);
  delay1.delay(1, 220);
  delay1.delay(2, 660);

  Serial.println("setup done");
}

void noteOn(int num)
{
  int voice = num / 8;
  float *scale = (voice < 2) ? cmaj_low : cmaj_high;
  float freq = scale[num % 8];
  synth.noteOn(num, freq, wave_type[voice % 4]);
}

void noteOff(int num)
{
  synth.noteOff(num);
}

void loop()
{
  // Monitor audio system resources
  if (millis() % 5000 == 0)
  { // Print every 5 seconds
    Serial.print("Memory Usage: ");
    Serial.print(AudioMemoryUsage());
    Serial.print(" blocks (max: ");
    Serial.print(AudioMemoryUsageMax());
    Serial.println(")");

    Serial.print("CPU Usage: ");
    Serial.print(AudioProcessorUsage());
    Serial.print("% (max: ");
    Serial.print(AudioProcessorUsageMax());
    Serial.println("%)");
  }

  trellis.tick();

  while (trellis.available())
  {
    keypadEvent e = trellis.read();
    int keyindex = e.bit.KEY;
    if (e.bit.EVENT == KEY_JUST_PRESSED)
    {
      trellis.setPixelColor(keyindex, Wheel(keyindex * 255 / 32));
      noteOn(keyindex);
    }
    else if (e.bit.EVENT == KEY_JUST_RELEASED)
    {
      noteOff(keyindex);
      trellis.setPixelColor(keyindex, 0);
    }
  }
  delay(10);
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos)
{
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85)
  {
    return Adafruit_NeoPixel::Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170)
  {
    WheelPos -= 85;
    return Adafruit_NeoPixel::Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return Adafruit_NeoPixel::Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}