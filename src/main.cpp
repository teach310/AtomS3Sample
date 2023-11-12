#include <M5Unified.h>
#include "BLEDevice.h"
#include "BLEServer.h"
#include <BLE2902.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID "068C47B7-FC04-4D47-975A-7952BE1A576F"
#define CHARACTERISTIC_UUID "E91E5ECB-A460-4DB1-97F7-F13D52222E15"

static int8_t state = 0;

#define STATE_ADVERTISING 0
#define STATE_CONNECTING 1
#define STATE_CONNECTED 2
#define STATE_DISCONNECTED 3

static bool redraw = false;

BLECharacteristic *pButtonInformationCharacteristic;

class MyServerCallbacks : public BLEServerCallbacks
{
  void onConnect(BLEServer *pServer)
  {
    state = STATE_CONNECTED;
    BLEDevice::stopAdvertising();
    redraw = true;
  };

  void onDisconnect(BLEServer *pServer)
  {
    state = STATE_DISCONNECTED;
    redraw = true;
  }
};

void startServer()
{
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);
  pButtonInformationCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ |
          BLECharacteristic::PROPERTY_NOTIFY);
  pButtonInformationCharacteristic->addDescriptor(new BLE2902());

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  BLEDevice::startAdvertising();
}

void setup()
{
  auto cfg = M5.config();
  M5.begin(cfg);
  USBSerial.begin(115200);
  M5.Display.setTextSize(2);

  BLEDevice::init("M5AtomS3");
  startServer();
  state = STATE_ADVERTISING;
  redraw = true;
}

std::string getButtonInformation(int buttonId, bool isPressed)
{
  std::string buff = "";
  buff += (char)buttonId;
  buff += (char)(isPressed ? 0x80 : 0x00);
  return buff;
}

void getButtonInformation(uint8_t buff[2], int buttonId, bool isPressed)
{
  buff[0] = buttonId;
  buff[1] = (isPressed ? 0x80 : 0x00);
}

void onPressBtnA()
{
  if (state != STATE_CONNECTED)
  {
    return;
  }

  uint8_t buff[2];
  getButtonInformation(buff, 1, true);
  pButtonInformationCharacteristic->setValue(buff, sizeof(buff));
  pButtonInformationCharacteristic->notify();
}

void onReleaseBtnA()
{
  if (state != STATE_CONNECTED)
  {
    return;
  }

  std::string buff = getButtonInformation(1, false);
  pButtonInformationCharacteristic->setValue(buff);
  pButtonInformationCharacteristic->notify();
}

void handleButtonEvent()
{
  if (M5.BtnA.wasPressed())
  {
    onPressBtnA();
  }

  if (M5.BtnA.wasReleased())
  {
    onReleaseBtnA();
  }
}

void drawAdvertising()
{
  M5.Display.startWrite();
  M5.Display.clear(BLACK);
  M5.Display.setCursor(0, 20);
  M5.Display.printf("Advertising");
  M5.Display.endWrite();
}

void drawConnected()
{
  M5.Display.startWrite();
  M5.Display.clear(BLACK);
  M5.Display.setCursor(0, 20);
  M5.Display.println("Connected");
  M5.Display.endWrite();
}

void drawDisplay()
{
  switch (state)
  {
  case STATE_ADVERTISING:
    drawAdvertising();
    break;
  case STATE_CONNECTED:
    drawConnected();
    break;
  }
}

void loop()
{
  M5.update();
  handleButtonEvent();

  if (state == STATE_DISCONNECTED)
  {
    state = STATE_ADVERTISING;
    BLEDevice::startAdvertising();
    redraw = true;
  }

  if (!redraw)
    return;
  redraw = false;
  drawDisplay();
}
