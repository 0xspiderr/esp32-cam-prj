#ifndef QR_SCANNER_H
#define QR_SCANNER_H

/*****************************************************
 *  INCLUDES
 *****************************************************/
#include <esp32-hal-log.h>
#include <esp_camera.h>
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
static uint8_t *convert_jpeg_to_rgb565     (size_t, size_t, camera_fb_t *);
static uint8_t *convert_rgb565_to_grayscale(size_t, uint8_t *);
static void     decode_qr_from_grayscale   (size_t, size_t, uint8_t *, size_t);

#endif //QR_SCANNER_H

