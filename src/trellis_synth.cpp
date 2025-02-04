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
    WAVEFORM_SAWTOOTH_REVERSE,
    WAVEFORM_SQUARE,
    WAVEFORM_SAWTOOTH,
    WAVEFORM_SQUARE,
};

// C major pentatonic: C, D, E, G, A (then C, D, E again up octave)
float cmaj_pent_low[8] = {
    130.81, // C
    146.83, // D
    164.81, // E
    196.00, // G
    220.00, // A
    261.63, // C (8va)
    293.66, // D (8va)
    329.63  // E (8va)
};

float cmaj_pent_high[8] = {
    261.63, // C
    293.66, // D
    329.63, // E
    392.00, // G
    440.00, // A
    523.25, // C (8va)
    587.33, // D (8va)
    659.26  // E (8va)
};

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
  // while (!Serial);

  trellis.begin();
  trellis.setBrightness(255);

  AudioMemory(180); // Increased for 8-voice polyphony

  // Initialize processor and memory measurements
  AudioProcessorUsageMaxReset();
  AudioMemoryUsageMaxReset();

  synth.begin();

  // reduce the gain on some channels, so half of the channels
  // are "positioned" to the left, half to the right, but all
  // are heard at least partially on both ears
  mixerLeft.gain(1, 0.36);
  mixerLeft.gain(3, 0.36);
  mixerRight.gain(0, 0.36);
  mixerRight.gain(2, 0.36);

  // set up delay effect
  delay1.delay(0, 110);
  delay1.delay(1, 220);
  delay1.delay(2, 660);

  Serial.println("setup done");
}

void noteOn(int num)
{
  int voice = num / 8;
  float *scale = (voice < 2) ? cmaj_pent_low : cmaj_pent_high;
  float freq = scale[num % 8];
  synth.noteOn(num, freq, wave_type[voice % 4]);
}

void noteOff(int num)
{
  synth.noteOff(num);
}

void loop()
{

  static uint32_t lastPrint = 0;
  uint32_t now = millis();

  // Monitor audio system resources every 5 seconds
  if (now - lastPrint >= 5000)
  {
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

    lastPrint = now;
  }

  // The trellis library has a nasty hack that attempts to
  // suppress events from all 4 buttons on a column being hit at once.
  // This is something the elastometer is prone to doing accidentally and it can be kind of annoying...
  // however, it's very rare and my users actually want to hold down a whole row at once. Additionally
  // the implementation is poor and leaves stuck keys.
  // trellis.tick();
  static_cast<Adafruit_Keypad &>(trellis).tick();

  while (trellis.available())
  {
    keypadEvent e = trellis.read();
    int keyindex = e.bit.KEY;
    if (e.bit.EVENT == KEY_JUST_PRESSED)
    {
      // trellis.setPixelColor(keyindex, 0xFFFFFF); // plain white
      trellis.setPixelColor(keyindex, Wheel(keyindex * 255 / 32)); // rainbow!
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