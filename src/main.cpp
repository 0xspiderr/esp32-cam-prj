#include <Arduino.h>


#define FLASH_PIN 4
#define BAUD_RATE 115200

void setup()
{
    Serial.begin(BAUD_RATE);
    pinMode(FLASH_PIN, OUTPUT);
}

void loop()
{
    digitalWrite(FLASH_PIN, HIGH);
    delay(500);
    digitalWrite(FLASH_PIN, LOW);
    delay(500);
}