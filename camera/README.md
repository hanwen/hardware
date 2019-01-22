IR camera
=========

This is a small IR camera based on the MLX90640 IR array.


Hardware
========

* Sparkfun IR Array Breakout 55 degree FoV, MLX90640 
* ESP8266 based NodeMCU board (I used LoLin v3)
* 12x patch cable 10cm female-female (2.54mm pitch)
* 1x Perspex sheet, ca. 200 x 100mmm
* 12x M2.5 screws (10mm),
* 24x M2.5 nuts
* 1x ST7735 based TFT display 128x160,

Instructions
============

1. Wire up IR

   D1 (nodemcu) - SDA (sensor) 
   D2 (nodemcu) - SDK (sensor) 
   3v (nodemcu) - 3.3V (sensor)
   G  (nodemcu) - GND (sensor)

2. Wire up TFT

    nodemcu  - TFT
    3v - LED
    D5 - SCK
    D7 - SDA
    D4 - A0/DC
    D0 - RST
    D8 - CS
    G  - GND
    VCC - 3V

3. Flash firmware & test.
4. Drill holes in perspex. Don't forget hole for the FLASH button.
5. Mount components. All components on the same side; TFT display facing the glass.



Wifi
====

Press the FLASH button to enter Wifi configuration. Connect to the
"IRCameraAP" access point. A captive portal should redirect to a
configuration page. Configure network and password. 

Once Wifi is enabled, the display shows the IP address

The IP address supports the following endpoints:

   * /temp - dump temperatures as a 32x24 JSON nested array
   * /parameter - get and set parameter values.
   * / 


LICENSE
=======

This is based on code from 

    https://github.com/sparkfun/SparkFun_MLX90640_Arduino_Example/

which is MIT licensed; the MLX90640 driver is licensed under the
Apache license
