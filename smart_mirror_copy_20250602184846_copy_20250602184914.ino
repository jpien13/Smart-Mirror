#include <Adafruit_Protomatter.h>

uint8_t rgbPins[]  = {7, 8, 9, 10, 11, 12};
uint8_t addrPins[] = {17, 18, 19, 20, 21};
uint8_t clockPin   = 14;
uint8_t latchPin   = 15;
uint8_t oePin      = 16;

Adafruit_Protomatter matrix(
  64, 4, 1, rgbPins, 4, addrPins, clockPin, latchPin, oePin, false);

void setup(void) {
  Serial.begin(9600);

  // Initialize matrix...
  ProtomatterStatus status = matrix.begin();
  Serial.print("Protomatter begin() status: ");
  Serial.println((int)status);
  if(status != PROTOMATTER_OK) {
    for(;;);
  }

  // Make four color bars (red, green, blue, white) with brightness ramp:
  for(int x=0; x<matrix.width(); x++) {
    uint8_t level = x * 256 / matrix.width(); // 0-255 brightness
    matrix.drawPixel(x, matrix.height() - 4, matrix.color565(level, 0, 0));
    matrix.drawPixel(x, matrix.height() - 3, matrix.color565(0, level, 0));
    matrix.drawPixel(x, matrix.height() - 2, matrix.color565(0, 0, level));
    matrix.drawPixel(x, matrix.height() - 1, matrix.color565(level, level, level));
  }

  // Simple shapes and text, showing GFX library calls:
  matrix.drawCircle(12, 10, 9, matrix.color565(255, 0, 0));               // Red
  matrix.drawRect(14, 6, 17, 17, matrix.color565(0, 255, 0));             // Green
  matrix.drawTriangle(32, 9, 41, 27, 23, 27, matrix.color565(0, 0, 255)); // Blue
  matrix.println("ADAFRUIT"); // Default text color is white

  // AFTER DRAWING, A show() CALL IS REQUIRED TO UPDATE THE MATRIX!

  matrix.show(); // Copy data to matrix buffers
}

void loop(void) {
  Serial.print("Refresh FPS = ~");
  Serial.println(matrix.getFrameCount());
  delay(1000);
}