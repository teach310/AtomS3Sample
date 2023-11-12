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
#include "BLEServer.h"
#include <BLE2902.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID "068C47B7-FC04-4D47-975A-7952BE1A576F"
#define CHARACTERISTIC_UUID "E91E5ECB-A460-4DB1-97F7-F13D52222E15"
#define NOTIFY_CHARACTERISTIC_UUID "c9da2ce8-d119-40d5-90f7-ef24627e8193"

// static int8_t state = 0;

// #define STATE_IDLE 0
// #define STATE_CONNECTING 1
// #define STATE_CONNECTED 2
// #define STATE_DISCONNECTED 3

// static String text = "";
// static bool redraw = false;

// BLECharacteristic *pNotifyCharacteristic;

class MyServerCallbacks : public BLEServerCallbacks
{
  void onConnect(BLEServer *pServer)
  {
    // state = STATE_CONNECTED;
    USBSerial.println("Connected");
    // redraw = true;
  };

  void onDisconnect(BLEServer *pServer)
  {
    // state = STATE_DISCONNECTED;
    USBSerial.println("Disconnected");
    // redraw = true;
  }
};

// class MyCharacteristicCallbacks : public BLECharacteristicCallbacks
// {
//   void onWrite(BLECharacteristic *pCharacteristic)
//   {
//     std::string value = pCharacteristic->getValue();

//     if (value.length() > 0)
//     {
//       USBSerial.println("*********");
//       USBSerial.print("New value: ");

//       for (int i = 0; i < value.length(); i++)
//         USBSerial.print(value[i]);

//       USBSerial.println();
//       USBSerial.println("*********");

//       // text = String(value.c_str());
//       // redraw = true;
//     }
//   }
// };

void setup()
{
  auto cfg = M5.config();
  M5.begin(cfg);
  USBSerial.begin(115200);
  M5.Display.setTextSize(2);

  BLEDevice::init("M5AtomS3");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ |
          BLECharacteristic::PROPERTY_NOTIFY);
  pCharacteristic->addDescriptor(new BLE2902());
  // pCharacteristic->setValue("Hello World");
  // pCharacteristic->setCallbacks(new MyCallbacks());

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  BLEDevice::startAdvertising();
  USBSerial.println("Waiting a connection");
  // state = STATE_IDLE;
  // redraw = true;
}

void onClickBtnA()
{
  USBSerial.println("onClickBtnA");
  // if (state == STATE_CONNECTED)
  // {
  //   // ランダムな三桁の数字を送信
  //   pNotifyCharacteristic->setValue(String(random(100, 999)).c_str());
  //   pNotifyCharacteristic->notify();
  // }
}

// void drawIdle()
// {
//   M5.Display.startWrite();
//   M5.Display.clear(BLACK);
//   M5.Display.setCursor(0, 20);
//   M5.Display.printf("Waiting a connection");
//   M5.Display.endWrite();
// }

// void drawConnected()
// {
//   M5.Display.startWrite();
//   M5.Display.clear(BLACK);
//   M5.Display.setCursor(0, 20);
//   M5.Display.println("Connected");
//   M5.Display.println(text);
//   M5.Display.endWrite();
// }

void loop()
{
  M5.update();
  if (M5.BtnA.wasClicked())
  {
    onClickBtnA();
  }

  // if (!redraw)
  //   return;
  // redraw = false;
  // switch (state)
  // {
  // case STATE_CONNECTED:
  //   drawConnected();
  //   break;
  // default:
  //   drawIdle();
  //   break;
  // }
}