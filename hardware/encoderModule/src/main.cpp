#include <Arduino.h>
#include <Encoder.h>
#include <Wire.h>

Encoder myEnc(11, 12);

void I2C_RxHandler(int numBytes)
{
    while (Wire.available()) { // Read Any Received Data
        char c = Wire.read();
        Serial.print(c);
    }
}

void setup()
{
    Serial.begin(115200);
    pinMode(LED_BUILTIN, OUTPUT);

    Wire.begin(0x40);
    Wire.onReceive(I2C_RxHandler);
}

long oldPosition = -999;

// the loop function runs over and over again forever
void loop()
{
    // delay(1000);
    // Serial.println("yo");

    digitalWrite(LED_BUILTIN, LOW);

    long newPosition = myEnc.read();
    if (newPosition != oldPosition) {
        oldPosition = newPosition;
        Serial.print("Position: ");
        Serial.println(newPosition);
        digitalWrite(LED_BUILTIN, HIGH);
    }
}
