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
    virtual void buttonDown()
    {
        Serial.println("generic button down");
    };

    virtual void buttonUp()
    {
        Serial.println("generic button up");
    };

    virtual void inc(int delta)
    {
        Serial.println("generic inc");
    };
    virtual void decr(int delta)
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

    bool isDown()
    {
        return !encoder.digitalRead(SS_SWITCH);
    }

    void loop()
    {
        int32_t d = encoder.getEncoderDelta();
        if (d > 0)
            inc(d);
        else if (d < 0)
            decr(-d);
        bool b = isDown();
        if (b != buttonState)
        {
            if (b)
                buttonDown();
            else
                buttonUp();
        }
        buttonState = b;
    }

    int32_t getPosition()
    {
        return encoder.getEncoderPosition();
    }
};