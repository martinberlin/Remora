// Remora Firmware Beta 1.1 First squeleton
#include <M5StickC.h>
#include <WiFi.h>
#include <WiFiManager.h>
// Animate.h needs configuration. Constants are now in Config.h
#include <Animate.h> // PixelCount, PixelPin
#include <ESPmDNS.h>
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

// Debug mode prints to serial
bool debugMode = false;
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
  WiFi.begin(FALLBACK_WIFI_SSID, FALLBACK_WIFI_PASS);
}

/**
 * Convert the IP to string 
 */
String ipAddress2String(const IPAddress& ipAddress)
{
  return String(ipAddress[0]) + String(".") +\
  String(ipAddress[1]) + String(".") +\
  String(ipAddress[2]) + String(".") +\
  String(ipAddress[3]);
}

void wifiConnected() {
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
        M5.Lcd.setTextColor(TFT_GREEN, TFT_BLACK);
        M5.Lcd.drawString("IP: "+ipAddress2String(WiFi.localIP()), 8, 18, 2);
        M5.Lcd.drawString("PORT: "+String(internalConfig.udpPort), 10, 50, 2);
        M5.Lcd.drawString("ONLINE", 110, 50, 2);
        if (!MDNS.begin(localDomain)) {
          while(1) { 
          delay(100);
          }
        }
        MDNS.addService("http", "tcp", 80);
        printMessage(String(localDomain)+".local mDns started");

        animate.startUdpListener(WiFi.localIP(), internalConfig.udpPort);
}

void WiFiEvent(WiFiEvent_t event) {
    Serial.printf("[WiFi-event] event: %d\n", event);
    switch(event) {
    case SYSTEM_EVENT_STA_GOT_IP:
        wifiConnected();
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        Serial.println("WiFi lost connection");
        //TODO: Ensure we don't start UDP while reconnecting to Wi-Fi (low prio)
	      xTimerStart(wifiReconnectTimer, 0);
        break;
        // Non used, just there to avoid warnings
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
      WiFiManager wifiManager;
    //reset saved settings
    //wifiManager.resetSettings();

  M5.begin();
  M5.Lcd.setRotation(1);  // 1 & 3 Landscape
  M5.Axp.ScreenBreath(9); // Brightness (min and visible 7 - 10 max)
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Lcd.drawString("REMORA", 10, 2, 2);
  M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);
  M5.Lcd.drawString(". . . . .", 90, 0, 2);
  M5.Lcd.drawString(String(M5.Axp.GetVbatData() * 1.1 / 1000) + " volt | " + String(-144.7 + M5.Axp.GetTempData() * 0.1) + " C", 10, 40);

  Serial.begin(115200);
  WiFi.onEvent(WiFiEvent);
  // Set static IP in case your infrastructure needs it:
  // wifiManager.setSTAStaticIPConfig(IPAddress(192,168,255,111), IPAddress(192,168,255,1), IPAddress(255,255,255,0));
  //Opens  "AutoConnectAP" and goes into a blocking loop awaiting configuration
  wifiManager.autoConnect("AutoConnectAP");
  
  //itoa(ESP.getEfuseMac(), internalConfig.chipId, 16);
  //printMessage("ESP32 ChipID: "+String(internalConfig.chipId));
  // Set up automatic reconnect timer
  wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void *)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));
}

void loop() {
  animate.loop();
}