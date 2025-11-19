/*****************************************************
*  INCLUDES
 *****************************************************/
#include "qr_scanner.h"
#include <string.h>

/*****************************************************
 *  VARIABLES
 *****************************************************/
static const char* TAG = "qr_scanner";
// FreeRTOS tasks
TaskHandle_t qr_scan_task = NULL;


/*****************************************************
 *  DEFINITIONS
 *****************************************************/
void init_qr_scanner() {
    if (qr_scan_task == NULL) {
        BaseType_t result = xTaskCreatePinnedToCore(
            scan_qr_code,
            "scan_qr_code",
            20000,
            NULL,
            5,
            &qr_scan_task,
            1);

        if (result == pdPASS) {
            // Task starts running immediately, so suspend it
            vTaskSuspend(qr_scan_task);
            ESP_LOGI(TAG, "QR scan task created and suspended");
        } else {
            ESP_LOGE(TAG, "Failed to create QR scan task");
            qr_scan_task = NULL;
        }
    }
}

void trigger_qr_scan() {
    if (qr_scan_task != NULL) {
        vTaskResume(qr_scan_task);
    } else {
        ESP_LOGE(TAG, "QR scan task not initialized");
    }
}

void scan_qr_code(void *arg) {
    // initially suspend this task after initializing it
    vTaskSuspend(NULL);

    for (;;)
    {
        camera_fb_t *fb = esp_camera_fb_get();
        uint8_t *rgb565_buffer = NULL;
        uint8_t *gray_buffer = NULL;
        size_t width = 0;
        size_t height = 0;
        size_t gray_size = 0;

        if (!fb || fb->format != PIXFORMAT_JPEG) {
            ESP_LOGI(TAG, "No jpeg frame");
            if (fb)
                esp_camera_fb_return(fb);
            goto memory_cleanup;
        }

        // convert jpeg to rgb565
        width = fb->width;
        height = fb->height;
        rgb565_buffer = convert_jpeg_to_rgb565(width, height, fb);
        fb = NULL;
        if (!rgb565_buffer)
        {
            ESP_LOGE(TAG, "Error returning buffer from jpeg to rgb565");
            goto memory_cleanup;
        }

        // convert rgb565 to grayscale
        gray_size = width * height;
        gray_buffer = convert_rgb565_to_grayscale(gray_size, rgb565_buffer);
        rgb565_buffer = NULL;
        if (!gray_buffer)
        {
            ESP_LOGE(TAG, "Error returning buffer from rgb565 to grayscale");
            goto memory_cleanup;
        }

        // decode qr code
        decode_qr_from_grayscale(width, height, gray_buffer, gray_size);
        gray_buffer = NULL;

        memory_cleanup:
            // cleanup non freed memory
            if (fb)
                esp_camera_fb_return(fb);
            if (rgb565_buffer)
                free(rgb565_buffer);
            if (gray_buffer)
                free(gray_buffer);
            ESP_LOGI(TAG, "QR decoding finished");
            vTaskSuspend(NULL);
    }
}

static uint8_t *convert_jpeg_to_rgb565(size_t width, size_t height, camera_fb_t *fb)
{
    size_t rgb565_size = width * height * 2;
    uint8_t *rgb565_buffer = (uint8_t *)heap_caps_malloc(rgb565_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

    if (!rgb565_buffer)
    {
        ESP_LOGE(TAG, "PSRAM allocation failed for RGB565 buffer!");
        esp_camera_fb_return(fb);
        return NULL;
    }

    // decode jpeg to rgb565
    bool decode_success = jpg2rgb565(
        fb->buf,
        fb->len,
        rgb565_buffer,
        JPG_SCALE_NONE
    );
    esp_camera_fb_return(fb);

    if (!decode_success) {
        ESP_LOGE(TAG, "JPEG to RGB565 decoding FAILED!");
        free(rgb565_buffer);
        return NULL;
    }

    return rgb565_buffer;
}

static uint8_t *convert_rgb565_to_grayscale(size_t gray_size, uint8_t *rgb565_buffer)
{
    uint8_t *gray_buffer = (uint8_t *)heap_caps_malloc(gray_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

    if (!gray_buffer)
    {
        ESP_LOGE(TAG, "PSRAM allocation failed for Grayscale buffer!");
        return NULL;
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

    return gray_buffer;
}

static void decode_qr_from_grayscale(size_t width, size_t height, uint8_t *gray_buffer, size_t gray_size)
{
    struct quirc *q = quirc_new();
    if (!q) {
        ESP_LOGE(TAG, "Failed to allocate quirc object!");
        free(gray_buffer);
        return;
    }

    if (quirc_resize(q, width, height) < 0) {
        ESP_LOGE(TAG, "Failed to resize quirc buffer!");
        quirc_destroy(q);
        free(gray_buffer);
        return;
    }
    uint8_t *quirc_image = quirc_begin(q, NULL, NULL);

    memcpy(quirc_image, gray_buffer, gray_size);
    quirc_end(q);
    free(gray_buffer);

    int count = quirc_count(q);
    ESP_LOGI(TAG, "Found %d potential QR codes.\n", count);

    for (int i = 0; i < count; i++)
    {
        struct quirc_code code;
        struct quirc_data data;

        quirc_extract(q, i, &code);

        quirc_decode_error_t err = quirc_decode(&code, &data);

        if (err == 0)
        {
            ESP_LOGI(TAG, "  Version: %d\n", data.version);
            ESP_LOGI(TAG, "  Data Type: %d (0=Numeric, 1=Alphanumeric, 2=Byte, 3=Kanji)\n", data.data_type);
            ESP_LOGI(TAG, "  Payload (%u bytes): %s\n", data.payload_len, data.payload);
            // create string from payload and process the url
            String url = String((char*)data.payload, data.payload_len);
            process_url_data(url);
        } else {
            ESP_LOGI(TAG, "QR Code decoding FAILED for code #%d. Error: %s\n", i + 1, quirc_strerror(err));
        }
    }

    quirc_destroy(q);
    ESP_LOGI(TAG, "Scan complete.");
}

static void process_url_data(String url)
{
    // read humidity and temperature
    // float temp = dht.readTemperature();
    // float humidity = dht.readHumidity();

    // test values
    url.replace("YOUR_TEAM", "TEST_TEAM_NAME");
    url.replace("FILL_HERE", String(12.5));
    url.replace("FILL_THERE", String(80));

    ESP_LOGI(TAG, "Modified url data: %s", url.c_str());

    // send http request
}