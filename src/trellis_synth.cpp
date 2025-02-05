/* Simple Audio library demonstration - pocket synth with C major scale and 4 wave types */

#include <Arduino.h>
#include <Audio.h>
#include <Adafruit_NeoTrellisM4.h>
#include "polysynth32.h"

#ifdef __SAMD51__
// Define strong symbols for these handlers to avoid them being clobbered by the weak symbols in coretex_handlers.c
extern "C" void DMAC_0_Handler();
void DMAC_1_Handler(void) { DMAC_0_Handler(); }
void DMAC_2_Handler(void) { DMAC_0_Handler(); }
void DMAC_3_Handler(void) { DMAC_0_Handler(); }
void DMAC_4_Handler(void) { DMAC_0_Handler(); }
#endif

uint32_t Wheel(byte WheelPos);

Polysynth32 synth;

AudioOutputAnalogStereo audioOut;

AudioConnection patchCord8(synth.getOutputLeft(), 0, audioOut, 0);
AudioConnection patchCord9(synth.getOutputRight(), 0, audioOut, 1);

Adafruit_NeoTrellisM4 trellis = Adafruit_NeoTrellisM4();

void setup()
{
  Serial.begin(115200);
  // while (!Serial);

  trellis.begin();
  trellis.setBrightness(255);

  AudioMemory(320); // Increased mainly to support the delay effects

  // Initialize processor and memory measurements
  AudioProcessorUsageMaxReset();
  AudioMemoryUsageMaxReset();

  synth.begin();

  Serial.println("setup done");
}

void noteOn(int num)
{
  synth.noteOn(num);
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