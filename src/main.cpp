/*****************************************************
 *  INCLUDES
 *****************************************************/
#include <HardwareSerial.h>
#include "camera.h"
#include "networking.h"
#include "soc/rtc_cntl_reg.h" // for disabling brownout detector


/*****************************************************
 *  DEFINES
 *****************************************************/
#define BAUD_RATE 115200


/*****************************************************
 *  BUILTIN ESP32 FUNCTIONS
 *****************************************************/
void setup()
{
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // disable brownout detector
    Serial.begin(BAUD_RATE);

    // wait for the serial port monitor to connect
    while (!Serial);

    // try to init camera
    esp_err_t err = init_camera();
    while (err != ESP_OK)
    {
        err = init_camera();
    }

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