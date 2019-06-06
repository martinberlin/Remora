// Remora Firmware Beta 1.0. First squeleton

#include <WiFi.h>
#include <Config.h>
#include "AsyncUDP.h"
// Timers
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

// START Quick and dirty test move to Animation class
#include <NeoPixelBus.h>
#include <NeoPixelAnimator.h>
const uint16_t PixelCount = 144; 
const uint8_t PixelPin = 19;  
struct RgbColor CylonEyeColor(HtmlColor(0x7f0000));

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(PixelCount, PixelPin);
NeoPixelAnimator animations(2); // only ever need 2 animations
uint16_t lastPixel = 0; // track the eye position
int8_t moveDir = 1; // track the direction of movement
// uncomment one of the lines below to see the effects of
// changing the ease function on the movement animation
AnimEaseFunction moveEase =
//      NeoEase::Linear;
//      NeoEase::QuadraticInOut;
//      NeoEase::CubicInOut;
        NeoEase::QuarticInOut;
//      NeoEase::QuinticInOut;
//      NeoEase::SinusoidalInOut;
//      NeoEase::ExponentialInOut;
//      NeoEase::CircularInOut;

void FadeAll(uint8_t darkenBy)
{
    RgbColor color;
    for (uint16_t indexPixel = 0; indexPixel < strip.PixelCount(); indexPixel++)
    {
        color = strip.GetPixelColor(indexPixel);
        color.Darken(darkenBy);
        strip.SetPixelColor(indexPixel, color);
    }
}

void FadeAnimUpdate(const AnimationParam& param)
{
    if (param.state == AnimationState_Completed)
    {
        FadeAll(10);
        animations.RestartAnimation(param.index);
    }
}

void MoveAnimUpdate(const AnimationParam& param)
{
    // apply the movement animation curve
    float progress = moveEase(param.progress);
    // use the curved progress to calculate the pixel to effect
    uint16_t nextPixel;
    if (moveDir > 0)
    {
        nextPixel = progress * PixelCount;
    }
    else
    {
        nextPixel = (1.0f - progress) * PixelCount;
    }

    // if progress moves fast enough, we may move more than
    // one pixel, so we update all between the calculated and
    // the last
    if (lastPixel != nextPixel)
    {
        for (uint16_t i = lastPixel + moveDir; i != nextPixel; i += moveDir)
        {
            strip.SetPixelColor(i, CylonEyeColor);
        }
    }
    strip.SetPixelColor(nextPixel, CylonEyeColor);
    lastPixel = nextPixel;

    if (param.state == AnimationState_Completed)
    {
        // reverse direction of movement
        moveDir *= -1;
        // done, time to restart this position tracking animation/timer
        animations.RestartAnimation(param.index);
    }
}

void SetupAnimations()
{
    // fade all pixels providing a tail that is longer the faster the pixel moves.
    //animations.StartAnimation(0, 5, FadeAnimUpdate);
    // take several seconds to move eye fron one side to the other
    animations.StartAnimation(1, 1000, MoveAnimUpdate);
}
// END

// Debug mode prints to serial
bool debugMode = true;
TimerHandle_t wifiReconnectTimer;

// Message transport protocol
AsyncUDP udp;

struct config {
  char chipId[20];
  int udpPort = 49161;
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
        Serial.write(packet.data(), packet.length());Serial.println();
        String command;
      
        for ( int i = 0; i < packet.length(); i++ ) {
            command += (char)packet.data()[i];
        }

        // Serial.println(command.charAt(0));
        // TODO Refactor this in an animation class
        if (command.charAt(0) == 'S') {
            printMessage("S Start long");
            // take 1000ms to move eye fron one side to the other
            animations.StartAnimation(0, 8, FadeAnimUpdate);
            animations.StartAnimation(1, 800, MoveAnimUpdate);
        }
        if (command.charAt(0) == 's') {
            printMessage("s Start short");
            // take 1000ms to move eye fron one side to the other
            animations.StartAnimation(0, 4, FadeAnimUpdate);
            animations.StartAnimation(1, 200, MoveAnimUpdate);
        }
        if (command.charAt(0) == 'E') {
            printMessage("End");
            animations.StopAnimation(0);
            animations.StopAnimation(1);
        }
        if (command.charAt(0) == 's' && command.charAt(1) == 'l' ) {
            printMessage("s Linear");
            moveEase.swap(NeoEase::Linear);
            //AnimEaseFunction moveEase = NeoEase::Linear;
        }
        if (command.charAt(0) == 's' && command.charAt(1) == 'q' ) {
            printMessage("s QuadraticInOut");
            AnimEaseFunction moveEase = NeoEase::QuadraticInOut; 
        }
        if (command.charAt(0) == 's' && command.charAt(1) == 'c' ) {
            AnimEaseFunction moveEase = NeoEase::CubicInOut;
        }
        if (command.charAt(0) == 's' && command.charAt(1) == 'w' ) {
            AnimEaseFunction moveEase = NeoEase::QuinticInOut;
        }
        if (command.charAt(0) == 's' && command.charAt(1) == 's' ) {
            AnimEaseFunction moveEase = NeoEase::SinusoidalInOut;
        }
        if (command.charAt(0) == 's' && command.charAt(1) == 'e' ) {
            AnimEaseFunction moveEase = NeoEase::ExponentialInOut;
        }
        if (command.charAt(0) == 's' && command.charAt(1) == 'o' ) {
            AnimEaseFunction moveEase = NeoEase::CircularInOut;
        }
        // Colors : Add & Substract
        if (command.charAt(0) == 'C' && command.charAt(1) == 'r' ) {
            printMessage("Add red");
            CylonEyeColor.R = 100;
        }
        if (command.charAt(0) == 'C' && command.charAt(1) == 'g' ) {
            printMessage("Add green");
            CylonEyeColor.G = 100;
        }
        if (command.charAt(0) == 'C' && command.charAt(1) == 'b' ) {
            printMessage("Add blue");
            CylonEyeColor.B = 100;
        }
        if (command.charAt(0) == 'c' && command.charAt(1) == 'r' ) {
            printMessage("Off red");
            CylonEyeColor.R = 0;
        }
        if (command.charAt(0) == 'c' && command.charAt(1) == 'g' ) {
            printMessage("Off green");
            CylonEyeColor.G = 0;
        }
        if (command.charAt(0) == 'c' && command.charAt(1) == 'b' ) {
            printMessage("Off blue");
            CylonEyeColor.B = 0;
        }
        if (command.charAt(0) == 'P' && command.charAt(1) == 'r' ) {
            printMessage("Pure red");
            CylonEyeColor.R = 100;
            CylonEyeColor.G = 0;
            CylonEyeColor.B = 0;
        }
        if (command.charAt(0) == 'p' && command.charAt(1) == 'g' ) {
            printMessage("Pure green");
            CylonEyeColor.R = 0;
            CylonEyeColor.G = 100;
            CylonEyeColor.B = 0;
        }
        if (command.charAt(0) == 'p' && command.charAt(1) == 'b' ) {
            printMessage("Pure blue");
            CylonEyeColor.R = 0;
            CylonEyeColor.G = 0;
            CylonEyeColor.B = 100;
        }
        if (command.charAt(0) == 'p' && command.charAt(1) == 'v' ) {
            printMessage("Pure violet");
            CylonEyeColor.R = 100;
            CylonEyeColor.G = 0;
            CylonEyeColor.B = 100;
        }
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
 
    strip.Begin();
    strip.Show();
    
}

void loop() {
    animations.UpdateAnimations();
    strip.Show();
}