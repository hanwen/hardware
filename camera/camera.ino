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

#include "MLX90640_API.h"
#include "MLX90640_I2C_Driver.h"

// Wifi stuff.
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>

std::unique_ptr<ESP8266WebServer> web;

const char *apName = "IRCameraAP";
const byte MLX90640_address = 0x33; // Default 7-bit unshifted address of the MLX90640

#define TA_SHIFT 8 // Default shift for MLX90640 in open air

static float mlx90640To[768];
paramsMLX90640 mlx90640;

void setup()
{
  setupTFT();
  Wire.begin(D1, D2);

  // Increase I2C clock speed to 400kHz
  Wire.setClock(400000);

  Serial.begin(115200);
  while (!Serial);

  if (!isConnected()) {
    Serial.println("MLX90640 not detected at default I2C address. Please check wiring. Freezing.");
    while (1);
  }
  Serial.println("MLX90640 online!");

  // Get device parameters - We only have to do this once
  int status;
  uint16_t eeMLX90640[832];
  status = MLX90640_DumpEE(MLX90640_address, eeMLX90640);
  if (status != 0)
    Serial.println("Failed to load system parameters");

  status = MLX90640_ExtractParameters(eeMLX90640, &mlx90640);
  if (status != 0)
    Serial.println("Parameter extraction failed");

  MLX90640_SetRefreshRate(MLX90640_address, 0x03);

  // Once EEPROM has been read at 400kHz we can increase to 1MHz
  Wire.setClock(1000000);

  // Once params are extracted, we can release eeMLX90640 array
}

int lastButton;
int button;

void loop()
{
  lastButton  = button;
  button = digitalRead(D3);

  int start = millis();
  int mode = MLX90640_GetCurMode(MLX90640_address);
  for (byte x = 0 ; x < 2 ; x++) // Read both subpages
  {
    uint16_t mlx90640Frame[834];

    int status = MLX90640_GetFrameData(MLX90640_address, mlx90640Frame);
    if (status < 0)
    {
      Serial.print("GetFrame Error: ");
      Serial.println(status);
    }
    float vdd = MLX90640_GetVdd(mlx90640Frame, &mlx90640);
    float Ta = MLX90640_GetTa(mlx90640Frame, &mlx90640);

    float tr = Ta - TA_SHIFT; // Reflected temperature based on the sensor ambient temperature
    float emissivity = 0.95;

    MLX90640_CalculateTo(mlx90640Frame, &mlx90640, emissivity, tr, mlx90640To);
  }
  int readMillis = millis() - start;
  MLX90640_BadPixelsCorrection(mlx90640.outlierPixels, mlx90640To, mode, &mlx90640);
  MLX90640_BadPixelsCorrection(mlx90640.brokenPixels, mlx90640To, mode, &mlx90640);

  if (button == 0 && lastButton == 1) {
    configWifi();
  }

  displayTemps(mlx90640To, readMillis);

  if (WiFi.status() == WL_CONNECTED) {
    wifiMessage("IP: " +  WiFi.localIP().toString());

    if (web == NULL) {
      setupWebserver();
    }

    web->handleClient();
  }
}

// Returns true if the MLX90640 is detected on the I2C bus
boolean isConnected()
{
  Wire.beginTransmission((uint8_t)MLX90640_address);
  if (Wire.endTransmission() != 0)
    return (false); // Sensor did not ACK
  return (true);
}

void printTemps(float *temps) {
  for (int y = 0; y < 24; y++) {
    for (int x = 0; x < 32; x++ ) {
      Serial.print(temps[x + y * 32], 1);
      Serial.print(",");
    }
    Serial.println();
  }
}

void heatmapColor(byte *rgb, float t, float tmin, float tmax) {
  static int colorRanges = 4;
  static byte map[][3] = {
    {0, 0, 1},
    {0, 1, 1},
    {0, 1, 0},
    {1, 1, 0},
    {1, 0, 0},
  };
  float n = colorRanges * (t - tmin) / (tmax - tmin);

  int r = int(floor(n));
  float dr = (n - r);

  for (int i = 0; i < 3; i++) {
    rgb[i] = 255.0 * map[r + 1][i] * dr + 255.0 * map[r][i] * (1 - dr);
  }
}



void minmax(float *temps,
            float *tmin,
            float *tmax,
            float *realMin,
            float *realMax) {
  *tmin = 10.0;
  *tmax = 40.0;
  *realMin = 100.0;
  *realMax = -10.0;
  for (int i = 0 ; i < 32 * 24; i++ ) {
    if (*tmin > temps[i]) *tmin = temps[i];
    if (*tmax < temps[i]) *tmax = temps[i];
    if (*realMin > temps[i]) *realMin = temps[i];
    if (*realMax < temps[i]) *realMax = temps[i];
  }
}


/*Connections to NodeMCU

  LED to 3V3
  SCK to D5
  SDA to D7
  A0/DC to D4
  RST to D0
  CS to D8
  GND to GND
  VCC to 3V3
*/

#include <SPI.h>

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SD.h>

#define TFT_CS  D8  // Chip select line for TFT display
#define TFT_DC   D4  // Data/command line for TFT
#define TFT_RST  D0  // Reset line for TFT (or connect to +5V)

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

void setupTFT() {
  tft.initR(INITR_BLACKTAB);   // initialize a ST7735S chip, black tab
  tft.fillScreen(ST7735_BLACK);
  tft.setRotation(2);
}

void drawPixels(float *temps, float tmin, float tmax) {
  for (int y = 0; y < 24; y++) {
    for (int x = 0; x < 32; x++ ) {
      float t = temps[(31 - x) + y * 32];
      byte col[3];
      heatmapColor(col, t, tmin, tmax);
      tft.fillRect(x * 4, y * 4, 4, 4, tft.color565(col[0], col[1], col[2]));
    }
  }
}

#include <Fonts/FreeMonoBold9pt7b.h>

void drawTemps(float tmin, float tmax, float realMin, float realMax) {
  int ymax = 24 * 4;
  int ytext = 108;
  int steps = 3;

  tft.setCursor(0, 6 * 24);

  for (int i = 0 ; i < steps + 1; i++) {
    float t = realMin + (realMax - realMin) * i / (steps);
    int x = 100 / steps * i;

    byte col[3];
    tft.setTextSize(1);
    heatmapColor(col, t, tmin, tmax);
    tft.setTextColor(tft.color565(col[0], col[1], col[2]), ST77XX_BLACK);
    tft.setCursor(x, ytext);
    tft.print(String(t, 0) + "  ");
  }
}

void drawStats(const String &msg) {
  tft.setFont();
  tft.setCursor(0, 6 * 24);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.setTextSize(0);
  tft.println(msg);
}

void displayTemps(float *temps, int readMillis) {
  float tmin, tmax, realMin, realMax;
  minmax(temps, &tmin, &tmax, &realMin, &realMax);

  int start = millis();
  drawPixels(temps, tmin, tmax);
  drawTemps(tmin, tmax, realMin, realMax);
  String msg = "R" + String(readMillis, 10) + "ms D" + String(millis() - start, 10) + "ms  ";
  drawStats(msg);
}


/***** web serving ******/

void configWifi() {
  tft.fillScreen(ST7735_BLACK);
  wifiMessage("ESSID: " + String(apName));

  WiFiManager wifiManager;
  wifiManager.setTimeout(120);
  wifiManager.setDebugOutput(true);
  if (!wifiManager.autoConnect(apName)) {
    wifiMessage("failed to connect");
    delay(3000);
  }
}

void wifiMessage(String const &msg) {
  tft.setFont();
  tft.setCursor(0, 132);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.setTextSize(0);
  tft.println(msg);
}

void handleTemp() {
  // - xx . x ,
  // xxx  . x ,
  String out;
  out.reserve(768 * 6 + 10);
  out += "[";
  for (int y = 0; y < 24; y++) {
    out += "[";
    for (int x = 0; x < 32; x++ ) {
      float t = mlx90640To[(31 - x) + y * 32];

      out += String(t, 1);
      if (x < 31) {
        out += ",";
      }
    }
    out += "]";
    if (y < 24) {
      out += ",\n";
    }
  }
  out += "]\n";

  web->send(200, "application/json", out.c_str());
}

void setupWebserver() {
  web.reset(new ESP8266WebServer(80));
  // TODO - allow to set emissivity
  // TODO - allow to skip processing and dump raw sensor data?
  // TODO - dump MLX parameters?
  web->on("/temp", handleTemp);
  web->on("/", []() {
    web->send(200, "text/plain",
              "<html><body><p>endpoint: <tt>GET /temp</tt> dump latest reading</body></html>");
  });
  web->begin();
}
