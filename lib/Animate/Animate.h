/*
  Interface of Led Animations class
*/
#ifdef ESP8266
    #include "IPAddress.h"
#endif
#ifndef neopixelbus_h
    #include <NeoPixelBus.h>
    #include <NeoPixelAnimator.h>
#endif
// Uncomment for RGBW (with 4 leds per pixel)
//#define RGBW 
typedef struct{
    uint8_t R = 0;
    uint8_t G = 0;
    uint8_t B = 0;
    #ifdef RGBW
    uint8_t W = 0;
    #endif
} pixel;

class Animate
{
  public:
    Animate();
    // Print out messages in Serial
    static const boolean debugMode = true;
    void startUdpListener(const IPAddress& ipAddress, int udpPort);
    void micRead();
    String ipAddress2String(const IPAddress& ipAddress);

    void loop();
    
    // Credits: @IoTpanic
    // Receive requires a pointer to a uint8_t array and the length of the array from a callback function
    bool receive(uint8_t *pyld, unsigned len);
    // Write sets the LED at the location to R,G,B to the values provided for the next show()
    void write(unsigned location, uint8_t R, uint8_t G, uint8_t B, uint8_t W = 0);
    // Show writes the previously made write() calls to the array of LEDs
    void show();
    // This version of show takes a pointer to an array of pixels, as well as how long the array is. Be sure the array is in order from LED location 0 onward
    // There is another versio that will accapt a single pixel and location
    void show(pixel *pixels, unsigned cnt);

    private:
    // unmarshal returns a pointer to an array of pixels and accepts a pointer to a uint8_t array payload with the length of the array, as
    // well as a pointer to an unsigned integer which will be changed to the number of LEDs decoded from the payload. If invalid a NULL will
    // be returned and the value at pixCnt will be set to zero.
    pixel *unmarshal(uint8_t *pyld, unsigned len, uint16_t *pixCnt, uint8_t *channel=NULL);

    uint8_t syncWord = 0x0;

    // We want to inform our lib if RGB or RGBW was selected
    #ifdef RGBW
    const bool RGBWE = true;
    #else
    const bool RGBWE = false;
    #endif
};

