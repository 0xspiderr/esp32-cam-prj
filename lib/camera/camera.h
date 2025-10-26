#ifndef ESP32_CAM_PRJ_CAMERA_H
#define ESP32_CAM_PRJ_CAMERA_H

/*****************************************************
 *  INCLUDES
 *****************************************************/
#include <esp_camera.h>
#include <esp_err.h>
#include <esp32-hal-gpio.h>


/*****************************************************
 *  DEFINES
 *****************************************************/
#define CAMERA_MODEL_AI_THINKER

// Camera specific config pins
#define PWDN_GPIO_NUM      32
#define RESET_GPIO_NUM     -1
#define XCLK_GPIO_NUM       0
#define SIOD_GPIO_NUM      26
#define SIOC_GPIO_NUM      27
#define Y9_GPIO_NUM        35
#define Y8_GPIO_NUM        34
#define Y7_GPIO_NUM        39
#define Y6_GPIO_NUM        36
#define Y5_GPIO_NUM        21
#define Y4_GPIO_NUM        19
#define Y3_GPIO_NUM        18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM     25
#define HREF_GPIO_NUM      23
#define PCLK_GPIO_NUM      22
#define FLASH_LED_GPIO_PIN 4

// duty cycle defines
#define UPPER_DUTY_LIMIT 255
#define LOWER_DUTY_LIMIT 0

/*****************************************************
 *  VARIABLES
 *****************************************************/
// for error logging
static camera_config_t camera_config;

// camera flash pwm config
static const int frequency = 5000;
static const int led_channel = 0; // if using pwm with the camera and esp32 at the same time, use another channel
static const int cam_resolution = 8;
static bool flash_state = false;

/*****************************************************
 *  PROTOTYPES
 *****************************************************/
void        configure_camera      (void);
static void setup_camera_flash_pwm(void);
static void set_flash_brightness  (int);
void        toggle_camera_flash   (void);
static bool is_duty_range_ok      (int);
esp_err_t   init_camera           (void);

#endif //ESP32_CAM_PRJ_CAMERA_H