# Remora

![Remora Logo](/docs/remora-370.png)

Remora will be a simple pseudo animation language to be a companion to Orca receiving UDP short instructions and sending animations to an addressable LEDs stripe.
This animations will run entirely in an ESP32 controller and are initially aimed to be short so they can be triggered by [Orca sequencer](https://github.com/hundredrabbits/Orca) bangs

## Communications protocol

UDP will be hearing in a configurable port so you can send short instructions from Orca or any other program you desire

### Requirements

1. Get an ESP32 Board (Any) and compile this firmware using Plataformio or Arduino
2. Make sure to update the settings for your LED Stripe **(data PIN, Length)** and also edit lib/Config/config.h adding your WiFi credentials
3. Update the default **Orca IP** [for UDP](https://github.com/hundredrabbits/Orca/issues/135) so it knows where to send the UDP Messages. Now Remora supports mDns so if you are on Linux or Mac try to make a ping to: 

led.local

And see if the ESP32 responds. For windows you need to install Bonjour for this to work. Update the default led.local in Config.h if you need another name.

Update the default localhost IP used by Orca in the file:
desktop/core/io/udp.js this.play line. Replace this by the IP of that the controller displays in Serial (Or by led.local if your system supports mDns).
Note: Doing this will disable any other localhost running UDP communications from Orca to another programs

## Hardware list

1. ESP32 Board (The cheapest one. Suggested 4,50€)
2. Addressable LEDs [WS2812B](https://www.aliexpress.com/item/Individually-Addressable-RGB-LED-Strip-Light-WS2812B-SK6812-LED-Stripe-DC-5V-5050-Waterproof-Diode-Flexible/32864337987.html) (1 Mtr, 144 leds, about 10€)
3. 1 mtr Aluminium profile (About 4€ in any store like Bauhaus)
4. Additionally and if you use a Stripe larger as 72 elements then you will get a brown out error when many LEDs are on since USB delivers max. 500mA so we recommend to give power from an external source than USB:
   [SANMIN 1PCS AC110V/220V-DC5V 3A 15W Isolated switch power supply module 220 to 5v](https://www.aliexpress.com/item/SANMIN-1PCS-AC110V-220V-DC5V-3A-15W-Isolated-switch-power-supply-module-220-to-5v-black/32842935108.html)
  
That and soldiering 3 cables plus additing an optional 220v to 5VDC converter will do it. Usually the data cable is on the middle, so soldiering 5V positive, ground and data to a PIN in the ESP32 is the only electronic schema that you will need to make it work. 
Note: Links are not affiliate links and are given just as a reference, check in others stores if you get a better price, I calculate in total should be not more than 19 € to put a one meter Stripe + Controller together.

## Instructions

At the beginning instructions will be kept to a minimun just as a demo. The idea is that anyone can fork this and add their own customizations. Friendly pull requests are welcome. 
UDP commands can be sent from Orca using the ;COMMAND
Minimum brightness value will be 0 and maximum 99 and the instructions will be kept as short as possible.


## Commands

[0-35 HSL Color angle] is optional and multiplied per 10 to give Hue angle. If it's not sent as default will play last color. Please check the table to understand how to go beyond 9. That is useful if you want to keep different animations running but trigger the colors universally. Default initial color is red.

**6**[ms duration *100][0-35 HSL Color angle] Launch animation right ex. 
610 will take 100ms with color red
69O 900ms with color blue (Hue 240° since O is 240)
69C same but with color 120

**4**[ms*100][0-35 Hue] Launch animation left ex. 490 will take 900ms with red
4990 900ms with green (Hue 90°)

**5**[ms*100][0-35 Hue] Launch 2 chaser animations left to right and right to left

**7**[ms*100][0-35 Hue] Make Noise (random on/off) all along the stripe

**8**[ms*100][0-35 Hue] Turn all to desired Hue color and fade to black

**9**[ms*50] Short white flash

**x**[coordinate] Light only one Led to last color and leave it on. Ex. ;x1 will light the first led on the stripe

**X**[coordinate] Light only one Led and fade in 100ms to black

For some Orca examples please refer to docs/*.orca.

**Switch to pure colors without doing any animation**

**r** Switch color to red

**b** Switch color to green

**g** Switch color to blue

**v** Switch color to violet

**y** Switch color to yellow

**w** Switch color to white

## Base36 Table

In this branch we will emulate Orca's **36 increments**. Operators using numeric values will typically also operate on letters and convert them into values as per the following table.
This will be used for the color angle of Hue. For example N will represent 24*10=240 (blue)

| **0** | **1** | **2** | **3** | **4** | **5** | **6** | **7** | **8** | **9** | **A** | **B**  | 
| :-:   | :-:   | :-:   | :-:   | :-:   | :-:   | :-:   | :-:   | :-:   | :-:   | :-:   | :-:    | 
| 0     | 1     | 2     | 3     | 4     | 5     | 6     | 7     | 8     | 9     | 10    | 11     |
| **C** | **D** | **E** | **F** | **G** | **H** | **I** | **J** | **K** | **L** | **M** | **N**  |
| 12    | 13    | 14    | 15    | 16    | 17    | 18    | 19    | 20    | 21    | 22    | 23     |
| **O** | **P** | **Q** | **R** | **S** | **T** | **U** | **V** | **W** | **X** | **Y** | **Z**  | 
| 24    | 25    | 26    | 27    | 28    | 29    | 30    | 31    | 32    | 33    | 34    | 35     |

#### Future color ideas

Probably all this color operations will be replaced to a simple 

**R**__**G**__**B**__

OR

**H**___**S**__**L**__ Hue wheel angle/ saturation

**6**{Length}**R**{Bright: 2 digits from 00 to 99}   example:
      **610R99**  Will make a 10 Red LEDs in maximun brightness slide right to the end and dissapear

**4** Any running animation will change direction to left

#### Coming soon: 

**5** Crossing lights path, left to right, right to left

**6|** Go rightwards and bump at the end to retun leftwards

**6**{Length}**R**{Bright: 2 digits from 00 to 99}   example:
      **>10R99**  Will make a 10 Red LEDs in maximun brightness slide right to the end and dissapear

**>02R99G99B00**  Will make a 2 Yellow LEDs go to the right and dissapear

**ALL_R00G99B00** Fade all LEDs to color green

**X25B99** Turn LED 25 into maximun Blue

**X25R99G99B99** Turn LED 25 into maximun Red, Green, Blue (White)

Please feel free to fork this and add your new animation ideas. Pull requests are welcome!

#### Proof of concept video

[Take 1](https://twitter.com/martinfasani/status/1136687580374798341)
[youtube Orca/Remora Take 2](https://www.youtube.com/watch?v=C8OmwIaXQIE)

## Companion Applications

- [Orca](https://github.com/hundredrabbits/orca), ORCΛ Sequencer
