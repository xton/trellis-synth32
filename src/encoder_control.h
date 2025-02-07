#pragma once

#include <Adafruit_seesaw.h>

#define SS_SWITCH 24 // this is the pin on the encoder connected to switch

class EncoderControl
{
private:
    Adafruit_seesaw encoder;
    bool buttonState = false;

public:
    EncoderControl() = default;

    // override these!
    virtual void buttonPushed()
    {
        Serial.println("generic button pushed");
    };
    virtual void inc()
    {
        Serial.println("generic inc");
    };
    virtual void decr()
    {
        Serial.println("generic decr");
    };

    // call these
    void begin(uint8_t addr)
    {
        while (!encoder.begin(addr))
        {
            Serial.printf("allocating encoder at address %d failed!\n", addr);
            delay(100);
        }
        encoder.pinMode(SS_SWITCH, INPUT_PULLUP);
    }

    void loop()
    {
        int32_t d = encoder.getEncoderDelta();
        while (d > 0)
        {
            inc();
            d--;
        }
        while (d < 0)
        {
            decr();
            d++;
        }
        bool b = encoder.digitalRead(SS_SWITCH);
        if (b != buttonState && b)
        {
            buttonPushed();
        }
        buttonState = b;
    }

    int32_t getPosition()
    {
        return encoder.getEncoderPosition();
    }
};