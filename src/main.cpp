// Remora Firmware Beta 1.1 First squeleton

#include <WiFi.h>
// Config.h and Animate.h need configuration
#include <Config.h>  // WiFi credentials, mDNS Domain
#include <Animate.h> // PixelCount, PixelPin
#include <ESPmDNS.h>
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

#define USE_WIFI_MANAGER	

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

#ifdef USE_WIFI_MANAGER
	#include <DNSServer.h>
	#include <WebServer.h>
	#include <WiFiManager.h>

  WiFiManager wifiManager;
#endif

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

#ifdef USE_WIFI_MANAGER
	//flag for saving data
	bool shouldSaveConfig = false;

	//callback notifying us of the need to save config
	void saveConfigCallback () {
		Serial.println("Should save config");
		shouldSaveConfig = true;
	}
#endif

void setup()
{
  WiFiManager wifiManager;
  Serial.begin(115200);
  
  //reset saved settings
  //wifiManager.resetSettings();
 
	#if defined(USE_WIFI_MANAGER)
		wifiManager.setTimeout(180);
		wifiManager.setConfigPortalTimeout(180); // try for 3 minute
		wifiManager.setMinimumSignalQuality(15);
		wifiManager.setRemoveDuplicateAPs(true);
		wifiManager.setSaveConfigCallback(saveConfigCallback);
		wifiManager.autoConnect("RemoraAP");
    Serial.println("Wifi Manager start");
  #endif
  
  WiFi.onEvent(WiFiEvent);

  itoa(ESP.getEfuseMac(), internalConfig.chipId, 16);
  printMessage("ESP32 ChipID: "+String(internalConfig.chipId));
  // Set up automatic reconnect timer
  wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void *)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));
}

void loop() {
  animate.loop();
}