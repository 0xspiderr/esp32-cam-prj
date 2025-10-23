#ifndef NETWORKING_H
#define NETWORKING_H
/*****************************************************
 *  INCLUDES
 *****************************************************/
#include <WebServer.h>
#include <esp_camera.h>
#include "../../include/wifi_credentials.h"


/*****************************************************
 *  VARIABLES
 *****************************************************/
static const char *URL = "/stream";
static WebServer server(80);

/*****************************************************
 *  PROTOTYPES
 *****************************************************/
void init_wifi  (void);
void init_server(void);


#endif //NETWORKING_H
