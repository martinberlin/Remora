// Remora Firmware Beta 1.1 compatible ESP8266
#include <ESP8266WiFi.h>
// Config.h and Animate.h need configuration
#include <Config.h>  // WiFi credentials
#include <Animate.h> // PixelCount, PixelPin

// Debug mode prints to serial
bool debugMode = true;
// Animation handling class
Animate animate;

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

void setup() {
  Serial.begin(115200);
  connectToWifi();

  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.println(".");
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  animate.startUdpListener(WiFi.localIP(), internalConfig.udpPort);
  }

void loop() {
  animate.loop();
}