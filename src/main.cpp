// Remora Firmware Beta 1.1 First squeleton

#include <WiFi.h>
// Config.h and Animate.h need configuration
#include <Config.h>  // WiFi credentials
#include <Animate.h> // PixelCount, PixelPin

#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

// Debug mode prints to serial
bool debugMode = true;
TimerHandle_t wifiReconnectTimer;
// Animation handling class
Animate animate;

struct config {
  char chipId[20];
  int udpPort = 49161; // Default Orca UDP Port
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

void WiFiEvent(WiFiEvent_t event) {
    Serial.printf("[WiFi-event] event: %d\n", event);
    switch(event) {
    case SYSTEM_EVENT_STA_GOT_IP:
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
        animate.startUdpListener(WiFi.localIP(), internalConfig.udpPort);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        Serial.println("WiFi lost connection");
        // Ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
	      xTimerStart(wifiReconnectTimer, 0);
        break;
    }
}

void setup()
{
  Serial.begin(115200);
  connectToWifi();
  WiFi.onEvent(WiFiEvent);
  itoa(ESP.getEfuseMac(), internalConfig.chipId, 16);
  printMessage("ESP32 ChipID: "+String(internalConfig.chipId));

  // Set up automatic reconnect timers
  wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void *)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));
}

void loop() {
  animate.loop();
}