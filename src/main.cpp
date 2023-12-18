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
  lox.startRangeContinuous(200);
}

void loop()
{

  if (lox.isRangeComplete())
  {
    M5.Display.startWrite();
    auto distance = lox.readRange();
    M5.Display.clear(BLACK);
    M5.Display.setCursor(0, 20);
    M5.Display.printf("Distance\nmm: %d\n", distance);
    M5.Display.endWrite();
    USBSerial.println("Distance in mm: " + String(distance));
    // 時間を出力
    auto time_data = millis();
    auto seconds = time_data / 1000;
    auto milliseconds = time_data % 1000;
    USBSerial.println("Time: " + String(seconds) + "." + String(milliseconds));
  } else {
    // USBSerial.println("Range not complete");
  }

  // delay(200);
}
