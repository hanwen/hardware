/*

Copyright (c) 2019 Google Inc.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include <Wire.h>

#include "SparkFun_SCD30_Arduino_Library.h"

#include "SSD1306.h"

SSD1306 *lcd = NULL;
SCD30 airSensor;

void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println();

  // NodeMCU has GPIO 5, 4 as D1, D2. Take care to select the right board.
  lcd = new SSD1306(0x3c, D1, D2);

  if (lcd) {
    lcd->init();
    lcd->flipScreenVertically();
    lcd->drawString(0, 0, "SCD30");
    lcd->display();
  }

  for (int i = 10; i--;) {
    if (airSensor.begin()) {
      break;
    }
    Serial.println("failed init'ing sensor");
    delay(1000);
    if (i == 0) {
      Serial.println("failed after 10 retries.");
      for (;;);
    }
  }

  airSensor.setAltitudeCompensation(540);
}

int lastSecs = -1;
int interval = 300;

uint16 co2;
float temp;
float humidity;

const int n = 96;
uint16_t val[n];


const int baseY = 11;

void loop() {
  int secs = millis() / 1000;
  bool gotData = false;

  if (airSensor.dataAvailable())
  {
    gotData = true;
    co2 = airSensor.getCO2();
    temp = airSensor.getTemperature();
    humidity = airSensor.getHumidity();
    Serial.println("got data");
  }

  if (secs / (interval) != lastSecs / (interval)) {
    for (int i = n - 1; i >= 1; i--) {
      val[i] = val[i - 1];
    }
    val[0] = co2;
  }
  lastSecs = secs;


  if (gotData) {
    Serial.println("CO2 " + String(co2) + " T "
                   + String(airSensor.getTemperature(), 0) + "C H" + String(airSensor.getHumidity(), 0) + "%");

    if (lcd) {
      lcd->clear();

      int subdiv = 10;
      for (int i = 0 ; i < n; i++) {
        lcd->setPixel(127 - i, transformPPM(val[i]));

        if (i % subdiv == 0) {
          lcd->setPixel(127 - i, transformPPM(1000));
          lcd->setPixel(127 - i, transformPPM(1500));
          lcd->setPixel(127 - i, transformPPM(500));
        }

      }
      lcd->drawString(0, transformPPM(1500) - 5, "1500");
      lcd->drawString(0, transformPPM(1000) - 5, "1000");
      lcd->drawString(0, transformPPM(500) - 5, "500");

      int totalSecs = subdiv * interval;
      int totalMins = totalSecs / 60;
      int totalHours = totalMins / 60;

      String label;

      if (totalMins > 2) {
        label = String(totalMins, 10) + "m/dot";
      } else {
        label = String(totalSecs, 10) + "s/dot";
      }

      lcd->drawString(0, 0, "CO2 " + String(co2) + " ppm");
      lcd->drawString(0, 10, "T " + String(airSensor.getTemperature(), 0) + "C H" + String(airSensor.getHumidity(), 0) + "%  " + label);
      lcd->display();
    }

  }
  delay(1000);
}

int transformPPM(uint16_t ppm) {
  int y = (baseY + (ppm - 400) / 60);
  if (y > 50) {
    y = 50;
  }
  if (y <= 0) {
    y =0;
  }
  
  return 63 - y;
}
