IR camera
=========

This is a small IR camera based on the MLX90640 IR array.


Hardware
========

* Sparkfun IR Array Breakout 55 degree FoV, MLX90640 
* ESP8266 based NodeMCU board (I used LoLin v3)
* 12x patch cable 2.54mm pitch, female-female
* 1x Perspex sheet, ca. 300 x 150mmm
* 12x M2.5 screws (10mm),
* 24x M2.5 nuts
* 1x ST7735 based TFT display 128x160,

Instructions
============

1. Wire up IR (TODO: pinout)
2. Wire up TFT (TODO: pinout)
3. Flash firmware & test.
4. Drill holes in perspex
5. Mount components (all components on the same side; TFT display facing the glass)

LICENSE
=======

This is based on code from 

    https://github.com/sparkfun/SparkFun_MLX90640_Arduino_Example/

which is MIT licensed; the MLX90640 driver is licensed under the
Apache license

TODO
====

Serve temperature data over wifi