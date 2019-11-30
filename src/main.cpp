// Remora Firmware Beta 1.1 First squeleton

#include <WiFi.h>
// Config.h and Animate.h need configuration
#include <Config.h>  // WiFi credentials, mDNS Domain
#include <Animate.h> // PixelCount, PixelPin
#include <ESPmDNS.h>
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
// Uncomment to use WiFi manager
//#define USE_WIFI_MANAGER	

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
  Serial.begin(115200);
  
  //reset saved settings
  //wifiManager.resetSettings();
 
	#if defined(USE_WIFI_MANAGER)
	    WiFiManager wifiManager;
		wifiManager.setTimeout(180);
		wifiManager.setConfigPortalTimeout(180); // try for 3 minute
		wifiManager.setMinimumSignalQuality(15);
		wifiManager.setRemoveDuplicateAPs(true);
		wifiManager.setSaveConfigCallback(saveConfigCallback);
		wifiManager.autoConnect("RemoraAP");
        Serial.println("Wifi Manager start");
	    #else
  		connectToWifi();
  	#endif

	  while (WiFi.status() != WL_CONNECTED) {
		delay(300);
		Serial.println(".");
		}
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

  itoa(ESP.getEfuseMac(), internalConfig.chipId, 16);
  printMessage("ESP32 ChipID: "+String(internalConfig.chipId));
  // Set up automatic reconnect timer
  wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void *)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));
}

void loop() {
  animate.loop();
}