/* Simple Audio library demonstration - pocket synth with C major scale and 4 wave types */

#include <Arduino.h>
#include <Audio.h>
#include <Adafruit_NeoTrellisM4.h>
#include <Adafruit_seesaw.h>
#include "polysynth32.h"
#include "encoder_control.h"

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeSansBold12pt7b.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define SS_SWITCH 24     // this is the pin on the encoder connected to switch

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library.
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET -1       // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C // ours is this (shrug)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

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
void monitorUsage();

Adafruit_NeoTrellisM4 trellis = Adafruit_NeoTrellisM4();

#define ENCODER1_ADDR 0x36
#define ENCODER2_ADDR 0x37

class EncoderLeft : public EncoderControl
{
public:
  void displayLed()
  {
    display.clearDisplay();
    display.setTextSize(1);
    display.setFont(&FreeSansBold12pt7b);
    display.setTextColor(SSD1306_WHITE); // Draw white text
    display.setCursor(0, 32);
    display.printf("Enc1: %d\n", getPosition());
    display.display();
  }

  void inc() override
  {
    Serial.println("left up");
    displayLed();
  }
  void decr() override
  {
    Serial.println("left down");
    displayLed();
  }
  void buttonPushed() override { Serial.println("left pdushed"); }
};

class EncoderRight : public EncoderControl
{
public:
  void inc() override { Serial.println("right up"); }
  void decr() override { Serial.println("right down"); }
  void buttonPushed() override { Serial.println("right pushed"); }
};

EncoderLeft encoder1;
EncoderRight encoder2;

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

  encoder1.begin(ENCODER1_ADDR);
  encoder2.begin(ENCODER2_ADDR);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  while (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    delay(100);
    Serial.println("SSD1306 allocation failed");
  }
  display.setRotation(2);

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

  monitorUsage();

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

  encoder1.loop();
  encoder2.loop();

  delay(10);
}

void monitorUsage()
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

    // uint32_t version1 = ((encoder1.getVersion() >> 16) & 0xFFFF);
    // uint32_t version2 = ((encoder2.getVersion() >> 16) & 0xFFFF);

    // Serial.printf("Versions: %d, %d\n", version1, version2);
    // Serial.printf("Deltas: %d %d\n", encoder1.getEncoderDelta(), encoder2.getEncoderDelta());
    // Serial.printf("Positions: %d %d\n", encoder1.getEncoderPosition(), encoder2.getEncoderPosition());

    lastPrint = now;
  }
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