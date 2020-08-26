
#include "cube.h"

NimBLEUUID Cube::serviceUUID("10B20100-5B3B-4571-9508-CF3EFCD7BBAE");
NimBLEUUID Cube::lampCharUUID("10B20103-5B3B-4571-9508-CF3EFCD7BBAE");
NimBLEUUID Cube::batteryCharUUID("10B20108-5B3B-4571-9508-CF3EFCD7BBAE");

Cube::Cube()
    : client(nullptr),
      service(nullptr),
      lamp(nullptr),
      battery(nullptr) {}

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
    auto service = client->getService(serviceUUID);
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

    GET_CHARACTERISTIC(lamp);
    GET_CHARACTERISTIC(battery);
    battery->subscribe(true, notifyCallback);

    // uint8_t data[] = {0x04, 0x01, 0x04,
    //                   0x10, 0x01, 0x01, 0xff, 0xff, 0x00,
    //                   0x10, 0x01, 0x01, 0x00, 0xff, 0x00,
    //                   0x10, 0x01, 0x01, 0x00, 0xff, 0xff,
    //                   0x10, 0x01, 0x01, 0xff, 0x00, 0xff};
    // lamp->writeValue(data, sizeof(data), true);
    return true;
}

void Cube::disconnect() {
    if (client != nullptr) {
        client->disconnect();
        NimBLEDevice::deleteClient(client);
        client = nullptr;
        service = nullptr;
        lamp = nullptr;
        battery = nullptr;
    }
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
