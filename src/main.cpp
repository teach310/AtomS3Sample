
// 参考
// https://github.com/espressif/arduino-esp32/blob/master/libraries/BLE/examples/BLE_server/BLE_server.ino

#include <M5Unified.h>
#include <BLEDevice.h>
#include <BLE2902.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID "068c47b7-fc04-4d47-975a-7952be1a576f"
#define CHARACTERISTIC_UUID "e3737b3f-a08d-405b-b32d-35a8f6c64c5d"

void startService(BLEServer *pServer)
{
  BLEService *pService = pServer->createService(SERVICE_UUID);

  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ |
          BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic->addDescriptor(new BLE2902()); // Descriptorを定義しておかないとClient側でエラーログが出力される
  pCharacteristic->setValue("Hello World");

  pService->start();
}

void startAdvertising()
{
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true); // trueにしないと、Advertising DataにService UUIDが含まれない。
  // minIntervalはデフォルトの20でとくに問題なさそうなため、setMinPreferredは省略
  BLEDevice::startAdvertising();
}

void setup()
{
  auto cfg = M5.config();
  M5.begin(cfg);
  USBSerial.begin(115200);
  M5.Display.setTextSize(2);

  BLEDevice::init("M5AtomS3 BLE Server");
  BLEServer *pServer = BLEDevice::createServer();
  startService(pServer);
  startAdvertising();

  M5.Display.println("Advertising!");
}

void loop()
{
}