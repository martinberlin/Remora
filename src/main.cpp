// Remora Firmware Beta 1.0. First squeleton

#include <WiFi.h>
#include <Config.h>
#include "AsyncUDP.h"
// Timers
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

// Debug mode prints to serial
bool debugMode = true;
TimerHandle_t wifiReconnectTimer;

// Message transport protocol
AsyncUDP udp;

struct config {
  char chipId[20];
  int udpPort = 1234;
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
   
   if(udp.listen(internalConfig.udpPort)) {
        printMessage("UDP Listening on IP: ");
        printMessage(IpAddress2String(WiFi.localIP())+":"+String(internalConfig.udpPort));

    // Executes on UDP receive
    udp.onPacket([](AsyncUDPPacket packet) {

        Serial.print("Data: ");
        Serial.write(packet.data(), packet.length());
        Serial.println();
        
        }); 
    }

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
    delay(10);
}