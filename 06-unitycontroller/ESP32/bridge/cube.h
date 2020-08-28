#include <Arduino.h>
#include <NimBLEDevice.h>

#define MAX_CUBES 8

class Cube {
   public:
    static NimBLEUUID serviceUUID;
    static NimBLEUUID idInfoCharUUID;
    static NimBLEUUID motorControlCharUUID;
    static NimBLEUUID lampControlCharUUID;
    static NimBLEUUID sensorInfoCharUUID;
    static NimBLEUUID buttonInfoCharUUID;
    static NimBLEUUID batteryInfoCharUUID;

    Cube();
    ~Cube();

    bool connect(String address, NimBLEClientCallbacks *clientCallbacks, notify_callback notifyCallback);
    void disconnect();

    void SetLamp(uint8_t *data, size_t length);

    NimBLEClient *getClient();
    String getAddress();

   private:
    NimBLEClient *client;
    NimBLERemoteService *service;
    NimBLERemoteCharacteristic *idInfo;
    NimBLERemoteCharacteristic *motorControl;
    NimBLERemoteCharacteristic *lampControl;
    NimBLERemoteCharacteristic *sersorInfo;
    NimBLERemoteCharacteristic *buttonInfo;
    NimBLERemoteCharacteristic *batteryInfo;
};
