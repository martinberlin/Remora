#include "Animate.h"
#include "AsyncUDP.h"

// <Configure> this to your own setup:
#define DEFAULT_HUE_ANGLE 0
const uint16_t PixelCount = 131;     // Length of LED stripe 144 - 13 = 131 Leds in a 30cm diameter round
const uint8_t  PixelPin = 26;        // Data line of Addressable LEDs
float maxL = 0.2f;
struct RgbwColor CylonEyeColor(HslColor(0.0f, 1.0f, maxL)); // Red as default
boolean enableBeatDetection = true; // Turn to true to enable Mic beat detection
byte maxBrightness = 20;             // 0 to 255 - Only for RGB

// </Configure>
uint16_t lastSignalLevel = 0;
#define READ_LEN (50)
uint8_t BUFFER[READ_LEN] = {0};
uint16_t *adcBuffer = NULL;

// Sent from main.cpp:
struct config {
  int udpPort; 
  String ipAddress;
} animateConfig;
// Message transport protocol
AsyncUDP udp;

// NOTE: Make sure to check what Feature is the right one for your LED Stripe: https://github.com/Makuna/NeoPixelBus/wiki/NeoPixelBus-object#neo-features
NeoPixelBus<NeoGrbwFeature, Neo800KbpsMethod> strip(PixelCount, PixelPin);
NeoPixelAnimator animations(2);
NeoGamma<NeoGammaTableMethod> colorGamma;
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
    // Constructor: Let's start sampling here
       i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM),
    .sample_rate =  44100,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT, // is fixed at 12bit, stereo, MSB
    .channel_format = I2S_CHANNEL_FMT_ALL_RIGHT,
    .communication_format = I2S_COMM_FORMAT_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 2,
    .dma_buf_len = 128,
   };

   i2s_pin_config_t pin_config;
   pin_config.bck_io_num   = I2S_PIN_NO_CHANGE;
   pin_config.ws_io_num    = PIN_CLK;
   pin_config.data_out_num = I2S_PIN_NO_CHANGE;
   pin_config.data_in_num  = PIN_DATA;
  
   
   i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
   i2s_set_pin(I2S_NUM_0, &pin_config);
   i2s_set_clk(I2S_NUM_0, 44100, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);
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
    RgbwColor color;
    for (uint16_t indexPixel = 0; indexPixel < strip.PixelCount(); indexPixel++)
    {
        color = strip.GetPixelColor(indexPixel);
        color.Darken(darkenBy);
        strip.SetPixelColor(indexPixel, color);
    }
}
void darkenAll(const AnimationParam& param)
{
    RgbwColor color;
    for (uint16_t indexPixel = 0; indexPixel < strip.PixelCount(); indexPixel++)
    {
        color = strip.GetPixelColor(indexPixel);
        color.Darken(2);
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
        byte rand = random(5);
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

void DrawPixelColorToColor(bool corrected, HslColor startColor, HslColor stopColor)
{
    for (uint16_t index = 0; index < strip.PixelCount(); index++)
    {
        float progress = index / static_cast<float>(strip.PixelCount() - 2);
        RgbwColor color = HslColor::LinearBlend<NeoHueBlendShortestDistance>(startColor, stopColor, progress);
        if (corrected)
        {
            color = colorGamma.Correct(color);
        }
        strip.SetPixelColor(index, color);
    }
}

/**
 * Convert incoming char to base36 and return it multiplied by 10
 */
int commandToBase36(String command, uint8_t offset) {
    int colorAngle = DEFAULT_HUE_ANGLE;
    int inputChar = (int)command.charAt(offset);
    if (inputChar>47 && inputChar<58) {
        colorAngle = inputChar-48;
    }
    if (inputChar>64 && inputChar<91) {
        colorAngle = inputChar-55;
    }
    return colorAngle*10;
}

/**
 * Return int with Hue angle analysing given command ex. offset 2: ;51240 will return 240 (blue)
 */
int commandToInt(String command, int length, uint8_t offset) {
    int colorAngle = DEFAULT_HUE_ANGLE; // Red as default (Maybe make default constant)
    if (length-offset == 1) {
        colorAngle = ((int)command.charAt(offset)-48);
    }
    if (length-offset == 2) {
        colorAngle = (((int)command.charAt(offset)-48)*10); // Dec
        colorAngle += ((int)command.charAt(offset+1)-48);
    }
    if (length-offset == 3 && ((int)command.charAt(offset+2)-48)<4) {
        colorAngle = (((int)command.charAt(offset)-48)*100); // Hundreds
        colorAngle += ((int)command.charAt(offset+1)-48)*10;   // Dec
        colorAngle += ((int)command.charAt(offset+2)-48);
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

    // Callback that gets fired every time an UDP Message arrives
    udp.onPacket([](AsyncUDPPacket packet) {
        if(debugMode) {
            Serial.print("Data L "+String(packet.length())+" : ");
            Serial.write(packet.data(), packet.length());Serial.println();
        }
        String command;
      
        for ( int i = 0; i < packet.length(); i++ ) {
            command += (char)packet.data()[i];
        }

        // Chords A -> G (65 -> 72)
        if (command.charAt(0) == '0' && (int)command.charAt(2)>64) {
            int duration = commandToBase36(command, 1) * 10;
            if (packet.length()>3) {
                int colorAngle = commandToInt(command, packet.length(), 3);
                CylonEyeColor = HslColor(colorAngle / 360.0f, 1.0f, maxL);
            }
            // A -> G
            byte note = (int)command.charAt(2)-65;
            byte noteLength = PixelCount/7;
            // a -> g
            if ((int)command.charAt(2)>96 && (int)command.charAt(2)<104) {
               note = (int)command.charAt(2)-96; 
               noteLength = noteLength /2;
            }
            for (uint16_t x = note*noteLength+1; x < (note+1)*noteLength; x++){
                strip.SetPixelColor(x, CylonEyeColor);
            }
            animations.StartAnimation(1, duration, darkenAll);
            return;
        }
        //debugMessage("LEN:"+String(packet.length() ));
        // ->
        if (command.charAt(0) == '6') {
            lastPixel = 0;
            moveDir = 1;
            // ord("1") is 49 in the ascii table
            int duration = commandToBase36(command, 1) * 10;
            if (packet.length()>2) { // Only change the color if Hue angle is sent otherwise keep last Hue
                int colorAngle = commandToBase36(command, 2);
                CylonEyeColor = HslColor(colorAngle / 360.0f, 1.0f, maxL);
                debugMessage("colorAngle:"+String(colorAngle));
            }
            debugMessage("> duration: "+String(duration));
            animations.StartAnimation(0, 4, fadeAnimUpdate);
            animations.StartAnimation(1, duration, moveAnimUpdate);
        }

        // <-
        if (command.charAt(0) == '4') {
            lastPixel = PixelCount;
            moveDir = -1;
            int duration = commandToBase36(command, 1) * 10;
            if (packet.length()>2) {
                int colorAngle = commandToBase36(command, 2);
                CylonEyeColor = HslColor(colorAngle / 360.0f, 1.0f, maxL);
            }
            animations.StartAnimation(0, 4, fadeAnimUpdate);
            animations.StartAnimation(1, duration, moveAnimUpdate);
        }


        if (command.charAt(0) == '3') {
            lastPixel = PixelCount;
            int duration = commandToBase36(command, 1);
            if (packet.length()>3) {
                DrawPixelColorToColor(true, HslColor(commandToBase36(command, 2) / 360.0f, 1.0f, 0.1f), HslColor(commandToBase36(command, 3) / 360.0f, 1.0f, maxL));
                animations.StartAnimation(0, duration, fadeAnimUpdate);
            }
            
        }

        if (command.charAt(0) == '1') {
            int duration = commandToBase36(command, 1);
            if (packet.length()>3) {
                DrawPixelColorToColor(true, HslColor(commandToBase36(command, 2) / 360.0f, 1.0f, maxL), HslColor(commandToBase36(command, 3) / 360.0f, 1.0f, 0.1f));
                animations.StartAnimation(0, duration, fadeAnimUpdate);
            }
        } 

        // 2 chasers left->right right<-left
        if (command.charAt(0) == '5') {
            lastPixel = 0;
            rightPixel = PixelCount;
            int duration = commandToBase36(command, 1) * 10;
            if (packet.length()>2) {
                int colorAngle = commandToBase36(command, 2);
                CylonEyeColor = HslColor(colorAngle / 360.0f, 1.0f, maxL);
            }
            animations.StartAnimation(0, 4, fadeAnimUpdate);
            animations.StartAnimation(1, duration, moveCrossedAnimUpdate);
        }

        // Fast random noise 0-3 on 1 Turns
        if (command.charAt(0) == '7') {
            int duration = commandToBase36(command, 1) * 10;
            if (packet.length()>2) { 
                int colorAngle = commandToBase36(command, 2);
                CylonEyeColor = HslColor(colorAngle / 360.0f, 1.0f, maxL);
            }
            debugMessage("7 rand-noise duration: "+String(duration));
            animations.StartAnimation(0, 1, allToColorNoise);
            animations.StartAnimation(1, duration, darkenAll);
        }

        // Turn to color and fade
        if (command.charAt(0) == '8') {
            int duration = commandToBase36(command, 1) * 10;
            if (packet.length()>2) {
                int colorAngle = commandToBase36(command, 2);
                CylonEyeColor = HslColor(colorAngle / 360.0f, 1.0f, maxL);
                debugMessage("Hue: "+String(colorAngle));
            }
            debugMessage("Fade duration: "+String(duration));
            animations.StartAnimation(0, 1, allToColor);
            animations.StartAnimation(1, duration, moveCrossedAnimBlackUpdate);
        }

        // Flash effect (white)
        if (command.charAt(0) == '9') {
            int duration = commandToBase36(command, 1) * 10;
            CylonEyeColor.R = maxBrightness;
            CylonEyeColor.G = maxBrightness;
            CylonEyeColor.B = maxBrightness;
            debugMessage("Fade duration: "+String(duration));
            animations.StartAnimation(0, 1, allToColor);
            animations.StartAnimation(1, duration, darkenAll);
        }
        // Light a single X and fade to black
        if (command.charAt(0) == 'X') {
            if (packet.length()>1) {
                int x = commandToInt(command, packet.length(), 1);
                debugMessage("X: "+String(x));
                if (x <= PixelCount) {
                  strip.SetPixelColor(x, CylonEyeColor);
                  animations.StartAnimation(1, 100, darkenAll);  
                }
            }
        }
        // Light a single X and leave it on
        if (command.charAt(0) == 'x') {
            if (packet.length()>1) {
                int x = commandToInt(command, packet.length(), 1);
                debugMessage("x: "+String(x));
                if (x <= PixelCount) {
                  strip.SetPixelColor(x, CylonEyeColor);
                }
            }
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


void micRead() 
{   

    i2s_read_bytes(I2S_NUM_0, (char*) BUFFER, READ_LEN, (100 / portTICK_RATE_MS));
    adcBuffer = (uint16_t *)BUFFER;
    
    int signalLevel = 0;   
    for ( int i = 0; i < READ_LEN; i++ ) {
       signalLevel += adcBuffer[i];
    }
    signalLevel = ((signalLevel/READ_LEN)/100 -290)*3;
    
   if (signalLevel>=30 && signalLevel<100 && (signalLevel!=lastSignalLevel)) {
    // BEAT
      Serial.println(signalLevel);
      
      animations.StartAnimation(0, 1, allToColorNoise);
      animations.StartAnimation(1, signalLevel, darkenAll);
      lastSignalLevel = signalLevel;
     }
}

void Animate::loop() {
    if(animations.IsAnimating()) {
        animations.UpdateAnimations();
    }
    
    strip.Show();
    if (enableBeatDetection) {
        micRead();
    }
}