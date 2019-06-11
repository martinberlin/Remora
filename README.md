# Remora

![Remora Logo](/docs/remora.png)

Remora will be a simple pseudo animation language to be a companion to Orca receiving UDP short instructions and sending animations to an addressable LEDs stripe.
This animations will run entirely in an ESP32 controller and are initially aimed to be short so they can be triggered by [Orca sequencer](https://github.com/hundredrabbits/Orca) bangs

## Communications protocol

UDP will be hearing in a configurable port so you can send short instructions from Orca or any other program you desire

### Requirements

1. Get an ESP32 Board (Any) and compile this firmware using Plataformio or Arduino
2. Make sure to update the settings for your LED Stripe **(data PIN, Length)** and also edit lib/Config/config.h adding your WiFi credentials
3. Update the default **Orca IP** [for UDP](https://github.com/hundredrabbits/Orca/issues/135)

The default localhost IP needs to be updated in Orca in the file:
desktop/core/io/udp.js this.play line. Replace this by the IP of that the controller displays in Serial.
Note: Doing this will disable any other localhost running UDP communications from Orca

## Hardware list

1. ESP32 Board (The cheapest one. Suggested 4,50€)
2. Addressable LEDs [WS2812B](https://www.aliexpress.com/item/Individually-Addressable-RGB-LED-Strip-Light-WS2812B-SK6812-LED-Stripe-DC-5V-5050-Waterproof-Diode-Flexible/32864337987.html) (1 Mtr, 144 leds, about 10€)
3. 1 mtr Aluminium profile (About 4€ in any store like Bauhaus)
4. Additionally and if you use a Stripe larger as 72 elements then you will get a brown out error when many LEDs are on since USB delivers max. 500mA so we recommend to give power from an external source than USB:
   [SANMIN 1PCS AC110V/220V-DC5V 3A 15W Isolated switch power supply module 220 to 5v](https://www.aliexpress.com/item/SANMIN-1PCS-AC110V-220V-DC5V-3A-15W-Isolated-switch-power-supply-module-220-to-5v-black/32842935108.html)
  
That and soldiering 3 cables plus additing an optional 220v to 5VDC converter will do it. Usually the data cable is on the middle, so soldiering 5V positive, ground and data to a PIN in the ESP32 is the only electrical. 
Note: Links are not affiliate links and are given just as a reference, check in others stores if you get a better price, I calculate in total should be not more than 19 € to put a one meter Stripe + Controller together.

## Instructions

At the beginning instructions will be kept to a minimun just as a demo. The idea is that anyone can fork this and add their own customizations. Friendly pull requests are welcome. 
UDP commands can be sent from Orca using the ;COMMAND
Minimum brightness value will be 0 and maximum 99 and the instructions will be kept as short as possible.

### Current demo set

The current demo instructions are just temporary as a proof-of-concept approach in the branch feature/animation
 https://github.com/martinberlin/Remora/tree/feature/animation

## See docs/commands.orca

**6{ms duration *100}** Launch animation right ex. 61 will take 100ms

**4{ms duration *100}** Launch animation left ex. 49 will take 900ms

**p(rgbyw)** ex pr Will render pure red (and 

Probably all this color operations will be replaced to a simple 

CR__G__B__

OR

H___S__L__ Hue wheel angle/ saturation

**6**{Length}**R**{Bright: 2 digits from 00 to 99}   example:
      **610R99**  Will make a 10 Red LEDs in maximun brightness slide right to the end and dissapear

**4** Any running animation will change direction to left

Coming soon: 

**5** Crossing lights path, left to right, right to left

This will be the initial animations to develop.

#### Coming in next phase we can imagine more combinations like:

**6|** Go rightwards and bump at the end to retun leftwards

**6**{Length}**R**{Bright: 2 digits from 00 to 99}   example:
      **>10R99**  Will make a 10 Red LEDs in maximun brightness slide right to the end and dissapear

**>02R99G99B00**  Will make a 2 Yellow LEDs go to the right and dissapear

**ALL_R00G99B00** Fade all LEDs to color green

**X25B99** Turn LED 25 into maximun Blue

**X25R99G99B99** Turn LED 25 into maximun Red, Green, Blue (White)

#### Proof of concept video

[Take 1](https://twitter.com/martinfasani/status/1136687580374798341)
