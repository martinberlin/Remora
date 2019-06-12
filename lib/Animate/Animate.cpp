#include "Animate.h"
#include "AsyncUDP.h"

// <Configure> this to your own setup:
const uint16_t PixelCount = 72; // Length of LED stripe
const uint8_t  PixelPin = 19;   // Data line of Addressable LEDs
struct RgbColor CylonEyeColor(HtmlColor(0x7f0000)); // Red as default
struct RgbColor blackColor(HtmlColor(0x000000)); 
byte maxBrightness = 105;       // 0 to 255
// </Configure>

// Sent from main.cpp:
struct config {
  int udpPort; 
  String ipAddress;
} animateConfig;
// Message transport protocol
AsyncUDP udp;

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

Animate::Animate() {
    // Constructor (Still need to research how to insert stuff here)
}
/**
 * Generic message printer. Modify this if you want to send this messages elsewhere (Display)
 */
void debugMessage(String message, bool newline = true)
{
  if (Animate::debugMode) {
    if (newline) {
      Serial.println(message);
    } else {
      Serial.print(message);
    }
   }
}

void fadeAll(uint8_t darkenBy)
{
    RgbColor color;
    for (uint16_t indexPixel = 0; indexPixel < strip.PixelCount(); indexPixel++)
    {
        color = strip.GetPixelColor(indexPixel);
        color.Darken(darkenBy);
        strip.SetPixelColor(indexPixel, color);
    }
}

void fadeAnimUpdate(const AnimationParam& param)
{
    if (param.state == AnimationState_Completed)
    {
        fadeAll(10);
        animations.RestartAnimation(param.index);
    }
}

void allBlack()
{
    RgbColor color;
    for (uint16_t indexPixel = 0; indexPixel < strip.PixelCount(); indexPixel++)
    {
        strip.SetPixelColor(indexPixel, blackColor);
    }
}

void moveAnimUpdate(const AnimationParam& param)
{
    // Apply the movement animation curve
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

    // If progress moves fast enough, we may move more than
    // one pixel, so we update all between the calculated and the last
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
        animations.StopAll();
        allBlack();
    }
}

/**
 * Convert the IP to string 
 */
String Animate::ipAddress2String(const IPAddress& ipAddress)
{
  return String(ipAddress[0]) + String(".") +\
  String(ipAddress[1]) + String(".") +\
  String(ipAddress[2]) + String(".") +\
  String(ipAddress[3]);
}

void Animate::startUdpListener(const IPAddress& ipAddress, int udpPort) {
    animateConfig.udpPort = udpPort;
    animateConfig.ipAddress = ipAddress2String(ipAddress);

    strip.Begin();

    if(udp.listen(animateConfig.udpPort)) {
        debugMessage("UDP Listening on IP: ");
        debugMessage(animateConfig.ipAddress+":"+String(animateConfig.udpPort));

    // Executes on UDP receive
    udp.onPacket([](AsyncUDPPacket packet) {
        Serial.print("Data: ");
        Serial.write(packet.data(), packet.length());Serial.println();
        String command;
      
        for ( int i = 0; i < packet.length(); i++ ) {
            command += (char)packet.data()[i];
        }

        debugMessage(String(command.charAt(0)));
     
        if (command.charAt(0) == '6') {
            lastPixel = 0;
            moveDir = 1;
            // ord("1") is 49 in the ascii table
            int duration = ((int)command.charAt(1)-48) * 100;
            debugMessage("> duration: "+String(duration));
            animations.StartAnimation(0, 4, fadeAnimUpdate);
            animations.StartAnimation(1, duration, moveAnimUpdate);
        }
        if (command.charAt(0) == '4') {
            lastPixel = PixelCount;
            moveDir = -1;
            // ord("1") is 49 in the ascii table
            int duration = ((int)command.charAt(1)-48) * 100;
            debugMessage("> duration: "+String(duration));
            animations.StartAnimation(0, 4, fadeAnimUpdate);
            animations.StartAnimation(1, duration, moveAnimUpdate);
        }
    
        if (command.charAt(0) == 'a' && command.charAt(1) == 'l' ) {
            debugMessage("a Linear");
            AnimEaseFunction moveEase = NeoEase::Linear;
            moveEase.swap(moveEase);
        }
        if (command.charAt(0) == 'a' && command.charAt(1) == 'q' ) {
            debugMessage("a QuadraticInOut");
            AnimEaseFunction moveEase = NeoEase::QuadraticInOut; 
            moveEase.swap(moveEase);
        }
        
        // Red + 0-99 test -- It does not work like expected for some reason launches animation again
        //                    with some larger amount of pixels
        if (command.charAt(0) == 'R') {
            int cen = ((int)command.charAt(1)-48) * 10;
            int dec = ((int)command.charAt(2)-48);
            int colorTo = (cen+dec)*2.57;
            debugMessage("Color red to:" + String(colorTo));
            CylonEyeColor.R = colorTo;
        }
        if (command.charAt(0) == 'G') {
            int cen = ((int)command.charAt(1)-48) * 10;
            int dec = ((int)command.charAt(2)-48);
            int colorTo = (cen+dec)*2.57;
            debugMessage("Color green to:" + String(colorTo));
            CylonEyeColor.G = colorTo;
        }
        if (command.charAt(0) == 'B') {
            int cen = ((int)command.charAt(1)-48) * 10;
            int dec = ((int)command.charAt(2)-48);
            int colorTo = (cen+dec)*2.57;
            debugMessage("Color blue to:" + String(colorTo));
            CylonEyeColor.B = colorTo;
        }

 // Pure colors for now
        if (command.charAt(0) == 'r') {
            debugMessage("Pure red");
            CylonEyeColor.R = maxBrightness;
            CylonEyeColor.G = 0;
            CylonEyeColor.B = 0;
        }
        if (command.charAt(0) == 'g') {
            debugMessage("Pure green");
            CylonEyeColor.R = 0;
            CylonEyeColor.G = maxBrightness;
            CylonEyeColor.B = 0;
        }
        if (command.charAt(0) == 'b') {
            debugMessage("Pure blue");
            CylonEyeColor.R = 0;
            CylonEyeColor.G = 0;
            CylonEyeColor.B = maxBrightness;
        }
        if (command.charAt(0) == 'y') {
            debugMessage("Pure yellow");
            CylonEyeColor.R = maxBrightness;
            CylonEyeColor.G = maxBrightness;
            CylonEyeColor.B = 0;
        }
        if (command.charAt(0) == 'w' ) {
            debugMessage("Pure white");
            CylonEyeColor.R = maxBrightness;
            CylonEyeColor.G = maxBrightness;
            CylonEyeColor.B = maxBrightness;
        }
        
        if (command.charAt(0) == 'v') {
            debugMessage("Pure violet");
            CylonEyeColor.R = maxBrightness;
            CylonEyeColor.G = 0;
            CylonEyeColor.B = maxBrightness;
        }
        }); 
    } else {
        debugMessage("UDP Listener could not start");
    }
}

void Animate::loop() {
    if(animations.IsAnimating()) {
        animations.UpdateAnimations();
    }
    
    strip.Show();
}