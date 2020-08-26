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

    NimBLEClient *getClient();
    String getAddress();

   private:
    NimBLEClient *client;
    NimBLEService *service;
    NimBLERemoteCharacteristic *lamp, *battery, *button;
};
