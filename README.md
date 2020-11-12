# Remora udpx

![Remora Logo](/extras/img/remora-370.png)

Remora is a simple pseudo animation language that works as a companion to ORCΛ receiving UDP short instructions and sending animations to addressable LED stripes.
This animations run entirely in the ESP32 controller and are initially aimed to be short so they can be triggered by [Orca sequencer](https://github.com/hundredrabbits/Orca) bangs.

Starting on Dec. 2019, the Remora project has been rebranded as **Remora udpx**, since now it supports also the Pixels protocol when it receives a udpx push. Any message that is>9 bytes will be interpreted as an animation frame. That means it can also receive video frames  from [open source udpx-app](https://github.com/martinberlin/udpx-app). You can simply install the [udpx client from the Play store](https://play.google.com/store/apps/details?id=io.cordova.udpx). 

## MIDI Partner project

![Remora Matrix Logo](https://raw.githubusercontent.com/martinberlin/Remora-matrix/2241c71170e061236acb74dfa06f1fc89e24e36d/assets/remora-matrix.svg)

As a partner project [Remora Matrix](https://github.com/martinberlin/Remora-matrix) uses [MIDI to trigger geometrical drawings in RGB Led matrixes](https://twitter.com/martinfasani/status/1320474348340334593) using FastLED.

## Bluetooth WiFi configuration

Starting in December 2019 all the configuration in the master branches of the udpx Firmware line we create will be over Bluetooth.
If you want to take advantage of this fast setup, there are [detailed instructions in udpx repository](https://github.com/martinberlin/udpx#configuring-wifi-using-bluetooth).
**Please install one of this Applications in any Android device:**

1. [udpx](https://play.google.com/store/apps/details?id=io.cordova.udpx)
2. [ESP32-WiFI-BLE](https://play.google.com/store/apps/details?id=tk.giesecke.esp32wifible)

The benefit of using udpx is that it has Bluetooth WiFi configuration and also multicast DNS discovery, so you can discover the IP address after some seconds connected. The benefit of ESP32-WiFi-BLE is that it let's you setup 2 access points, in case you have a show and need to use your mobile Hotspot. In a nutshell:

1. Start the App, power up your Remora ESP32 (First pixel BLUE)
2. Config Tab -> Select your device starting with ESP32_MAC_49161
3. Write your WiFi SSID / Password and hit Send 

That's it, if your WiFi name and password match you should be connected you get 1 GREEN led indicating you got an IP address. If the credentials are wrong or can't connect your first pixel will shine RED
Have more devices? Just go back to point 2 and set up different controllers.
You can also check this [short video](https://twitter.com/martinfasani/status/1203936678769045506) to get an idea of how to set it up with ORCΛ

Please note, if you want to use mDNS discovery for any other Firmware, follow this naming standard like we do here:
SERVICE_PORT If that's the multicast host name, then you will be able to set both IP and Port in one click. 

**Credits are due:** All logic doing the Firmware part is from Bernd Giesecke, since I followed his great example, to make Remora configurable per Bluetooth serial.

We took this decision since we find the Bluetooth way a much more standard way to configure IoT devices. Specially ESP32 that has BLE and Bluetooth serial included, should be in our philosophy the way to set fast things up and start using your controller. We refuse to keep on forcing people to connect to an access point to set up WiFi specially if you use your mobile phone as an access point. 

## Android App latest builds

If you want to test this fast using Android to send UDP messages, just download and install the udpx app from Play store:
<a href="https://play.google.com/store/apps/details?id=io.cordova.udpx" />
<img src="https://github.com/martinberlin/udpx/raw/master/examples/udpx-app-180x120.jpg" />
[udpx esp32](https://play.google.com/store/apps/details?id=io.cordova.udpx)

This is updated at least once a month so maybe won't have the latest features.
Luckycloud cloudstorage hosts the latest APK builds, if you want to check latest unstable features first uninstall the App, and download:

[Latest signed APK release](https://storage.luckycloud.de/d/0c007c42956746c186a1/?p=/android/releases&mode=list)

## Branches
 
 - **master**    stable branch with configuration over Bluetooth
 - **develop**   main branch ahead of others where latest updates are merged
 - **wifi-manager** main branch without Bluetooth configuration, but with WiFi manager (requires restart)
 - **m5/***      to be compiled on M5StickC devices

## Communications protocol

UDP will be hearing in port 49161 so you can send short instructions from Orca or any other program you desire.
This project is mainly headed to receive short UDP commands from ORCΛ but on master and develop branches also [udpx Pixel library](https://github.com/IoTPanic/pixels/) coded by IoTPanic is included.
So if the UDP payload is major than 9 pixels it will trigger the binary protocol. [udpx video Demo](http://udpx.fasani.de)

Please read the documentation of the [UDPX Repository](https://github.com/martinberlin/udpx) if you want to understand more about the communications protocol.

### Requirements and discovering the ESP32 IP Address

1. Get an ESP32 Board (Any) and compile this firmware using Plataformio or Arduino
2. Make sure to update the settings for your LED Stripe **(data PIN, Length)** and also edit lib/Config/config.h adding your WiFi credentials
3. Update the default **Orca IP** [for UDP](https://github.com/hundredrabbits/Orca/issues/135) so it knows where to send the UDP Messages. Now Remora supports mDns so if you are on Linux or Mac try to make a ping to: 

**ping SERVICE_PORT.local**

And see if the ESP32 responds to mark it's IP Address. That's how you can hit an animation from any device connected to the same network, sending an UDP message to that IP and default port 49161. Note that for windows you need to install Bonjour to resolve local mDNS to their IP Addresses. If mDns discovery it's not an option, then connect the ESP32 board to the PC and open Serial debbugging in port 115200 on Arduino or Plataformio. Reset the board and you can see the IP Address as soon it connects to WiFi.

If you use the [UDP redirector described in this section](#sending-animations-to-multiple-led-stripes) Orca can still send UDP messages to localhost and can be redirected by this background Nodejs script, enabling also the possibility to send to multiple devices from a single point. If you want to use only one, then you can just update the IP to your controller IP using the ip: command from ORCΛ

Check further details on how this works on [this blog post at fasani.de](https://fasani.de/2019/06/24/met-remora-a-simple-firmware-and-language-to-launch-addressable-led-animations-from-orc%ce%bb/)

## Hardware list

1. ESP32 Board (The cheapest one in Alixpress/Ebay. Suggested 4,50€)
2. Addressable LEDs [WS2812B](https://www.aliexpress.com/item/Individually-Addressable-RGB-LED-Strip-Light-WS2812B-SK6812-LED-Stripe-DC-5V-5050-Waterproof-Diode-Flexible/32864337987.html) (1 Mtr, 144 leds, about 10€)
3. 1 mtr Aluminium profile (About 4€ in any store like Bauhaus)
4. Additionally and if you use a Stripe larger as 72 elements then you will get a brown out error when many LEDs are on since USB delivers max. 500mA so we recommend to give power from an external source than USB:
   [SANMIN 1PCS AC110V/220V-DC5V 3A 15W Isolated switch power supply module 220 to 5v](https://www.aliexpress.com/item/SANMIN-1PCS-AC110V-220V-DC5V-3A-15W-Isolated-switch-power-supply-module-220-to-5v-black/32842935108.html)
5. Optional: [3D model to print the case](https://hackaday.io/project/166146-remora) In the project hackaday page you can file STL files for a simple case that fits a mini ESP32 

If you want to have a bigger panel, you can of course use any Led Matrix, we suggest the brand BTF-Lighting since they have awesome 22x22 or 44x11 matrix at affordable pricing.
That and soldiering 3 cables plus additing an optional 220v to 5VDC converter will do it. Usually the data cable is on the middle, so soldiering 5V positive, ground and data to GPIO19 in the ESP32 is the only electronic schema that you will need to make it work. 
Note: Check in others stores if you get a better price, in total should be not more than 19 € to put a one meter Stripe + Controller together. 

## Electronic schematics

![Schematics](/docs/Schematic_Remora.svg)

## Commands

[0-35 HSL Color angle] is optional and multiplied per 10 to give Hue angle. If it's not sent as default will play last color. Please check the [base 36 table](#base36-table) to understand how to go beyond 9 for duration and hue values. Default initial color is red.

**1**[ms*100][0-35 A Hue][0-35 B] Launch animation left with color A to B ex. 190O will take 900ms with one corner in red to blue 

**3**[ms*100][0-35 A Hue][0-35 B] Launch animation right with color B to A ex. 350O will take 500ms with one corner in red to blue 

**4**[ms*100][0-35 Hue] Launch animation left ex. 490 will take 900ms with red
4990 900ms with green (Hue 90°)

**5**[ms*100][0-35 Hue] Launch 2 chaser animations left to right and right to left

**6**[ms duration *100][0-35 HSL Color angle] Launch animation right ex. 
610 will take 100ms with color red
69O 900ms with color blue (Hue 240° since O is 240)
69C same but with color 120° green

**7**[ms*100][0-35 Hue] Makes noise (random on/off) all along the stripe

**8**[ms*100][0-35 Hue] Turn all to desired Hue color and fade to black dissapearing on the center

**9**[ms*50] Short white flash

**0**[ms*100][Note:AG or ag][0-35 Hue] Divides the Lenght of the stripe in 7 and simulates the note being played. Ex. 02A0 will play an A note for 200 ms in red color (Hue 0)

**x**[coordinate] Light only one Led to last color and leave it on. Ex. ;x1 will light the first led on the stripe

**X**[coordinate] Light only one Led and fade in 100ms to black

To open a detailed list of Orca examples please check [docs/Remora-commands.orca](docs/Remora-commands.orca) or check the preview and video in [Patchstorage](https://patchstorage.com/remora-udp-animations/)

**Switch colors without doing any animation**

Experimental and not thoughfully tested. Useful if you want to keep different animations running but trigger the colors universally.

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


## Sending animations to multiple LED Stripes

Change directory to extras. Run:   
**nodejs udproxy.js**

Configure your Id and IP combinations in ip-config.json 

    {
        "1" : "192.168.0.2", 
        "2" : "192.168.0.3" 
    }
  
 From Orca: Prepend your animations using this one character to lookup the IP 
 Ex.  ;**1**52  Will lookup IP with id 1 and redirect the rest of the message (52) to 192.168.0.2
 This will enable to run multiple ID stripes using a single IP address as a central point.
 
#### Proof of concept videos

[Testing this firmware after upload](https://www.youtube.com/watch?v=ZHNhSbunzAY)

[Orca/Remora example](https://www.youtube.com/watch?v=C8OmwIaXQIE)

[Twitter videos](https://twitter.com/martinfasani/media)

## Support 

If you find a bug please make a git issue explaining how to reproduce it. For any other support please send mention me [in twitter](https://twitter.com/martinfasani) and I will add you as a contact so you can send me a private message. 

Please feel free to fork this and make it yours adding new animation ideas. Pull requests are welcome!

## Companion Applications

- [ORCΛ](https://github.com/hundredrabbits/orca), ORCΛ Sequencer
- [UDProxy](/extras) UDP is a simple nodejs script to enable sending animations to multiple Led stripes.
- [Remora Matrix](https://github.com/martinberlin/Remora-matrix)
- [udpx Firmware](https://github.com/martinberlin/udpx) can receive larger UDP packets since has a Brotli and Zlib decompressor built in. It does not animations directly in ESP32, it just receives frames, so it's a better option if you want to display video in a LED Matrix


### Sponsoring

If you like this Firmware please consider becoming a sponsor where you can donate as little as 2 u$ per month. Just click on:
**❤ Sponsor**  on the top right

♢ For cryptocurrency users is also possible to help this project transferring Ethereum:

     0x68cEAB84F33776a7Fac977B2Bdc0D50933344086

We are thankful for the support and contributions so far! 
Every small quantity of digital gold helps to spend more time giving support and developing further this projects.

