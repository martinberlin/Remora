# Remora

Remora will be a simple pseudo animation language to be a companion to Orca receiving UDP short instructions and sending animations to an addressable LEDs stripe.
This animations will run entirely in an ESP32 controller and are initially aimed to be short so they can be triggered by Orca bangs

## Communications protocol

UDP will be hearing in a configurable port so you can send short instructions from Orca or any other program you desire

## Instructions

At the beginning instructions will be kept to a minimun and here is our initial short list. 
UDP commands can be sent from Orca using the ;COMMAND
Minimum brightness value will be 0 and maximum 99 and the instructions will be kept as short as possible.

**FPS**30 Set frames per second (As default 30)
R99 Existing animation will go rightwards ( ;> )

**<** Any running animation will change direction to left

**>**{Length}**R**{Bright: 2 digits from 00 to 99}   example:
      **>10R99**  Will make a 10 Red LEDs in maximun brightness slide right to the end and dissapear

**>02R99G99B00**  Will make a 2 Yellow LEDs go to the right and dissapear

This will be the initial animations to develop.
Coming in next phase we can imagine more combinations like:

**>|** Go rightwards and bump at the end to retun leftwards
**ALL_R00G00B00** Turn all LEDs to that color
**X25R99** Turn LED 25 into maximun Red
**X25R99G99B99** Turn LED 25 into maximun Red, Green, Blue (White)