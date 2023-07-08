// #include <M5Unified.h>
// // #include "Adafruit_VL53L0X.h"
// #include "BLEDevice.h"

// // 取得した距離センサの値をBluetoothで送信するプログラム

// Adafruit_VL53L0X lox = Adafruit_VL53L0X();

// #define T_PERIOD 1000 // Transmission period (ms)
// #define S_PERIOD 1000 // Sleep period (ms)

// BLEAdvertising *pAdvertising;
// uint8_t seq = 0; // Sequence number

// void setAdvData(BLEAdvertising *pAdvertising, uint16_t distance)
// {
//   BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
//   // ESP_BLE_ADV_FLAG_GEN_DISC: General discoverable mode
//   // https://qiita.com/Kosuke_Matsui/items/c46a7e0b2299fe2e611f
//   oAdvertisementData.setFlags(ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT);

//   std::string strServiceData = "";
//   strServiceData += (char)0x06; // Length
//   strServiceData += (char)0xFF;  // Type: Manufacturer Specific Data
//   strServiceData += (char)0xFF;  // Test Company ID 下位バイト
//   strServiceData += (char)0xFF;  // Test Company ID 上位バイト
//   strServiceData += (char)seq;
//   strServiceData += (char)(distance & 0xFF); // Distance 下位バイト
//   strServiceData += (char)((distance >> 8) & 0xFF); // Distance 上位バイト

//   oAdvertisementData.addData(strServiceData);
//   pAdvertising->setAdvertisementData(oAdvertisementData);
// }

// void setup()
// {
//   auto cfg = M5.config();
//   M5.begin(cfg);
//   USBSerial.begin(115200);
//   M5.Display.setTextSize(2);
//   Wire.setPins(SDA, SCL);
//   if (!lox.begin())
//   {
//     USBSerial.println("Failed to boot VL53L0X");
//     while (1)
//       ;
//   }
//   USBSerial.println("Start App\n\n");

//   BLEDevice::init("M5AtomS3 Distance Sensor");
//   BLEServer *pServer = BLEDevice::createServer();
//   pAdvertising = pServer->getAdvertising();
//   lox.startRangeContinuous();
// }

// void loop()
// {
//   // Transmission
//   M5.Display.startWrite();
//   auto distance = lox.readRange();
//   M5.Display.clear(BLACK);
//   M5.Display.setCursor(0, 20);
//   M5.Display.printf("Distance\nmm: %d\n", distance);
//   M5.Display.endWrite();
//   USBSerial.println("Distance in mm: " + String(distance));
//   setAdvData(pAdvertising, distance);
//   pAdvertising->start();
//   delay(T_PERIOD);
//   pAdvertising->stop();
//   delay(S_PERIOD);
//   seq++;
// }

// 参考
// https://github.com/espressif/arduino-esp32/blob/master/libraries/BLE/examples/BLE_server/BLE_server.ino

#include <M5Unified.h>
#include "BLEDevice.h"
#include "BLEServer.h" // BLEServerCallbacksを使うために必要

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID "068c47b7-fc04-4d47-975a-7952be1a576f"
#define CHARACTERISTIC_UUID "e3737b3f-a08d-405b-b32d-35a8f6c64c5d"

static int8_t state = 0;

#define STATE_IDLE 0
#define STATE_CONNECTING 1
#define STATE_CONNECTED 2
#define STATE_DISCONNECTED 3

class MyServerCallbacks : public BLEServerCallbacks
{
  void onConnect(BLEServer *pServer)
  {
    state = STATE_CONNECTED;
    USBSerial.println("Connected");
  };

  void onDisconnect(BLEServer *pServer)
  {
    state = STATE_DISCONNECTED;
    USBSerial.println("Disconnected");
  }
};

void setup()
{
  auto cfg = M5.config();
  M5.begin(cfg);
  USBSerial.begin(115200);
  M5.Display.setTextSize(2);

  BLEDevice::init("M5AtomS3 Test BLE Server");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Serviceはキャラクタリスティックの入れ物
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Characteristicはセンサーの値や状態を表す
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ |
          BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic->setValue("Hello World");
  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true); // これをtrueにしないと、Advertising DataにService UUIDが含まれない。
  // minIntervalはデフォルトの20でとくに問題なさそうなため、setMinPreferredは省略
  BLEDevice::startAdvertising();
  USBSerial.println("Waiting a connection");
  state = STATE_IDLE;
}

void drawIdle()
{
  M5.Display.startWrite();
  M5.Display.clear(BLACK);
  M5.Display.setCursor(0, 20);
  M5.Display.printf("Waiting a connection");
  M5.Display.endWrite();
}

void drawConnected()
{
  M5.Display.startWrite();
  M5.Display.clear(BLACK);
  M5.Display.setCursor(0, 20);
  M5.Display.printf("Connected");
  M5.Display.endWrite();
}

void loop()
{
  delay(1000);
  switch (state)
  {
  case STATE_CONNECTED:
    drawConnected();
    break;
  default:
    drawIdle();
    break;
  }
}