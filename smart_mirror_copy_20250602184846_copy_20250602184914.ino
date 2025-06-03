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

// Create the Protomatter instance for a 64×32 panel:
Adafruit_Protomatter matrix(
  64, 4, 1, rgbPins, 4, addrPins, clockPin, latchPin, oePin, false);

const char* ssid     = "SSID";
const char* password = "password";

WiFiUDP   ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -18000, 60000);

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
  matrix.fillScreen(0x000000);
  matrix.println("CONNECTING...");
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
  timeClient.update();

  // Get the epoch time (already adjusted by –18000 in the constructor)
  unsigned long epoch = timeClient.getEpochTime();
  int hh = (epoch  % 86400L) / 3600;   // hour 0–23
  int mm = (epoch % 3600L)  / 60;      // minute 0–59
  int ss =  epoch % 60;               // second 0–59

  // Build a “HH:MM:SS” string
  char buf[9];
  sprintf(buf, "%02d:%02d:%02d", hh, mm, ss);


  matrix.fillScreen(0x000000);    // clear 
  matrix.setCursor(0, 8);         // x=0, y=8 (8px-high font)
  matrix.print(buf);              // print “12:34:56”
  matrix.show();                  

  Serial.println(buf);

  delay(1000); 
}
