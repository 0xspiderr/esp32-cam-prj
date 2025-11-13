/*****************************************************
 *  INCLUDES
 *****************************************************/
#include "networking.h"
#include "../camera/camera.h"


/*****************************************************
 *  VARIABLES
 *****************************************************/
// for error logging
static const char *TAG = "NETWORKING";
// stream specific variables
const char        *STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
const char        *STREAM_BOUNDARY     = "\r\n--" PART_BOUNDARY "\r\n";
const char        *STREAM_PART         = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";
httpd_handle_t    camera_httpd         = NULL;
httpd_handle_t    stream_httpd         = NULL;

/*****************************************************
 *  DEFINITIONS
 *****************************************************/
void init_wifi()
{
    size_t no_connection_cnt = 0;
    WiFi.mode(WIFI_MODE_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.println("Attempting to connect to WiFi");
    // try wifi connection, restart esp after 15 seconds if not succesfull and try again.
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(100);
        if (++no_connection_cnt > 150)
        {
            Serial.println("Connection failed, restarting ESP-32");
            ESP.restart();
        }
    }

    Serial.println("");
    Serial.print("Connected to WiFi: http://");
    Serial.print(WiFi.localIP());
    Serial.println("");
    Serial.println("RSSI(signal strength):");   // numbers closer to 0 mean better signal strength
    Serial.print(WiFi.RSSI());
    Serial.println("");
}

// starts the server and registers URIs
// TODO: move uri setup to a different function
void init_server()
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.stack_size = 8192;
    config.server_port = 80;
    config.core_id = 0;

    httpd_uri_t index_uri =
    {
        .uri = "/",
        .method = HTTP_GET,
        .handler = index_handler,
        .user_ctx = NULL
    };

    httpd_uri_t stream_uri =
    {
        .uri = "/stream",
        .method = HTTP_GET,
        .handler = stream_handler,
        .user_ctx = NULL
    };

    httpd_uri_t flash_uri =
    {
        .uri = "/flash",
        .method = HTTP_POST,
        .handler = flash_handler,
        .user_ctx = NULL
    };

    httpd_uri_t scan_qr_uri =
    {
        .uri = "/convert-jpeg",
        .method = HTTP_POST,
        .handler =  scan_qr_handler,
        .user_ctx = NULL
    };

    if (httpd_start(&camera_httpd, &config) == ESP_OK)
    {
        httpd_register_uri_handler(camera_httpd, &index_uri);
        httpd_register_uri_handler(camera_httpd, &flash_uri);
        httpd_register_uri_handler(camera_httpd, &scan_qr_uri);
    }

    config.server_port = 81;
    config.ctrl_port = 81;  // udp port
    if (httpd_start(&stream_httpd, &config) == ESP_OK)
        httpd_register_uri_handler(stream_httpd, &stream_uri);
}
/* Endpoint handlers ******************************************************/
static esp_err_t index_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, INDEX_HTML, strlen(INDEX_HTML));
}


static esp_err_t stream_handler(httpd_req_t *req)
{
    camera_fb_t *fb           = NULL; // camera frame buffer
    char         part_buf[64];
    esp_err_t    res          = ESP_OK;

    res = httpd_resp_set_type(req, STREAM_CONTENT_TYPE);
    if (res != ESP_OK)
        return res;

    while (true)
    {
        fb = esp_camera_fb_get();
        if (!fb)
        {
            ESP_LOGE(TAG, "camera photo capture failed");
            esp_camera_fb_return(fb);
            continue;
        }
        // send part header
        if (res == ESP_OK)
        {
            ssize_t hlen = snprintf(part_buf, 64, STREAM_PART, fb->len);
            res = httpd_resp_send_chunk(req, (const char*)part_buf, hlen);
        }
        // send frame buffer data
        if (res == ESP_OK)
            res = httpd_resp_send_chunk(req, (const char *)fb->buf, fb->len);
        // send stream boundary
        if (res == ESP_OK)
            res = httpd_resp_send_chunk(req, STREAM_BOUNDARY, strlen(STREAM_BOUNDARY));
        esp_camera_fb_return(fb);
        if (res != ESP_OK)
            break;
    }
    return res;
}


static esp_err_t flash_handler(httpd_req_t *req)
{
    if (req->method == HTTP_POST)
    {
        toggle_camera_flash();
        httpd_resp_send(req, "toggled flash", HTTPD_RESP_USE_STRLEN);
        return ESP_OK;
    }

    // for any methods aside http_post
    httpd_resp_send_404(req);
    return ESP_FAIL;
}


static esp_err_t scan_qr_handler(httpd_req_t *req)
{
    if (req->method == HTTP_POST)
    {
        // Trigger the scan
        trigger_qr_scan();

        // Or access directly
        if (qr_scan_task != NULL) {
            vTaskResume(qr_scan_task);
        }

        httpd_resp_send(req, "converted", HTTPD_RESP_USE_STRLEN);
        return ESP_OK;
    }

    httpd_resp_send_404(req);
    return ESP_FAIL;
}
/**************************************************************************/