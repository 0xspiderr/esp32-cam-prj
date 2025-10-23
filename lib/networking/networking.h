#ifndef NETWORKING_H
#define NETWORKING_H
/*****************************************************
 *  INCLUDES
 *****************************************************/
#include <WebServer.h>
#include "../../include/wifi_credentials.h"


/*****************************************************
 *  VARIABLES
 *****************************************************/
static WebServer server(80);

/*****************************************************
 *  PROTOTYPES
 *****************************************************/
void init_wifi    (void);
void init_server  (void);
void get_root_page(void);

#endif //NETWORKING_H
