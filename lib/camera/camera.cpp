/*****************************************************
 *  INCLUDES
 *****************************************************/
#include "camera.h"

/*****************************************************
 *  DEFINITIONS
 *****************************************************/
void configure_camera()
{
    camera_config.ledc_channel = LEDC_CHANNEL_0;
    camera_config.ledc_timer = LEDC_TIMER_0;
    camera_config.pin_d0 = Y2_GPIO_NUM;
    camera_config.pin_d1 = Y3_GPIO_NUM;
    camera_config.pin_d2 = Y4_GPIO_NUM;
    camera_config.pin_d3 = Y5_GPIO_NUM;
    camera_config.pin_d4 = Y6_GPIO_NUM;
    camera_config.pin_d5 = Y7_GPIO_NUM;
    camera_config.pin_d6 = Y8_GPIO_NUM;
    camera_config.pin_d7 = Y9_GPIO_NUM;
    camera_config.pin_xclk = XCLK_GPIO_NUM;
    camera_config.pin_pclk = PCLK_GPIO_NUM;
    camera_config.pin_vsync = VSYNC_GPIO_NUM;
    camera_config.pin_href = HREF_GPIO_NUM;
    camera_config.pin_sccb_sda = SIOD_GPIO_NUM;
    camera_config.pin_sccb_scl = SIOC_GPIO_NUM;
    camera_config.pin_pwdn = PWDN_GPIO_NUM;
    camera_config.pin_reset = RESET_GPIO_NUM;
    camera_config.xclk_freq_hz = 20000000;
    camera_config.frame_size = FRAMESIZE_VGA;       // good frame size for streaming, SVGA would be another choice
    camera_config.pixel_format = PIXFORMAT_JPEG;    // good format for streaming, GRAYSCALE would be another choice
    camera_config.jpeg_quality = 20;                // lower number -> higher quality
    camera_config.fb_count = 2;                     // fb_count > 1 -> the driver works in continous mode
    camera_config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
}

esp_err_t init_camera()
{
    // power up the camera if the power down line gpio pin is defined
    if (PWDN_GPIO_NUM != -1)
    {
        pinMode(PWDN_GPIO_NUM, OUTPUT);
        digitalWrite(PWDN_GPIO_NUM, LOW);
    }

    setup_camera_flash_pwm();
    configure_camera();
    // initialize the camera and check for any errors during init
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "camera initialization failed");
        return err;
    }

    return ESP_OK;
}

/* camera flash functions */
void setup_camera_flash_pwm()
{
    // initialize camera flash with pwm
    ledcSetup(led_channel, frequency, cam_resolution);
    ledcAttachPin(FLASH_LED_GPIO_PIN, led_channel);
}

void set_flash_brightness(int duty_cycle)
{
    if (is_duty_range_ok(duty_cycle) == false)
        return;

    ledcWrite(led_channel, duty_cycle);
}

bool is_duty_range_ok(int duty_cycle)
{
    return duty_cycle >= 0 && duty_cycle <= 255;
}
/************************************************/
