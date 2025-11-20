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
    char cmd[8];
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
static esp_err_t index_handler          (httpd_req_t *);
static esp_err_t stream_handler         (httpd_req_t *);
static esp_err_t flash_handler          (httpd_req_t *);
static esp_err_t scan_qr_handler        (httpd_req_t *);
static esp_err_t movement_cmd_handler   (httpd_req_t *);
static esp_err_t flash_intensity_handler(httpd_req_t *);
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
/* Stil imagine: se asigura ca incape, dar se micsoreaza pe ecrane mici */
img {
  max-width: 100%;
  height: auto;
  display: block;
  margin: 10px auto; /* Centrare pe mijloc */
  background-color: #333; /* Un fundal gri cat timp se incarca */
}

body {
  font-family: sans-serif;
  text-align: center;
  background-color: #f2f2f2;
  margin: 0;
  padding: 10px;
}

/* Container pentru grupuri */
.grup {
  background: white;
  border: 1px solid #ccc;
  padding: 10px;
  margin-bottom: 15px;
  border-radius: 8px;
  display: inline-block;
  width: 95%;
  max-width: 340px; /* Putin mai lat decat imaginea video */
}

/* Butoane */
button {
  margin: 4px;
  padding: 10px 12px;
  font-size: 14px;
  cursor: pointer;
  touch-action: manipulation;
}

/* Butonul rosu mare */
.stop-btn {
  background-color: red;
  color: white;
  font-weight: bold;
  font-size: 18px;
  padding: 15px 0;
  border: none;
  border-radius: 5px;
  display: block;
  width: 100%;
  margin-top: 15px;
}

/* Rand nou pentru aliniere */
.row { display: block; margin: 2px 0; }
</style>

</head>
<body>
<img src="" id="photo">
<br>
<div class="grup">
<button onclick="toggleFlash()">toggle flash</button>
<button onclick="convertJpeg()">scan qr code</button>
<div class="slider-container">
  <div class="intensity-label">flash intensity: <span id="intensityValue">128</span></div>
  <input type="range" min="0" max="255" value="255" class="slider" id="flashSlider">
</div>
</div>

<div class="grup">
  <div class="row">
    <button onmousedown="sendCmd('F')" onmouseup="sendCmd('S')">forward</button>
  </div>
  <div class="row">
    <button onclick="sendCmd('L')">turn left</button>
    <button onclick="sendCmd('NS')">turn forward</button>
    <button onclick="sendCmd('R')">turn right</button>
  </div>
  <div class="row">
    <button onmousedown="sendCmd('B')" onmouseup="sendCmd('S')">back</button>
  </div>
</div>


<div class="grup">
  <button onmousedown="sendCmd('AMU')" onmouseup="sendCmd('SAM')">arm motor up</button>
  <button onmousedown="sendCmd('AMD')" onmouseup="sendCmd('SAM')">arm motor down</button>

<button onclick="sendCmd('ASO')">arm servo open</button>
<button onclick="sendCmd('ASC')">arm servo close</button>

<button onclick="sendCmd('CSR')">cam servo rotate right</button>
<button onclick="sendCmd('CSL')">cam servo rotate left</button>

<button onclick="sendCmd('SAM')">arm motor stop</button>
<button class="stop-btn" onclick="sendCmd('S')"> emergency stop </button>
</div>
<script>
var flashIntensity = 255;
window.onload = function() {
  document.getElementById("photo").src = window.location.href.slice(0,-1) + ":81/stream";

  // Event listener pentru slider
  var slider = document.getElementById("flashSlider");
  var output = document.getElementById("intensityValue");

  slider.oninput = function() {
    flashIntensity = this.value;
    output.innerHTML = this.value;
    // Trimite noua valoare catre ESP32
    fetch('/flash-intensity?value=' + flashIntensity);
  }
}
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
