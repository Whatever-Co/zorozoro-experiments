#pragma once

#include <bluefruit.h>

class App {
   public:
    static void Setup();
    static void Loop();

    static void OnIdInfo(BLEClientCharacteristic *chr, uint8_t *data, uint16_t length);
    static void OnMotor(BLEClientCharacteristic *chr, uint8_t *data, uint16_t length);
    static void OnBatteryInfo(BLEClientCharacteristic *chr, uint8_t *data, uint16_t length);

   private:
    static void OnMessage(MQTTClient *client, char topic[], char payload[], int length);
    static void OnConnect(uint16_t conn_handle);
    static void OnDisconnect(uint16_t conn_handle, uint8_t reason);

    static void StartAcceptNewCube();
    static void StopAcceptNewCube();

    static bool ConnectToCube(String address);

    static void SubscribeTopics(String address);
    static void UnsubscribeTopics(String address);

    static String ip_address_;
    static bool accept_new_cube_;
};
