#include <Arduino.h>
#include "WebServer.h"
#include "WiFi.h"
#include "esp_camera.h"
#include "../lib/camera/camera.h"
#include "../lib/networking/networking.h"


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
    init_server(); // if the connection to the wifi was successfuly established, initialize the server
}

void loop()
{
    // for (int i = 0; i < 256; ++i) {
    //     set_flash_brightness(i);
    //     delay(20);
    // }
    // for (int i = 255; i > 0; --i) {
    //     set_flash_brightness(i);
    //     delay(20);
    // }
}