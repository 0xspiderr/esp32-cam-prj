#ifndef NETWORKING_H
#define NETWORKING_H
/*****************************************************
 *  INCLUDES
 *****************************************************/
#include <WebServer.h>
#include "../../include/wifi_credentials.h"
#include "esp_http_server.h"


/*****************************************************
 *  DEFINES
 *****************************************************/
#define PART_BOUNDARY "123456789000000000000987654321"


/*****************************************************
 *  VARIABLES
 *****************************************************/
static const char *STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char *STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char *STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";
static httpd_handle_t camera_httpd = NULL;
static httpd_handle_t stream_httpd = NULL;


/*****************************************************
 *  PROTOTYPES
 *****************************************************/
void init_wifi           (void);
void init_server         (void);
static esp_err_t index_handler(httpd_req_t *);


/*****************************************************
 *  WEBPAGE
 *****************************************************/
// PROGMEM - puts this variable in flash memory and rawliteral declaration prevents the need for escape chars
static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<html>
    <head>
        <title>esp cam test</title>
        <meta name="viewport" content="width=device-width, intial-scale=1">
    </head>
    <body>
        <img src="" id="photo">
    </body>
</html>
)rawliteral";
#endif //NETWORKING_H
