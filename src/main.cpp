
// 参考
// https://github.com/espressif/arduino-esp32/blob/master/libraries/BLE/examples/BLE_server/BLE_server.ino

#include <M5Unified.h>
#include <BLEDevice.h>
#include <BLE2902.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID "068c47b7-fc04-4d47-975a-7952be1a576f"
#define CHARACTERISTIC_UUID "e3737b3f-a08d-405b-b32d-35a8f6c64c5d"
#define NOTIFY_CHARACTERISTIC_UUID "c9da2ce8-d119-40d5-90f7-ef24627e8193"

static String text = "";
static bool redraw = false;
static bool connected = true;

BLECharacteristic *pNotifyCharacteristic;

class MyServerCallbacks : public BLEServerCallbacks
{
  void onConnect(BLEServer *pServer)
  {
    USBSerial.println("onConnect");
    text = "Connected!";
    redraw = true;
    connected = true;
  };

  void onDisconnect(BLEServer *pServer)
  {
    USBSerial.println("onDisconnect");
    text = "Disconnected!";
    redraw = true;
    connected = false;
  }
};

class MyCharacteristicCallbacks : public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *pCharacteristic)
  {
    std::string value = pCharacteristic->getValue();

    if (value.length() > 0)
    {
      USBSerial.print("onWrite: ");
      USBSerial.println(value.c_str());
      text = value.c_str();
      redraw = true;
    }
  }
};

void startService(BLEServer *pServer)
{
  BLEService *pService = pServer->createService(SERVICE_UUID);

  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ |
          BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic->addDescriptor(new BLE2902()); // Descriptorを定義しておかないとClient側でエラーログが出力される
  pCharacteristic->setValue("Hello World");
  pCharacteristic->setCallbacks(new MyCharacteristicCallbacks());

  // 権限を最小にするためにNotify用のCharacteristicはReadWrite用とは別に定義
  pNotifyCharacteristic = pService->createCharacteristic(
      NOTIFY_CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_NOTIFY);
  pNotifyCharacteristic->addDescriptor(new BLE2902());

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

void onClickBtnA()
{
  if (!connected)
  {
    return;
  }

  String value = "Notify Data " + String(random(100, 999));
  pNotifyCharacteristic->setValue(value.c_str());
  pNotifyCharacteristic->notify();
}

void setup()
{
  auto cfg = M5.config();
  M5.begin(cfg);
  USBSerial.begin(115200);
  M5.Display.setTextSize(2);

  BLEDevice::init("M5AtomS3 BLE Server");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  startService(pServer);
  startAdvertising();

  text = "Advertising!";
  redraw = true;
}

void loop()
{
  M5.update();
  if (M5.BtnA.wasClicked())
  {
    onClickBtnA();
  }

  if (!redraw)
  {
    return;
  }
  redraw = false;

  M5.Display.clear();
  M5.Display.setCursor(0, 0);
  M5.Display.println(text);
}