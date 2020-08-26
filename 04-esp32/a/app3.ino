#include <Arduino.h>

#include "NimBLEDevice.h"

static NimBLEUUID serviceUUID("10B20100-5B3B-4571-9508-CF3EFCD7BBAE");
static NimBLEUUID charUUID("10B20103-5B3B-4571-9508-CF3EFCD7BBAE");

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = true;
static BLEAdvertisedDevice *foundDevice = nullptr;

bool connectToServer(BLEAdvertisedDevice *device);

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
    void onResult(NimBLEAdvertisedDevice *advertisedDevice) {
        Serial.print("BLE Advertised Device found: ");
        Serial.println(advertisedDevice->toString().c_str());

        if (advertisedDevice->isAdvertisingService(serviceUUID)) {
            BLEDevice::getScan()->stop();
            foundDevice = advertisedDevice;
            doConnect = true;
            doScan = true;
        }
    }
};

class MyClientCallback : public BLEClientCallbacks {
    void onConnect(BLEClient *pclient) {
        Serial.println("connected");
    }

    void onDisconnect(BLEClient *pclient) {
        connected = false;
        Serial.println("onDisconnect");
    }
};

void setup() {
    Serial.begin(115200);
    Serial.println("Starting Arduino BLE Client application.....");
    // pinMode(26, ANALOG);
    // Serial.println(String(analogRead(26)));
    BLEDevice::init("");

    BLEScan *scan = BLEDevice::getScan();
    scan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    // scan->setInterval(1349);
    // scan->setWindow(449);
    scan->setActiveScan(true);
    scan->start(3, false);
}

void loop() {
    if (foundDevice) {
        connectToServer(foundDevice);
        foundDevice = nullptr;
    }
    // if (doConnect == true) {
    //     if (connectToServer()) {
    //         Serial.println("We are now connected to the BLE Server.");
    //     } else {
    //         Serial.println("We have failed to connect to the server; there is nothin more we will do.");
    //     }
    //     doConnect = false;
    // }

    BLEDevice::getScan()->start(0);

    delay(100);

    // Serial.println(String(millis() / 1000));
}

bool connectToServer(BLEAdvertisedDevice *device) {
    Serial.print("Forming a connection to ");
    // Serial.println(myDevice->getAddress().toString().c_str());

    BLEClient *pClient = BLEDevice::createClient();
    Serial.println(" - Created client");

    pClient->setClientCallbacks(new MyClientCallback());

    pClient->connect(device);
    Serial.println(" - Connected to server");

    BLERemoteService *pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
        Serial.print("Failed to find our service UUID: ");
        Serial.println(serviceUUID.toString().c_str());
        pClient->disconnect();
        return false;
    }
    Serial.println(" - Found our service");

    BLERemoteCharacteristic *pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic == nullptr) {
        Serial.print("Failed to find our characteristic UUID: ");
        Serial.println(charUUID.toString().c_str());
        pClient->disconnect();
        return false;
    }
    Serial.println(" - Found our characteristic");

    uint8_t data[] = {0x04, 0x00, 0x04,
                      0x10, 0x01, 0x01, 0xff, 0xff, 0x00,
                      0x10, 0x01, 0x01, 0x00, 0xff, 0x00,
                      0x10, 0x01, 0x01, 0x00, 0xff, 0xff,
                      0x10, 0x01, 0x01, 0xff, 0x00, 0xff};
    Serial.printf("data len = %d\n", sizeof(data));

    pRemoteCharacteristic->writeValue(data, sizeof(data), true);

    connected = true;
    return true;
}
