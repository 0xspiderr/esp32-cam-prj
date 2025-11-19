#ifndef QR_SCANNER_H
#define QR_SCANNER_H

/*****************************************************
 *  INCLUDES
 *****************************************************/
#include <WiFi.h>
#include <ArduinoJson.h>
#include <esp_http_client.h>
#include <esp32-hal-log.h>
#include <esp_camera.h>
#include <WString.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "../../.pio/libdeps/esp32cam/ESP32QRCodeReader/src/quirc/quirc.h"

/*****************************************************
 *  VARIABLES
 *****************************************************/
extern TaskHandle_t qr_scan_task;


/*****************************************************
 *  PROTOTYPES
 *****************************************************/
void            init_qr_scanner            (void);
void            trigger_qr_scan            (void);
void            scan_qr_code               (void *);
// internal functions used only in this translation unit
static uint8_t *convert_jpeg_to_rgb565     (size_t, size_t, camera_fb_t *);
static uint8_t *convert_rgb565_to_grayscale(size_t, uint8_t *);
static void     decode_qr_from_grayscale   (size_t, size_t, uint8_t *, size_t);
// internal networking methods for qr code
static void     process_url_data           (String);
static void     send_http_req              (String);
esp_err_t       http_event_handler         (esp_http_client_event_t *event);
void            parse_json_response        (String);
#endif //QR_SCANNER_H

