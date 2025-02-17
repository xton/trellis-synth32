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
#include <new>
#include <Fonts/TomThumb.h>
#include <Fonts/FreeMono9pt7b.h>

#include "menu.h"
#include "scale_generator.h"

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

Polysynth32 synthinstance;

AudioOutputAnalogStereo audioOut;

// delayed init of these connections
alignas(AudioConnection) byte bpol[sizeof(AudioConnection)];
alignas(AudioConnection) byte bpor[sizeof(AudioConnection)];
AudioConnection *patchOutLeft;
AudioConnection *patchOutRight;

void monitorUsage();

Adafruit_NeoTrellisM4 trellis = Adafruit_NeoTrellisM4();

#define ENCODER1_ADDR 0x36
#define ENCODER2_ADDR 0x37

// effects and postprocessing
GainFilter gainFilter;
BitCrusherFilter bitCrusherFilter;
LimiterFilter limiterFilter;
FeeedbackFilter feedbackFilter;
FlangeFilter flangeFilter;
ChorusFilter chorusFilter;

#define PERCENT_CONVERSION SIMPLE_LAMBDA(float f, (int)(f * 100))

auto volumeSetting =
    Setting("Vol: %d%%", 0.2f, 0.0f, 1.0f,
            SIMPLE_LAMBDA(float f, f + 0.05f),
            SIMPLE_LAMBDA(float f, f - 0.05f),
            PUBLISH_METHOD(gainFilter.gain, float),
            PERCENT_CONVERSION);

auto voiceSetting =
    Setting("Voice: %d", 0, 0, Polysynth32::LAYER_COUNT - 1,
            SIMPLE_LAMBDA(int i, i + 1),
            SIMPLE_LAMBDA(int i, i - 1),
            PUBLISH_METHOD(synthinstance.selectVoice, int));

auto scalePatternSetting = Setting<int>("Pat: %d", 0, 0, NUM_SCALES - 1,
                                        SIMPLE_LAMBDA(int i, i + 1),
                                        SIMPLE_LAMBDA(int i, i - 1),
                                        PUBLISH_METHOD(synthinstance.selectScalePattern, int), NULL,
                                        DISPLAY_LAMBDA(int i, {
                                          Serial.printf("set pat %d\n", i);
                                          gfx.setFont(&TomThumb);
                                          gfx.printf("%s", SCALE_PATTERNS[i].name);
                                        }));

auto scaleRootSetting = Setting<int>("Root: %d", 0, 0, NUM_ROOTS - 1,
                                     SIMPLE_LAMBDA(int i, i + 1),
                                     SIMPLE_LAMBDA(int i, i - 1),
                                     PUBLISH_METHOD(synthinstance.selectScaleRoot, int), NULL,
                                     DISPLAY_LAMBDA(int i, {
                                       Serial.printf("set root %d\n", i);
                                       gfx.printf("%s", ROOT_NOTE_NAMES[i]);
                                     }));

auto crusherBitsSetting =
    Setting("Bits: %d", 16, 2, 16,
            SIMPLE_LAMBDA(int i, i + 1),
            SIMPLE_LAMBDA(int i, i - 1),
            PUBLISH_METHOD(bitCrusherFilter.bits, int));

auto crusherSampleRateSetting =
    Setting("SR: %d", 44100, 690, 44100,
            SIMPLE_LAMBDA(int i, i * 2),
            SIMPLE_LAMBDA(int i, i / 2),
            PUBLISH_METHOD(bitCrusherFilter.sampleRate, int));

//

auto driveSetting = Setting("Drive: %d%%", 1.0f, 0.0f, 4.0f,
                            SIMPLE_LAMBDA(float f, f + 0.05f),
                            SIMPLE_LAMBDA(float f, f - 0.05f),
                            PUBLISH_METHOD(feedbackFilter.setDrive, float),
                            PERCENT_CONVERSION);

auto wetDrySetting = Setting("Wet/Dry: %d%%", 0.7f, 0.0f, 1.0f,
                             SIMPLE_LAMBDA(float f, f + 0.05f),
                             SIMPLE_LAMBDA(float f, f - 0.05f),
                             PUBLISH_METHOD(feedbackFilter.setWetDryMix, float),
                             PERCENT_CONVERSION);
//

auto delayRightSetting = Setting("DelayR: %d", 266, 0, 660,
                                 SIMPLE_LAMBDA(int i, i < 10 ? i + 1 : i * 130 / 100),
                                 SIMPLE_LAMBDA(int i, i * 100 / 130),
                                 PUBLISH_METHOD(feedbackFilter.setDelayRight, int));

auto delayLeftSetting = Setting("DelayL: %d", 399, 0, 660,
                                SIMPLE_LAMBDA(int i, i < 10 ? i + 1 : i * 130 / 100),
                                SIMPLE_LAMBDA(int i, i * 100 / 130),
                                PUBLISH_METHOD(feedbackFilter.setDelayLeft, int));

//

auto feedbackSetting = Setting("FB: %d%%", 0.4f, 0.0f, 0.9f,
                               SIMPLE_LAMBDA(float f, f + 0.05f),
                               SIMPLE_LAMBDA(float f, f - 0.05f),
                               PUBLISH_METHOD(feedbackFilter.setFeedback, float),
                               PERCENT_CONVERSION);

auto crossFeedbackSetting = Setting("XFB: %d%%", 0.2f, 0.0f, 0.5f,
                                    SIMPLE_LAMBDA(float f, f + 0.05f),
                                    SIMPLE_LAMBDA(float f, f - 0.05f),
                                    PUBLISH_METHOD(feedbackFilter.setCrossFeedback, float),
                                    PERCENT_CONVERSION);

//

auto filterFreqSetting = Setting("FFreq: %d", 1000, 400, 2000,
                                 SIMPLE_LAMBDA(int i, i + 10),
                                 SIMPLE_LAMBDA(int i, i - 10),
                                 PUBLISH_METHOD(feedbackFilter.setFilterFreq, int));

auto filterResSetting = Setting("FRes: %d%%", 0.7f, 0.0f, 4.0f,
                                SIMPLE_LAMBDA(float f, f + 0.05f),
                                SIMPLE_LAMBDA(float f, f - 0.05f),
                                PUBLISH_METHOD(feedbackFilter.setFilterRes, float),
                                PERCENT_CONVERSION);

// auto reverbSetting = Setting("Reverb: %d%%", 0.2f, 0.0f, 1.0f,
//                              SIMPLE_LAMBDA(float f, f + 0.05f),
//                              SIMPLE_LAMBDA(float f, f - 0.05f),
//                              PUBLISH_METHOD(reverbFilter.time, float),
//                              PERCENT_CONVERSION);

auto flangeSetting = Setting("Flange: %d", false, false, true,
                             SIMPLE_LAMBDA(bool b, !b),
                             SIMPLE_LAMBDA(bool b, !b),
                             PUBLISH_METHOD(flangeFilter.enable, bool));

auto chorusSetting = Setting("Chorus: %d", 1, 0, 10,
                             SIMPLE_LAMBDA(int i, i + 1),
                             SIMPLE_LAMBDA(int i, i - 1),
                             PUBLISH_METHOD(chorusFilter.voices, int));

auto presetSlide = PresetSlide(
    Preset("flat", {
      wetDrySetting.set(0);
      crusherBitsSetting.reset();
      crusherSampleRateSetting.reset();
      flangeSetting.reset();
      chorusSetting.reset();
    }),
    Preset("default", {
      crusherBitsSetting.reset();
      crusherSampleRateSetting.reset();
      driveSetting.reset();
      wetDrySetting.reset();
      delayRightSetting.reset();
      delayLeftSetting.reset();
      feedbackSetting.reset();
      crossFeedbackSetting.reset();
      filterFreqSetting.reset();
      filterResSetting.reset();
      flangeSetting.reset();
      chorusSetting.reset();
    }),
    Preset("whine", {
      driveSetting.set(4.0);
      wetDrySetting.set(0.6);
      delayLeftSetting.set(64);
      delayRightSetting.set(88);
      feedbackSetting.set(0.29);
      crossFeedbackSetting.set(0.2);
      filterFreqSetting.set(550);
      filterResSetting.set(3.5);
    }),
    Preset("screech", {
      crusherBitsSetting.set(3);
      driveSetting.set(4.0);
      wetDrySetting.set(0.6);
      delayLeftSetting.set(64);
      delayRightSetting.set(88);
      feedbackSetting.set(0.09);
      crossFeedbackSetting.set(0.05);
      filterFreqSetting.set(630);
      filterResSetting.set(3.5);
    }));

auto menu = Menu(display,
                 Slide(volumeSetting, voiceSetting, "main"),
                 Slide(scalePatternSetting, scaleRootSetting, "scale"),
                 Slide(crusherBitsSetting, crusherSampleRateSetting, "bit crusher"),

                 Slide(driveSetting, wetDrySetting, "drive"),
                 Slide(delayLeftSetting, delayRightSetting, "delay"),
                 Slide(feedbackSetting, crossFeedbackSetting, "feedback"),
                 Slide(filterFreqSetting, filterResSetting, "filter"),
                 Slide(flangeSetting, chorusSetting, "flange"),
                 presetSlide.getSlide());

class EncoderLeft : public EncoderControl
{
public:
  void inc(int delta) override
  {
    menu.leftInc(delta);
  }
  void decr(int delta) override { menu.leftDec(delta); }
  void buttonDown() override { menu.leftDown(); }
  void buttonUp() override { menu.leftUp(); }
};

class EncoderRight : public EncoderControl
{
public:
  void inc(int delta) override { menu.rightInc(delta); }
  void decr(int delta) override { menu.rightDec(delta); }
  void buttonDown() override { menu.rightDown(); }
  void buttonUp() override { menu.rightUp(); }
};

EncoderLeft encoder1;
EncoderRight encoder2;

void setup()
{

  Serial.begin(115200);
  // while (!Serial);

  trellis.begin();
  trellis.setBrightness(255);

  AudioMemory(200); // Increased mainly to support the delay effects

  // Initialize processor and memory measurements
  AudioProcessorUsageMaxReset();
  AudioMemoryUsageMaxReset();

  // delay(2000);
  // Serial.println("we start");
  synthinstance.begin();

  // setup all filters
  synthinstance.pushFilter(flangeFilter);
  synthinstance.pushFilter(chorusFilter);
  synthinstance.pushFilter(feedbackFilter);
  synthinstance.pushFilter(bitCrusherFilter);
  synthinstance.pushFilter(limiterFilter);
  synthinstance.pushFilter(gainFilter);

  // finally, connect the final output to sound out
  patchOutLeft = new (bpol) AudioConnection(synthinstance.getOutputLeft(), 0, audioOut, 0);
  patchOutRight = new (bpor) AudioConnection(synthinstance.getOutputRight(), 0, audioOut, 1);

  // Serial.println("synth started");

  encoder1.begin(ENCODER1_ADDR);
  // Serial.println("encoder 1");
  encoder2.begin(ENCODER2_ADDR);
  // Serial.println("encoder 2");

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  while (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    delay(100);
    Serial.println("SSD1306 allocation failed");
  }
  display.setRotation(2);
  display.clearDisplay();
  display.display();

  // getting weird corruption on first display otherwise
  menu.begin();
  delay(30);
  menu.display();

  Serial.println("setup done");
}

void noteOn(int num)
{
  synthinstance.noteOn(num);
}

void noteOff(int num)
{
  synthinstance.noteOff(num);
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

  if (presetSlide.tick())
  {
    menu.display();
  }

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