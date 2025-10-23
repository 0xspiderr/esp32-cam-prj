/*****************************************************
 *  INCLUDES
 *****************************************************/
#include "networking.h"

WebServer server(80);

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

    // if the connection to the wifi was successful, initialize the server
    Serial.println("");
    Serial.print("Connected to WiFi: http://");
    Serial.print(WiFi.localIP());
    Serial.println("");
    Serial.println("RSSI(signal strength):");
    Serial.print(WiFi.RSSI());
    Serial.println("");

    init_server();
}


// starts the server and registers callbacks
void init_server()
{
    server.on("/", HTTP_GET, get_root_page);
    // server.on("/api/status", HTTP_GET, get_server_status);

    // server.onNotFound(server_not_found_msg);
    server.begin();
}


// server callback functions
void get_root_page()
{
    Serial.println("Client trying to access home page");
    server.send(200, "text/plain", "<p>hello world</p>");
}


void get_server_status()
{
    server.send(200, "application/json", "{\"status\":\"online\",\"uptime\":\"" + String(millis()/1000) + "\"}");
}


void server_not_found_msg()
{
    server.send(404, "application/json", "{\"error\":\"endpoint not found\"}");
}