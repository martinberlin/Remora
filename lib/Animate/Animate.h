/*
  Interface of Led Animations class
*/
#ifndef neopixelbus_h
    #include <NeoPixelBus.h>
    #include <NeoPixelAnimator.h>
#endif
class Animate
{
  public:
    Animate();
    static const boolean debugMode = true;
    void startUdpListener(const IPAddress& ipAddress, int udpPort);

    String ipAddress2String(const IPAddress& ipAddress);

    void loop();
    
};

