/*****************************************************
 *  INCLUDES
 *****************************************************/
#include "networking.h"
#include "webpage.h"


/*****************************************************
 *  DECLARATIONS
 *****************************************************/
void init_wifi()
{
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.println("Attempting to connect to WiFi");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(100);
    }

    // if the connection to the wifi was successfull, initialize the server
    Serial.println("");
    Serial.print("Connected to WiFi: http://");
    Serial.print(WiFi.localIP());
    Serial.println("RSSI(signal strength):" + WiFi.RSSI());

    init_server();
}


// starts the server and registers callbacks
void init_server()
{
    server.on("/", get_root_page);
    server.begin();
}


// server callback functions
void get_root_page()
{
    server.send(200, "text/plain", index_html);
}
