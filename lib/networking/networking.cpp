/*****************************************************
 *  INCLUDES
 *****************************************************/
#include "networking.h"

#include <esp_camera.h>


/*****************************************************
 *  DECLARATIONS
 *****************************************************/
void init_wifi()
{
    size_t no_connection_cnt = 0;
    WiFi.mode(WIFI_MODE_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.println("Attempting to connect to WiFi");
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
    Serial.println("RSSI(signal strength):");
    Serial.print(WiFi.RSSI());
    Serial.println("");
}


// starts the server and registers URIs
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

    if (httpd_start(&camera_httpd, &config) == ESP_OK)
        httpd_register_uri_handler(camera_httpd, &index_uri);

    config.server_port = 81;
    config.ctrl_port = 81;  // udp port
    if (httpd_start(&stream_httpd, &config) == ESP_OK)
        httpd_register_uri_handler(stream_httpd, &stream_uri);
}


static esp_err_t index_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, INDEX_HTML, strlen(INDEX_HTML));
}


static esp_err_t stream_handler(httpd_req_t *req)
{
    camera_fb_t *fb       = NULL; // camera frame buffer
    size_t      fb_len    = 0;
    uint8_t     *fb_buf   = NULL;
    char        *part_buf[64];
    esp_err_t    res      = ESP_OK;

    res = httpd_resp_set_type(req, STREAM_CONTENT_TYPE);
    if (res != ESP_OK)
        return res;

    while (true)
    {
        fb = esp_camera_fb_get();
        if (!fb)
        {
            ESP_LOGE("NETWORKING", "camera photo capture failed");
            res = ESP_FAIL;
        }
        else
        {
            if (fb->width > 400)
            {
                if (fb->format != PIXFORMAT_JPEG)
                {
                    bool jpeg_converted = frame2jpg(fb, 80, &fb_buf, &fb_len);
                    esp_camera_fb_return(fb);
                    fb = NULL;
                    if (!jpeg_converted)
                    {
                        ESP_LOGE("NETWORKING", "jpeg compression failed");
                        res = ESP_FAIL;
                    }
                }
                else
                {
                    fb_len = fb->len;
                    fb_buf = fb->buf;
                }
            }
            if (res == ESP_OK)
            {
                ssize_t hlen = snprintf((char*)part_buf, 64, STREAM_PART, fb_len);
                res = httpd_resp_send_chunk(req, (const char*)part_buf, hlen);
            }
            if (res == ESP_OK)
            {
                res = httpd_resp_send_chunk(req, (const char*)fb_buf, fb_len);
            }
            if (res == ESP_OK)
            {
                res = httpd_resp_send_chunk(req, STREAM_BOUNDARY, strlen(STREAM_BOUNDARY));
            }
            if (fb)
            {
                esp_camera_fb_return(fb);
                fb = NULL;
                fb_buf = NULL;
            }
            else if (fb_buf)
            {
                free(fb_buf);
                fb_buf = NULL;
            }

            if (res != ESP_OK)
                break;
        }
    }

    return res;
}
