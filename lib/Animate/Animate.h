/*
  Interface of Led Animations class
*/
#ifndef neopixelbus_h
    #include <NeoPixelBus.h>
    #include <NeoPixelAnimator.h>
#endif
#define PIN_CLK  0
#define PIN_DATA 34

#include <driver/i2s.h>

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
    
};

