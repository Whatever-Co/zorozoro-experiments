#include <Arduino.h>
#include <NimBLEDevice.h>

#define MAX_CUBES 8

class Cube {
   public:
    static NimBLEUUID serviceUUID;
    static NimBLEUUID lampCharUUID;
    static NimBLEUUID batteryCharUUID;
    static NimBLEUUID buttonCharUUID;

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
    NimBLERemoteCharacteristic *lamp, *battery, *button;
};
