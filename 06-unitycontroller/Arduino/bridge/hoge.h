#pragma once

// #include <Ethernet2.h>
// #include <PubSubClient.h>
// #include <SPI.h>
#include <bluefruit.h>

// #include "cube.h"
// #include "cube_manager.h"

//----------------------------------------

class App {
   public:
    static void Setup();
    static void Loop();

    static void OnBatteryInfo(BLEClientCharacteristic *chr, uint8_t *data, uint16_t length);

   private:
    static void OnMessage(char *topic, byte *payload, unsigned int length);
    static void OnConnect(uint16_t conn_handle);
    static void OnDisconnect(uint16_t conn_handle, uint8_t reason);

    static void StartAcceptNewCube();
    static void StopAcceptNewCube();

    static bool ConnectToCube(String address);

    static void SubscribeTopics(String address);
    static void UnsubscribeTopics(String address);

    static bool accept_new_cube_;
};

bool App::accept_new_cube_ = false;
