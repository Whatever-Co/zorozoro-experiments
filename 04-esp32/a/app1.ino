#include <Arduino.h>

#include "BLEDevice.h"

// The remote service we wish to connect to.
static BLEUUID serviceUUID("10B20100-5B3B-4571-9508-CF3EFCD7BBAE");
// The characteristic of the remote service we are interested in.
static BLEUUID charUUID("10B20103-5B3B-4571-9508-CF3EFCD7BBAE");

static boolean doConnect = true;
static boolean connected = false;
static BLERemoteCharacteristic *pRemoteCharacteristic;

bool connectToServer();

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient *pclient) { Serial.println("connected"); }

  void onDisconnect(BLEClient *pclient) {
    connected = false;
    Serial.println("onDisconnect");
  }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("");
}

void loop() {
  if (doConnect == true) {
    if (connectToServer()) {
      Serial.println("We are now connected to the BLE Server.");
    } else {
      Serial.println(
          "We have failed to connect to the server; there is nothin more we "
          "will do.");
    }
    doConnect = false;
  }
}

bool connectToServer() {
  Serial.print("Forming a connection to ");
  // Serial.println(myDevice->getAddress().toString().c_str());

  BLEClient *pClient = BLEDevice::createClient();
  Serial.println(" - Created client");

  pClient->setClientCallbacks(new MyClientCallback());

  // Connect to the remove BLE Server.
  // pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of
  // address, it will be recognized type of peer device address (public or
  // private)
  pClient->connect(BLEAddress("c4:91:dc:ee:0e:38"), BLE_ADDR_TYPE_RANDOM);
  Serial.println(" - Connected to server");

  // Obtain a reference to the service we are after in the remote BLE server.
  BLERemoteService *pRemoteService = pClient->getService(serviceUUID);
  if (pRemoteService == nullptr) {
    Serial.print("Failed to find our service UUID: ");
    Serial.println(serviceUUID.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found our service");

  // Obtain a reference to the characteristic in the service of the remote BLE
  // server.
  pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
  if (pRemoteCharacteristic == nullptr) {
    Serial.print("Failed to find our characteristic UUID: ");
    Serial.println(charUUID.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found our characteristic");

  uint8_t data[] = {0x04, 0x00, 0x03, 0x30, 0x01, 0x01, 0xff,
                    0xff, 0x00, 0x30, 0x01, 0x01, 0x00, 0xff,
                    0x00, 0x30, 0x01, 0x01, 0x00, 0xff, 0xff};
  Serial.printf("data len = %d\n", sizeof(data));

  pRemoteCharacteristic->writeValue(data, sizeof(data), true);

  // Read the value of the characteristic.
  // if (pRemoteCharacteristic->canRead())
  // {
  //   std::string value = pRemoteCharacteristic->readValue();
  //   Serial.print("The characteristic value was: ");
  //   Serial.println(value.c_str());
  // }

  // if (pRemoteCharacteristic->canNotify())
  //   pRemoteCharacteristic->registerForNotify(notifyCallback);

  connected = true;
  return true;
}
