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
static bool stream_paused = false;


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
    config.server_port = 80;
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

    httpd_uri_t grayscale_uri =
    {
        .uri = "/grayscale",
        .method = HTTP_POST,
        .handler = grayscale_handler,
        .user_ctx = NULL
    };

    if (httpd_start(&camera_httpd, &config) == ESP_OK)
    {
        httpd_register_uri_handler(camera_httpd, &index_uri);
        httpd_register_uri_handler(camera_httpd, &flash_uri);
        httpd_register_uri_handler(camera_httpd, &grayscale_uri);
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
    uint8_t     *jpg_buf      = NULL;
    size_t       jpg_buf_len  = 0;

    res = httpd_resp_set_type(req, STREAM_CONTENT_TYPE);
    if (res != ESP_OK)
        return res;

    while (true)
    {
        fb = esp_camera_fb_get();
        jpg_buf_len = 0;
        jpg_buf = NULL;

        if (!fb)
        {
            ESP_LOGE(TAG, "camera photo capture failed");
            delay(10);
            continue;
        }

        if (fb->format != PIXFORMAT_JPEG)
        {
            bool valid_jpg = frame2jpg(fb, 80, &jpg_buf, &jpg_buf_len);
            if (!valid_jpg)
            {
                ESP_LOGE(TAG, "JPEG compression failed");
                esp_camera_fb_return(fb);
                delay(10);
                continue;
            }
        }
        else
        {
            jpg_buf = fb->buf;
            jpg_buf_len = fb->len;
        }

        ESP_LOGI(TAG, "JPEG compression succeeded");
        Serial.println(jpg_buf_len);

        // send part header
        if (res == ESP_OK)
        {
            ssize_t hlen = snprintf(part_buf, 64, STREAM_PART, jpg_buf_len);
            res = httpd_resp_send_chunk(req, (const char*)part_buf, hlen);
        }
        // send frame buffer data
        if (res == ESP_OK)
            res = httpd_resp_send_chunk(req, (const char *)jpg_buf, jpg_buf_len);
        // send stream boundary
        if (res == ESP_OK)
            res = httpd_resp_send_chunk(req, STREAM_BOUNDARY, strlen(STREAM_BOUNDARY));

        if (fb->format != PIXFORMAT_JPEG && jpg_buf)
            free(jpg_buf);

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


static esp_err_t grayscale_handler(httpd_req_t *req)
{
    if (req->method == HTTP_POST)
    {
        toggle_grayscale();
        httpd_resp_send(req, "toggled greyscale", HTTPD_RESP_USE_STRLEN);
        return ESP_OK;
    }

    httpd_resp_send_404(req);
    return ESP_FAIL;
}
/**************************************************************************/