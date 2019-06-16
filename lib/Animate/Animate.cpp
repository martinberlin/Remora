#include "Animate.h"
#include "AsyncUDP.h"

// <Configure> this to your own setup:
#define DEFAULT_HUE_ANGLE 0
const uint16_t PixelCount = 72; // Length of LED stripe
const uint8_t  PixelPin = 19;   // Data line of Addressable LEDs
struct RgbColor CylonEyeColor(HslColor(0.0f,1.0f,0.5f)); // Red as default

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
uint16_t rightPixel = PixelCount; // Used for 5: moveCrossedAnimUpdate()

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
/**
 * Turn off all LEDs
 */
void allBlack()
{
    for (uint16_t indexPixel = 0; indexPixel < strip.PixelCount(); indexPixel++)
    {
        strip.SetPixelColor(indexPixel, HtmlColor(0x000000));
    }
}
/**
 * Fade all by darkenBy incrementally
 */
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
void darkenAll(const AnimationParam& param)
{
    RgbColor color;
    for (uint16_t indexPixel = 0; indexPixel < strip.PixelCount(); indexPixel++)
    {
        color = strip.GetPixelColor(indexPixel);
        color.Darken(10);
        strip.SetPixelColor(indexPixel, color);
    }
    if (param.state == AnimationState_Completed)
    {
        animations.StopAll();
        allBlack();
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

void allToColor(const AnimationParam& param)
{
    for (uint16_t indexPixel = 0; indexPixel < strip.PixelCount(); indexPixel++)
    {
        strip.SetPixelColor(indexPixel, CylonEyeColor);
    }
}

void allToColorNoise(const AnimationParam& param)
{
    for (uint16_t indexPixel = 0; indexPixel < strip.PixelCount(); indexPixel++)
    {
        byte rand = random(2);
        if (rand==1) {
            strip.SetPixelColor(indexPixel, CylonEyeColor);
        } else {
            strip.SetPixelColor(indexPixel, HtmlColor(0x000000));
        }
        
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

void moveCrossedAnimBlackUpdate(const AnimationParam& param)
{
    float progress = moveEase(param.progress);
    // use the curved progress to calculate the pixel to effect
    uint16_t nextPixel;
    uint16_t nextRightPixel;
    nextPixel = progress * PixelCount;
    nextRightPixel = (1.0f - progress) * PixelCount;

    if (lastPixel != nextPixel)
    {
        for (uint16_t i = lastPixel + 1; i != nextPixel; i += 1)
        {
            strip.SetPixelColor(i, HtmlColor(0x000000));
        }
        for (uint16_t i = rightPixel; i != nextRightPixel; i -= 1)
        {
            strip.SetPixelColor(i, HtmlColor(0x000000));
        }
    }
    strip.SetPixelColor(nextPixel, HtmlColor(0x000000));
    lastPixel = nextPixel;
    rightPixel = nextRightPixel;

    if (param.state == AnimationState_Completed)
    {
        animations.StopAll();
        allBlack();
    }
}

void moveCrossedAnimUpdate(const AnimationParam& param)
{
    float progress = moveEase(param.progress);
    // use the curved progress to calculate the pixel to effect
    uint16_t nextPixel;
    uint16_t nextRightPixel;
    nextPixel = progress * PixelCount;
    nextRightPixel = (1.0f - progress) * PixelCount;

    if (lastPixel != nextPixel)
    {
        for (uint16_t i = lastPixel + 1; i != nextPixel; i += 1)
        {
            strip.SetPixelColor(i, CylonEyeColor);
        }
        for (uint16_t i = rightPixel; i != nextRightPixel; i -= 1)
        {
            strip.SetPixelColor(i, CylonEyeColor);
        }
    }
    strip.SetPixelColor(nextPixel, CylonEyeColor);
    lastPixel = nextPixel;
    rightPixel = nextRightPixel;

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

/**
 * Return int with Hue angle analysing given command ex. offset 2: ;51240 will return 240 (blue)
 */
int hueSelect(String command, int length, uint8_t offset) {
    int colorAngle = DEFAULT_HUE_ANGLE; // Red as default (Maybe make default constant)
    if (length-offset == 1) {
        colorAngle = ((int)command.charAt(2)-48);
    }
    if (length-offset == 2) {
        colorAngle = (((int)command.charAt(2)-48)*10); // Dec
        colorAngle += ((int)command.charAt(3)-48);
    }
    if (length-offset == 3 && ((int)command.charAt(4)-48)<4) {
        colorAngle = (((int)command.charAt(2)-48)*100); // Hundreds
        colorAngle += ((int)command.charAt(3)-48)*10;   // Dec
        colorAngle += ((int)command.charAt(3)-48);
    }
    return colorAngle;
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
        //debugMessage("LEN:"+String(packet.length() ));
        if (command.charAt(0) == '6') {
            lastPixel = 0;
            moveDir = 1;
            // ord("1") is 49 in the ascii table
            int duration = ((int)command.charAt(1)-48) * 100;
            if (packet.length()>2) { // Only change the color if Hue angle is sent otherwise keep last Hue
                int colorAngle = hueSelect(command, packet.length(), 2);
                CylonEyeColor = HslColor(colorAngle / 360.0f, 1.0f, 0.25f);
                debugMessage("colorAngle:"+String(colorAngle));
            }
            debugMessage("> duration: "+String(duration));
            animations.StartAnimation(0, 4, fadeAnimUpdate);
            animations.StartAnimation(1, duration, moveAnimUpdate);
        }

        if (command.charAt(0) == '4') {
            lastPixel = PixelCount;
            moveDir = -1;
            int duration = ((int)command.charAt(1)-48) * 100;
            if (packet.length()>2) {
                int colorAngle = hueSelect(command, packet.length(), 2);
                CylonEyeColor = HslColor(colorAngle / 360.0f, 1.0f, 0.25f);
            }
            animations.StartAnimation(0, 4, fadeAnimUpdate);
            animations.StartAnimation(1, duration, moveAnimUpdate);
        }
        // 2 chasers left->right right<-left
        if (command.charAt(0) == '5') {
            lastPixel = 0;
            rightPixel = PixelCount;
            int duration = ((int)command.charAt(1)-48) * 100;
            if (packet.length()>2) {
                int colorAngle = hueSelect(command, packet.length(), 2);
                CylonEyeColor = HslColor(colorAngle / 360.0f, 1.0f, 0.25f);
            }
            animations.StartAnimation(0, 4, fadeAnimUpdate);
            animations.StartAnimation(1, duration, moveCrossedAnimUpdate);
        }

        // Fast random noise 0-3 on 1 Turns
        if (command.charAt(0) == '7') {
            int duration = ((int)command.charAt(1)-48) * 100;
            if (packet.length()>2) { 
                int colorAngle = hueSelect(command, packet.length(), 2);
                CylonEyeColor = HslColor(colorAngle / 360.0f, 1.0f, 0.25f);
            }
            debugMessage("7 rand-noise duration: "+String(duration));
            animations.StartAnimation(0, 1, allToColorNoise);
            animations.StartAnimation(1, duration, darkenAll);
        }

        // Turn to color and fade
        if (command.charAt(0) == '8') {
            int duration = ((int)command.charAt(1)-48) * 100;
            if (packet.length()>2) {
                int colorAngle = hueSelect(command, packet.length(), 2);
                CylonEyeColor = HslColor(colorAngle / 360.0f, 1.0f, 0.25f);
            }
            debugMessage("Fade duration: "+String(duration));
            animations.StartAnimation(0, 1, allToColor);
            animations.StartAnimation(1, duration, moveCrossedAnimBlackUpdate);
        }

        // Flash effect (white)
        if (command.charAt(0) == '9') {
            int duration = ((int)command.charAt(1)-48) * 50;
            CylonEyeColor.R = maxBrightness;
            CylonEyeColor.G = maxBrightness;
            CylonEyeColor.B = maxBrightness;
            debugMessage("Fade duration: "+String(duration));
            animations.StartAnimation(0, 1, allToColor);
            animations.StartAnimation(1, duration, darkenAll);
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