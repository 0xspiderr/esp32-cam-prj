/*****************************************************
 *  INCLUDES
 *****************************************************/
#include "camera.h"
#include "../networking/networking.h"
#include <JPEGDEC.h>

/*****************************************************
 *  VARIABLES
 *****************************************************/
camera_config_t camera_config;
// for error logging
static const char *TAG = "CAMERA";
// camera flash pwm config
const int frequency       = 5000;
const int led_channel     = 0; // if using pwm with the camera and esp32 at the same time, use another channel
const int cam_resolution  = 8;
bool      flash_state     = false;
bool      grayscale_state = false;


/*****************************************************
 *  DEFINITIONS
 *****************************************************/
void configure_camera()
{
    camera_config.ledc_channel = LEDC_CHANNEL_0;
    camera_config.ledc_timer   = LEDC_TIMER_0;
    camera_config.pin_d0       = Y2_GPIO_NUM;
    camera_config.pin_d1       = Y3_GPIO_NUM;
    camera_config.pin_d2       = Y4_GPIO_NUM;
    camera_config.pin_d3       = Y5_GPIO_NUM;
    camera_config.pin_d4       = Y6_GPIO_NUM;
    camera_config.pin_d5       = Y7_GPIO_NUM;
    camera_config.pin_d6       = Y8_GPIO_NUM;
    camera_config.pin_d7       = Y9_GPIO_NUM;
    camera_config.pin_xclk     = XCLK_GPIO_NUM;
    camera_config.pin_pclk     = PCLK_GPIO_NUM;
    camera_config.pin_vsync    = VSYNC_GPIO_NUM;
    camera_config.pin_href     = HREF_GPIO_NUM;
    camera_config.pin_sccb_sda = SIOD_GPIO_NUM;
    camera_config.pin_sccb_scl = SIOC_GPIO_NUM;
    camera_config.pin_pwdn     = PWDN_GPIO_NUM;
    camera_config.pin_reset    = RESET_GPIO_NUM;
    camera_config.xclk_freq_hz = 20000000;
    camera_config.frame_size   = FRAMESIZE_QVGA;      // good frame size for streaming, SVGA/QVGA would be another choice
    camera_config.jpeg_quality = 10;                  // lower number -> higher quality
    camera_config.fb_count     = 1;                   // fb_count > 1 -> the driver works in continous mode
    camera_config.grab_mode    = CAMERA_GRAB_WHEN_EMPTY;
    camera_config.fb_location  = CAMERA_FB_IN_PSRAM;
}

esp_err_t init_camera(pixformat_t format)
{
    // power up the camera if the power down line gpio pin is defined
    if (PWDN_GPIO_NUM != -1)
    {
        pinMode(PWDN_GPIO_NUM, OUTPUT);
        digitalWrite(PWDN_GPIO_NUM, LOW);
    }

    setup_camera_flash_pwm();
    configure_camera();
    camera_config.pixel_format = format;
    // initialize the camera and check for any errors during init
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "camera initialization failed");
        return err;
    }

    // flip image because it's shown upside down and flipped horizontally
    sensor_t *sensor = esp_camera_sensor_get();
    sensor->set_vflip(sensor, 1);
    sensor->set_hmirror(sensor, 1);
    sensor->set_brightness(sensor, 1);

    return ESP_OK;
}


/* Camera flash functions *****************************/
static void setup_camera_flash_pwm()
{
    // initialize camera flash with pwm
    ledcSetup(led_channel, frequency, cam_resolution);
    ledcAttachPin(FLASH_LED_GPIO_PIN, led_channel);
}


static void set_flash_brightness(int duty_cycle)
{
    if (is_duty_range_ok(duty_cycle) == false)
        return;

    ledcWrite(led_channel, duty_cycle);
}


void toggle_camera_flash()
{
    sensor_t *sensor = esp_camera_sensor_get();
    sensor->set_special_effect(sensor, NO_EFFECT);
    flash_state = !flash_state;
    if (flash_state == true)
        set_flash_brightness(128);
    else
        set_flash_brightness(0);
}


void toggle_grayscale(bool *stream_paused)
{
    sensor_t *s = esp_camera_sensor_get();
    if (!s) {
        ESP_LOGE(TAG, "Camera sensor not found");
        return;
    }

    *stream_paused = true;

    esp_camera_deinit();
    if (s->pixformat == PIXFORMAT_JPEG)
        init_camera(PIXFORMAT_GRAYSCALE);
    else
        init_camera(PIXFORMAT_JPEG);

    delay(5000);
    *stream_paused = false;

}

void capture_decode_and_quirc(void *arg) {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb || fb->format != PIXFORMAT_JPEG) {
        Serial.println("Capture failed or format is not JPEG!");
        return;
    }

    size_t width = fb->width;
    size_t height = fb->height;
    size_t rgb565_size = width * height * 2;
    uint8_t *rgb565_buffer = (uint8_t *)heap_caps_malloc(rgb565_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!rgb565_buffer) {
        Serial.println("PSRAM allocation failed for RGB565 buffer!");
        esp_camera_fb_return(fb);
        return;
    }

    // decode jpeg to rgb565
    bool decode_success = jpg2rgb565(
        (const uint8_t *)fb->buf,
        fb->len,
        rgb565_buffer,
        JPG_SCALE_NONE
    );
    esp_camera_fb_return(fb);

    if (!decode_success) {
        Serial.println("JPEG to RGB565 decoding FAILED!");
        free(rgb565_buffer);
        return;
    }

    // convert rgb565 to grayscale
    size_t gray_size = width * height;
    uint8_t *gray_buffer = (uint8_t *)heap_caps_malloc(gray_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

    if (!gray_buffer) {
        Serial.println("PSRAM allocation failed for Grayscale buffer!");
        free(rgb565_buffer);
        return;
    }
    for (size_t i = 0; i < gray_size; i++) {
        uint16_t pixel = ((uint16_t *)rgb565_buffer)[i];

        uint8_t R = (pixel >> 11) & 0x1F;
        uint8_t G = (pixel >> 5) & 0x3F;
        uint8_t B = pixel & 0x1F;

        uint8_t r8 = R << 3;
        uint8_t g8 = G << 2;
        uint8_t b8 = B << 3;

        uint8_t gray_value = (uint8_t)((30 * r8 + 59 * g8 + 11 * b8) / 100);
        gray_buffer[i] = gray_value;
    }

    free(rgb565_buffer);

    // decode qr code
    struct quirc *q = quirc_new();
    if (!q) {
        Serial.println("Failed to allocate quirc object!");
        free(gray_buffer);
        return;
    }

    if (quirc_resize(q, width, height) < 0) {
        Serial.println("Failed to resize quirc buffer!");
        quirc_destroy(q);
        free(gray_buffer);
        return;
    }
    uint8_t *quirc_image = quirc_begin(q, NULL, NULL);
    memcpy(quirc_image, gray_buffer, gray_size);
    quirc_end(q);
    free(gray_buffer);

    int count = quirc_count(q);
    Serial.printf("Found %d potential QR codes.\n", count);

    for (int i = 0; i < count; i++) {
        struct quirc_code code;
        struct quirc_data data;

        quirc_extract(q, i, &code);

        quirc_decode_error_t err = quirc_decode(&code, &data);

        if (err == 0) {
            Serial.printf("  Version: %d\n", data.version);
            Serial.printf("  Data Type: %d (0=Numeric, 1=Alphanumeric, 2=Byte, 3=Kanji)\n", data.data_type);
            Serial.printf("  Payload (%u bytes): %s\n", data.payload_len, data.payload);
        } else {
            Serial.printf("QR Code decoding FAILED for code #%d. Error: %s\n", i + 1, quirc_strerror(err));
        }
    }
    quirc_destroy(q);
    Serial.println("Scan complete.");

    vTaskDelete(NULL);
}


static bool is_duty_range_ok(int duty_cycle)
{
    return duty_cycle >= LOWER_DUTY_LIMIT && duty_cycle <= UPPER_DUTY_LIMIT;
}
/******************************************************/
