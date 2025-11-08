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
#define PART_BOUNDARY       "123456789000000000000987654321"
#define CAMERA_STREAM_DELAY 20

/*****************************************************
 *  PROTOTYPES
 *****************************************************/
void             init_wifi     (void);
void             init_server   (void);

// internal functions used only in this unit
static esp_err_t index_handler   (httpd_req_t *);
static esp_err_t stream_handler  (httpd_req_t *);
static esp_err_t flash_handler   (httpd_req_t *);
static esp_err_t scan_qr_handler (httpd_req_t *);


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
<button onclick="toggleGrayscale()">toggle grayscale</button>
<button onclick="convertJpeg()">convert jpeg</button>
<script>
window.onload=document.getElementById("photo").src=window.location.href.slice(0,-1)+":81/stream";
function toggleFlash(){
fetch('/flash', {method:'POST'});}
function toggleGrayscale(){
fetch('/grayscale', {method:'POST'});}
function convertJpeg(){
fetch('/convert-jpeg', {method:'POST'});}
</script>
</body>
</html>)rawliteral";
#endif //NETWORKING_H
