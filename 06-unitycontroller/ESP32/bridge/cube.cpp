
#include "cube.h"

NimBLEUUID Cube::serviceUUID("10B20100-5B3B-4571-9508-CF3EFCD7BBAE");
NimBLEUUID Cube::idInfoCharUUID("10B20101-5B3B-4571-9508-CF3EFCD7BBAE");
NimBLEUUID Cube::motorControlCharUUID("10B20102-5B3B-4571-9508-CF3EFCD7BBAE");
NimBLEUUID Cube::lampControlCharUUID("10B20103-5B3B-4571-9508-CF3EFCD7BBAE");
NimBLEUUID Cube::sensorInfoCharUUID("10B20106-5B3B-4571-9508-CF3EFCD7BBAE");
NimBLEUUID Cube::buttonInfoCharUUID("10B20107-5B3B-4571-9508-CF3EFCD7BBAE");
NimBLEUUID Cube::batteryInfoCharUUID("10B20108-5B3B-4571-9508-CF3EFCD7BBAE");

Cube::Cube()
    : client(nullptr),
      service(nullptr),
      idInfo(nullptr),
      motorControl(nullptr),
      lampControl(nullptr),
      sersorInfo(nullptr),
      buttonInfo(nullptr),
      batteryInfo(nullptr) {}

Cube::~Cube() {
    disconnect();
}

bool Cube::connect(String address, NimBLEClientCallbacks* clientCallbacks, notify_callback notifyCallback) {
    Serial.print("Forming a connection to ");
    Serial.println(address);
    client = NimBLEDevice::createClient();
    client->setClientCallbacks(clientCallbacks, false);
    Serial.println(" - Created client");
    client->connect(NimBLEAddress(address.c_str(), BLE_ADDR_RANDOM));
    Serial.println(" - Connected");
    service = client->getService(serviceUUID);
    if (service == nullptr) {
        Serial.print("Failed to find our service UUID: ");
        Serial.println(serviceUUID.toString().c_str());
        disconnect();
        return false;
    }
    Serial.println(" - Found our service");

#define GET_CHARACTERISTIC(x)                                     \
    x = service->getCharacteristic(x##CharUUID);                  \
    if (x == nullptr) {                                           \
        Serial.print("Failed to find our characteristic UUID: "); \
        Serial.println(x##CharUUID.toString().c_str());           \
        disconnect();                                             \
        return false;                                             \
    }                                                             \
    Serial.println(" - Found our characteristic " #x);

    GET_CHARACTERISTIC(idInfo);
    GET_CHARACTERISTIC(motorControl);
    GET_CHARACTERISTIC(lampControl);
    GET_CHARACTERISTIC(buttonInfo);
    GET_CHARACTERISTIC(batteryInfo);

    idInfo->subscribe(true, notifyCallback);
    buttonInfo->subscribe(true, notifyCallback);
    batteryInfo->subscribe(true, notifyCallback);
    return true;
}

void Cube::disconnect() {
    lampControl = nullptr;
    if (batteryInfo) {
        batteryInfo->unsubscribe();
    }
    batteryInfo = nullptr;
    if (buttonInfo) {
        buttonInfo->unsubscribe();
    }
    buttonInfo = nullptr;
    service = nullptr;
    if (client != nullptr) {
        client->disconnect();
        NimBLEDevice::deleteClient(client);
        client = nullptr;
    }
}

void Cube::SetMotor(uint8_t* data, size_t length) {
    motorControl->writeValue(data, length, false);
}

void Cube::SetLamp(uint8_t* data, size_t length) {
    lampControl->writeValue(data, length, true);
}

NimBLEClient* Cube::getClient() {
    return client;
}

String Cube::getAddress() {
    if (client == nullptr) {
        return String("");
    }
    return String(client->getPeerAddress().toString().c_str());
}
