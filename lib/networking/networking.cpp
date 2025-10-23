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

    Serial.println("Connected to WiFi: http://");
    Serial.print(String(WiFi.localIP().toString().c_str()) + "/");
    Serial.print(URL);
}


void init_server()
{

}

