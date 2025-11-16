#ifndef NETWORKING_H
#define NETWORKING_H
/*****************************************************
 *  INCLUDES
 *****************************************************/
#include "../../include/wifi_credentials.h"
#include "esp_http_server.h"
#include <esp_now.h>
#include <pgmspace.h>
#include "../qr_scanner/qr_scanner.h"


/*****************************************************
 *  DEFINES
 *****************************************************/
#define PART_BOUNDARY       "123456789000000000000987654321"
#define CAMERA_STREAM_DELAY 20

/*****************************************************
 *  STRUCTURES
 *****************************************************/
typedef struct esp_now_command
{
    char cmd[4];
} esp_now_command;


/*****************************************************
 *  PROTOTYPES
 *****************************************************/
// init functions
void init_wifi    (void);
void init_server  (void);
void init_esp_now (void);

// internal functions used only in this unit
// handlers
static esp_err_t index_handler       (httpd_req_t *);
static esp_err_t stream_handler      (httpd_req_t *);
static esp_err_t flash_handler       (httpd_req_t *);
static esp_err_t scan_qr_handler     (httpd_req_t *);
static esp_err_t movement_cmd_handler(httpd_req_t *);
// esp-now
static void      on_data_sent        (const uint8_t *, esp_now_send_status_t);
static void      send_move_command   (const char *);


/*****************************************************
 *  WEBPAGE
 *****************************************************/
// PROGMEM - puts this variable in flash memory and rawliteral declaration prevents the need for escape chars
static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<html>
<head>
<title>esp cam test</title>
<meta name="viewport" content="width=device-width, intial-scale=1">
<style>
img{
width:auto;
max-width:100%;
height:auto;
}
</style>
</head>
<body>
<img src="" id="photo">
<br>
<button onclick="toggleFlash()">toggle flash</button>
<button onclick="convertJpeg()">scan qr code</button>
<button onmousedown="sendCmd('F')"
        onmouseup="sendCmd('S')">forward</button>
<button onmousedown="sendCmd('B')"
        onmouseup="sendCmd('S')">back</button>
<button onmousedown="sendCmd('L')"
        onmouseup="sendCmd('S')">turn left</button>
<button onmousedown="sendCmd('R')"
        onmouseup="sendCmd('S')">turn right</button>
<script>
window.onload=document.getElementById("photo").src=window.location.href.slice(0,-1)+":81/stream";
function toggleFlash(){
fetch('/flash', {method:'POST'});}
function convertJpeg(){
fetch('/convert-qr', {method:'POST'});}
function sendCmd(cmd){
fetch('/command?cmd='+cmd);}
</script>
</body>
</html>)rawliteral";
#endif //NETWORKING_H
