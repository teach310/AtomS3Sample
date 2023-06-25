#include <M5Unified.h>

uint32_t count;

void setup()
{

  auto cfg = M5.config();
  M5.begin(cfg);
  USBSerial.begin(115200);
  M5.Display.setTextSize(3);
  USBSerial.println("Hello World!!");
  count = 0;
}

void loop()
{

  M5.Display.setCursor(0, 20);
  M5.Display.printf("COUNT: %d\n", count);
  USBSerial.printf("COUNT: %d\n", count);
  count++;
  delay(1000);
}
