#include <WiFi.h>
// Config.h and Animate.h need configuration
#include <Config.h>  // WiFi credentials, mDNS Domain
#include <Animate.h> // PixelCount, PixelPin
#include <ESPmDNS.h>
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include <U8g2lib.h> 

// HELTEC Board pins, update if you use another Board:
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);
byte u8cursor = 1;

// Debug mode prints to serial
bool debugMode = true;
TimerHandle_t wifiReconnectTimer;
// Animation handling class
Animate animate;
const char* localDomain = MDNSDOMAIN; // mDNS: led.local (Config.h)
struct config {
  char chipId[20];
  int udpPort = 49161; // 49161 Default Orca UDP Port
} internalConfig;

/**
 * Generic message printer. Modify this if you want to send this messages elsewhere (Display)
 */
void printMessage(String message, bool newline = true)
{
  if (debugMode) {
    if (newline) {
      Serial.println(message);
    } else {
      Serial.print(message);
    }
   }
}

void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID1, WIFI_PASS1);
}

/**
 * Convert the IP to string 
 */
String IpAddress2String(const IPAddress& ipAddress)
{
  return String(ipAddress[0]) + String(".") +\
  String(ipAddress[1]) + String(".") +\
  String(ipAddress[2]) + String(".") +\
  String(ipAddress[3]);
}

void WiFiEvent(WiFiEvent_t event) {
    Serial.printf("[WiFi-event] event: %d\n", event);

    switch(event) {
    case SYSTEM_EVENT_STA_GOT_IP:
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
    

        if (!MDNS.begin(localDomain)) {
          while(1) { 
          delay(100);
          }
        }
        MDNS.addService("http", "tcp", 80);
        printMessage(String(localDomain)+".local mDns started");

        animate.startUdpListener(WiFi.localIP(), internalConfig.udpPort);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        Serial.println("WiFi lost connection");
        // TODO: Ensure we don't start UDP while reconnecting to Wi-Fi (low prio)
	      xTimerStart(wifiReconnectTimer, 0);
        break;
        
        // Non used, just there to avoid warnings
        case SYSTEM_EVENT_STA_WPS_ER_PBC_OVERLAP:
        case SYSTEM_EVENT_WIFI_READY:
        case SYSTEM_EVENT_SCAN_DONE:
        case SYSTEM_EVENT_STA_START:
        case SYSTEM_EVENT_STA_STOP:
        case SYSTEM_EVENT_STA_CONNECTED:
        case SYSTEM_EVENT_STA_AUTHMODE_CHANGE:
        case SYSTEM_EVENT_STA_LOST_IP:
        case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
        case SYSTEM_EVENT_STA_WPS_ER_FAILED:
        case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
        case SYSTEM_EVENT_STA_WPS_ER_PIN:
        case SYSTEM_EVENT_AP_START:
        case SYSTEM_EVENT_AP_STOP:
        case SYSTEM_EVENT_AP_STACONNECTED:
        case SYSTEM_EVENT_AP_STADISCONNECTED:
        case SYSTEM_EVENT_AP_STAIPASSIGNED:
        case SYSTEM_EVENT_AP_PROBEREQRECVED:
        case SYSTEM_EVENT_GOT_IP6:
        case SYSTEM_EVENT_ETH_START:
        case SYSTEM_EVENT_ETH_STOP:
        case SYSTEM_EVENT_ETH_CONNECTED:
        case SYSTEM_EVENT_ETH_DISCONNECTED:
        case SYSTEM_EVENT_ETH_GOT_IP:
        case SYSTEM_EVENT_MAX:
        break;
    }
}

void setup()
{
  Serial.begin(115200);

  u8x8.begin();
  u8x8.setCursor(0, u8cursor);
  u8x8.setFont(u8x8_font_amstrad_cpc_extended_u);
  u8x8.print("REMORA");
  delay(100);
  u8x8.print(" .");
  delay(100);
  u8x8.print(" .");
  delay(100);
  u8x8.print(" .");
  delay(100);
  u8x8.print(" .");
  delay(100);
  u8x8.print(" .");

  connectToWifi();

  
  WiFi.onEvent(WiFiEvent);

  // Set up automatic reconnect timer
  wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void *)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));
}

void loop() {
  animate.loop();
}