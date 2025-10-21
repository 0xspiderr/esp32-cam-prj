/*****************************************************
 *  INCLUDES
 *****************************************************/
#include "networking.h"


/*****************************************************
 *  DECLARATIONS
 *****************************************************/
void init_wifi()
{
    WiFi.persistent(true);  // make wifi persistent(save credentials if rebooting)
    WiFi.mode(WIFI_STA);    // station mode, esp connects to an access point
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.println("Attempting to connect to WiFi");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(100);
    }

    Serial.println("Connected to WiFi: http://%s%s\n");
    Serial.print(WiFi.localIP().toString().c_str());
    Serial.print(URL);
}


void init_server()
{
    // welcome message
    server.on("/", HTTP_GET, [](){
    server.send(200, "text/plain", "ESP32-CAM Video Streaming");
    });
    // trigger handle_stream to begin streaming
    server.on(URL, HTTP_GET, handle_stream);
    server.begin();
}


void handle_stream()
{
    camera_fb_t *fb = NULL;
    size_t jpg_buf_len = 0;
    uint8_t *jpg_buf = NULL;
    char part_buf[64];

    // multipart/x-mixed-replace - allows streaming by updating image data for each frame
    String response = "HTTP/1.1 200 OK\r\n"
    "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";

    server.client().write(response.c_str(), response.length());

    while (server.client().connected())
    {
        // get camera frame buffer
        fb = esp_camera_fb_get();
        if (!fb)
        {
            Serial.println("Camera capture failed");
            break;
        }

        if(fb->format != PIXFORMAT_JPEG){
            bool jpeg_converted = frame2jpg(fb, 80, &jpg_buf, &jpg_buf_len);
            esp_camera_fb_return(fb);
            if(!jpeg_converted){
                Serial.println("JPEG compression failed");
                break;
            }
        } else {
            jpg_buf_len = fb->len;
            jpg_buf = fb->buf;
        }

        size_t hlen = snprintf(part_buf, 64, "--frame\r\nContent-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n", jpg_buf_len);
        server.client().write(part_buf, hlen);
        server.client().write((const char *)jpg_buf, jpg_buf_len);
        server.client().write("\r\n", 2);

        if(fb->format != PIXFORMAT_JPEG){
            free(jpg_buf);
        }
        esp_camera_fb_return(fb);
    }
}

