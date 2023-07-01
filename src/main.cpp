#include <M5Unified.h>
#include "Adafruit_VL53L0X.h"

Adafruit_VL53L0X lox = Adafruit_VL53L0X();

void setup()
{
  auto cfg = M5.config();
  M5.begin(cfg);
  USBSerial.begin(115200);
  M5.Display.setTextSize(2);
  Wire.setPins(SDA, SCL);
  if (!lox.begin())
  {
    USBSerial.println("Failed to boot VL53L0X");
    while (1)
      ;
  }
  USBSerial.println("Start App\n\n");
  lox.startRangeContinuous();
}

void loop()
{
  M5.Display.startWrite();
  auto distance = lox.readRange();
  M5.Display.clear(BLACK);
  M5.Display.setCursor(0, 20);
  M5.Display.printf("Distance\nmm: %d\n", distance);
  M5.Display.endWrite();
  USBSerial.println("Distance in mm: " + String(distance));
  delay(200);
}
