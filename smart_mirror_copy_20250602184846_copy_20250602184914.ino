#include <Adafruit_Protomatter.h>
#include <SPI.h>
#include <WiFiNINA.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

uint8_t rgbPins[]  = {7, 8, 9, 10, 11, 12};
uint8_t addrPins[] = {17, 18, 19, 20, 21};
uint8_t clockPin   = 14;
uint8_t latchPin   = 15;
uint8_t oePin      = 16;

Adafruit_Protomatter matrix(
  64, 4, 1, rgbPins, 4, addrPins, clockPin, latchPin, oePin, false);

const char* ssid     = "wifi";
const char* password = "password";

WiFiUDP   ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -14400, 60000);

void setup(void) {
  Serial.begin(115200);
  while (!Serial) delay(10);
  Serial.println(F("MatrixPortal M4 + NTP Clock (WiFiNINA)"));

  ProtomatterStatus status = matrix.begin();
  Serial.print(F("Protomatter begin() status: "));
  Serial.println((int)status);
  if (status != PROTOMATTER_OK) {
    Serial.println(F("Matrix init failed!"));
    while (1) delay(10);
  }
  matrix.setRotation(45);
  matrix.fillScreen(0x000000);
  matrix.setCursor(0, 8);
  matrix.print("CONNECTING...");
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
}

void loop(void) {
  if (timeClient.update()) {
    int rawHour = timeClient.getHours();    // 0–23
    int minute  = timeClient.getMinutes();  // 0–59

    int hour12 = rawHour;
    if (hour12 >= 12) {
      if (hour12 > 12) hour12 -= 12;
    }
    if (hour12 == 0) hour12 = 12;

    char buf[6]; // “h:mm” up to “12:59” + '\0'
    sprintf(buf, "%2d:%02d", hour12, minute);

    matrix.fillScreen(0x000000);
    matrix.setCursor(0, 4);
    matrix.print(buf);
    matrix.show();

    Serial.println(buf);
  }
  delay(200);
}
