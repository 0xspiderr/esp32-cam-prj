#include <Arduino.h>
#include "WebServer.h"
#include "WiFi.h"
#include "esp_camera.h"
#include "../lib/camera/camera.h"
#include "../lib/networking/networking.h"


#define FLASH_PIN 4
#define BAUD_RATE 115200


void setup()
{
    Serial.begin(BAUD_RATE);

    // wait for the serial port monitor to connect
    while (!Serial);

    // stop setup if camera couldn't initialize
    if (init_camera() != ESP_OK)
        return;

    init_wifi();
    init_server();
}

void loop()
{
    server.handleClient();
}