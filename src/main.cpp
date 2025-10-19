#include <Arduino.h>
#include "WebServer.h"
#include "WiFi.h"
#include "esp_camera.h"
#include "wifi_credentials.h"


#define FLASH_PIN 4
#define BAUD_RATE 115200


WiFiServer server(80);


void setup()
{
    Serial.begin(BAUD_RATE);

    // Wait for the serial port monitor to connect
    while (!Serial);

    Serial.print("Connecting to:");
    Serial.println(WIFI_SSID);
    WiFi.begin(WIFI_SSID, PASSWORD);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.print("Connected to wifi with IP address: ");
    Serial.println(WiFi.localIP());
    server.begin();
}

void loop()
{
    WiFiClient client = server.available();
    if (client) {
        Serial.println("HTTP client connected");
    }
}