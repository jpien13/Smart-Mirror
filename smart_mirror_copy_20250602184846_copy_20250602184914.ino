#include <Adafruit_Protomatter.h>
#include <SPI.h>
#include <WiFiNINA.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <Arduino_JSON.h>
#include <Arduino.h>
#include <stdlib.h>

#define BLACK    0x0000
#define BLUE     0x001F
#define RED      0xF800
#define GREEN    0x07E0
#define CYAN     0x07FF
#define MAGENTA  0xF81F
#define YELLOW   0xFFE0 
#define WHITE    0xFFFF

String WeatherAPIKey = "";

const uint8_t cloud_bitmap[] PROGMEM = {
  0b00000000, 0b00000000, 0b00000000,
  0b00000000, 0b00000000, 0b00000000,
  0b00000000, 0b00000000, 0b00000000,
  0b00000111, 0b10000000, 0b00000000,
  0b00001000, 0b01000000, 0b00000000,
  0b00001000, 0b00100000, 0b00000000,
  0b00010000, 0b00010000, 0b00000000,
  0b00010000, 0b00011110, 0b00000000,
  0b01100000, 0b00100001, 0b00000000,
  0b10000000, 0b00000000, 0b10000000,
  0b10000000, 0b00000000, 0b10000000,
  0b10000000, 0b00000000, 0b10000000,
  0b10000000, 0b00000000, 0b10000000,
  0b01000000, 0b00000001, 0b00000000,
  0b00111111, 0b11111111, 0b00000000,
  0b00000000, 0b00000000, 0b00000000,
  0b00000000, 0b00000000, 0b00000000,
  0b00000000, 0b00000000, 0b00000000
};


unsigned long lastWeatherUpdate = 0;
const unsigned long weatherInterval = 30UL * 60UL * 1000UL; // 30 minutes in ms

uint8_t rgbPins[]  = {7, 8, 9, 10, 11, 12};
uint8_t addrPins[] = {17, 18, 19, 20, 21};
uint8_t clockPin   = 14;
uint8_t latchPin   = 15;
uint8_t oePin      = 16;

Adafruit_Protomatter matrix(
  64, 4, 1,    // 64×32 panel, 4 chained
  rgbPins, 4,  // R1,G1,B1,R2,G2,B2
  addrPins,    // A,B,C,D
  clockPin, latchPin, oePin,
  false        // no double‐buffering
);

String currentCondition = "None";
float currentTemp = 0.0;

const char* ssid     = "";
const char* password = "";

WiFiUDP   ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -14400, 60000);

unsigned long initialEpoch  = 0;
unsigned long initialMillis = 0;



void drawPixel(uint16_t x, uint16_t y, uint16_t color);
void drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
void drawFastVLine(uint16_t x0, uint16_t y0, uint16_t length, uint16_t color);
void drawFastHLine(uint8_t x0, uint8_t y0, uint8_t length, uint16_t color);
void drawRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t color);
void fillRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t color);
void drawCircle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color);
void fillCircle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color);
void drawRoundRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t radius, uint16_t color);
void fillRoundRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t radius, uint16_t color); 
void drawTriangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void fillTriangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void drawChar(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bg, uint8_t size);
void setCursor(int16_t x0, int16_t y0);
void setTextColor(uint16_t color);
void setTextColor(uint16_t color, uint16_t backgroundcolor);
void setTextSize(uint8_t size);
void setTextWrap(boolean w);
void drawBitmap(int16_t x, int16_t y, uint8_t *bitmap, int16_t w, int16_t h, uint16_t color);


void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  Serial.println(F("MatrixPortal + ESP32: NTP sync, then free‐running clock"));

  ProtomatterStatus status = matrix.begin();
  Serial.print(F("Protomatter begin() status: "));
  Serial.println((int)status);
  if (status != PROTOMATTER_OK) {
    Serial.println(F("Matrix init failed! Halting."));
    while (1) delay(10);
  }
  matrix.setRotation(45);

  matrix.fillScreen(0x000000);
  matrix.setCursor(0, 8);
  matrix.print(ssid);
  matrix.show();

  Serial.print(F("Connecting to Wi-Fi "));
  Serial.println(ssid);
  while (WiFi.begin(ssid, password) != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println();
  Serial.println(F("Wi-Fi connected!"));
  Serial.print(F("IP Address: "));
  Serial.println(WiFi.localIP());

  timeClient.begin();

  Serial.print(F("Waiting for NTP time"));
  unsigned long startAttempt = millis();
  while (true) {
    if (timeClient.forceUpdate()) {
      unsigned long nowEpoch = timeClient.getEpochTime();
      if (nowEpoch > 0) {
        initialEpoch  = nowEpoch;
        initialMillis = millis();
        Serial.print(F("Success! Epoch = "));
        Serial.println(initialEpoch);
        break;
      }
    }

    if (millis() - startAttempt > 1000) {
      Serial.print(F("."));
      startAttempt = millis();
    }
    delay(100); 
  }
  Serial.println();
  timeClient.end();
  matrix.fillScreen(0x000000);
  matrix.show();
}

void animation(String condition) {
  const uint8_t* bitmap = nullptr;

  if (condition == "Clouds") {
    bitmap = cloud_bitmap;
  }

  if (bitmap != nullptr) {
    matrix.drawBitmap(7, 15, bitmap, 18, 18, WHITE);
  }
}

WiFiClient client;

String getWeather() {
  const char* host = "api.openweathermap.org";
  const int port = 80;
  const char* pathBase = "/data/2.5/weather?lat=40.678581&lon=-74.439980&units=imperial&appid=";

  String url = String(pathBase) + WeatherAPIKey;

  Serial.println("Connecting to OpenWeatherMap…");
  Serial.println("URL: " + url);
  if (!client.connect(host, port)) {
    Serial.println("Connection failed."); 
    return "";
  }

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");

  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return "";
    }
  }

  while (client.available()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") break;
  }

  String payload = "";
  while (client.available()) {
    payload += client.readStringUntil('\n');
  }

  client.stop();
  Serial.println("Weather data:");
  Serial.println(payload);
  return payload;
}


void loop() {
  unsigned long elapsedSecs  = (millis() - initialMillis) / 1000;
  unsigned long nowEpoch     = initialEpoch + elapsedSecs;

  unsigned long secsInDay    = nowEpoch % 86400UL; // seconds since midnight
  int rawHour   = secsInDay / 3600;                
  int rawMinute = (secsInDay % 3600) / 60;         
  int rawSecond = secsInDay % 60;                 

  int dispHour = rawHour;
  if (dispHour == 0)       dispHour = 12;
  else if (dispHour > 12)  dispHour -= 12;

  char timeBuf[6];
  sprintf(timeBuf, "%2d:%02d", dispHour, rawMinute);

  matrix.fillScreen(0x000000);
  matrix.setCursor(0, 4);
  matrix.print(timeBuf);

  unsigned long currentMillis = millis();

  if (currentMillis - lastWeatherUpdate >= weatherInterval || lastWeatherUpdate == 0) {
    lastWeatherUpdate = currentMillis;

    String json = getWeather();
    JSONVar myObject = JSON.parse(json);

    if (JSON.typeof(myObject) == "undefined") {
      Serial.println("Parsing weather JSON failed!");
    } else {
      currentTemp = (double) myObject["main"]["temp"];
      String condition = (const char*) myObject["weather"][0]["main"];

      Serial.print("Temp: ");
      Serial.println(currentTemp);
      Serial.print("Condition: ");
      Serial.println(condition);

      currentCondition = condition;
    }
  }

  int tempInt = (int)currentTemp;
  matrix.setCursor(8, 40);
  matrix.print(String(tempInt) + "F");



  animation(currentCondition);
  matrix.show();
  delay(200);
}


