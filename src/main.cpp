#include <M5Unified.h>
#include "Adafruit_VL53L0X.h"
#include "BLEDevice.h"

// 取得した距離センサの値をBluetoothで送信するプログラム

Adafruit_VL53L0X lox = Adafruit_VL53L0X();

#define T_PERIOD 1000 // Transmission period (ms)
#define S_PERIOD 1000 // Sleep period (ms)

BLEAdvertising *pAdvertising;
uint8_t seq = 0; // Sequence number

void setAdvData(BLEAdvertising *pAdvertising, uint16_t distance)
{
  BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
  // ESP_BLE_ADV_FLAG_GEN_DISC: General discoverable mode
  // https://qiita.com/Kosuke_Matsui/items/c46a7e0b2299fe2e611f
  oAdvertisementData.setFlags(ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT);
  
  std::string strServiceData = "";
  strServiceData += (char)0x06; // Length
  strServiceData += (char)0xFF;  // Type: Manufacturer Specific Data
  strServiceData += (char)0xFF;  // Test Company ID 下位バイト
  strServiceData += (char)0xFF;  // Test Company ID 上位バイト
  strServiceData += (char)seq;
  strServiceData += (char)(distance & 0xFF); // Distance 下位バイト
  strServiceData += (char)((distance >> 8) & 0xFF); // Distance 上位バイト

  oAdvertisementData.addData(strServiceData);
  pAdvertising->setAdvertisementData(oAdvertisementData);
}

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

  BLEDevice::init("M5AtomS3 Distance Sensor");
  BLEServer *pServer = BLEDevice::createServer();
  pAdvertising = pServer->getAdvertising();
  lox.startRangeContinuous();
}

void loop()
{
  // Transmission
  M5.Display.startWrite();
  auto distance = lox.readRange();
  M5.Display.clear(BLACK);
  M5.Display.setCursor(0, 20);
  M5.Display.printf("Distance\nmm: %d\n", distance);
  M5.Display.endWrite();
  USBSerial.println("Distance in mm: " + String(distance));
  setAdvData(pAdvertising, distance);
  pAdvertising->start();
  delay(T_PERIOD);
  pAdvertising->stop();
  delay(S_PERIOD);
  seq++;
}
