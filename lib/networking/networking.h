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
extern WebServer server;

/*****************************************************
 *  PROTOTYPES
 *****************************************************/
void init_wifi           (void);
void init_server         (void);
void get_root_page       (void);
void get_server_status   (void);
void server_not_found_msg(void);

#endif //NETWORKING_H
