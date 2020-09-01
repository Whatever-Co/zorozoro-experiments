#pragma once

#include <Arduino.h>
#include <bluefruit.h>

class Cube {
   public:
    static uint8_t SERVICE_UUID[];
    static uint8_t LAMP_CHARACTERISTIC_UUID[];

    Cube()
        : conn_handle(BLE_CONN_HANDLE_INVALID) {}

    ~Cube() {
        disconnect();
    }

    bool setup(uint8_t conn_handle) {
        this->conn_handle = conn_handle;

        Serial.print("Discovering toio Service ... ");
        service = new BLEClientService(SERVICE_UUID);
        service->begin();
        if (!service->discover(conn_handle)) {
            Serial.println("No Service Found");
            disconnect();
            return false;
        }
        Serial.println("Service Found");
        Serial.print("Discovering Lamp Characteristic ... ");
        lamp = new BLEClientCharacteristic(LAMP_CHARACTERISTIC_UUID);
        lamp->begin();
        if (!lamp->discover()) {
            Serial.println("No Characteristic Found. Characteristic is mandatory but not found. ");
            disconnect();
            return false;
        }
        Serial.println("Characteristic Found");
        setLamp();
        return true;
    }

    void disconnect() {
        Serial.printf("disconnect: %d, %p, %p\n", conn_handle, service, lamp);
        if (conn_handle != BLE_CONN_HANDLE_INVALID) {
            Bluefruit.disconnect(conn_handle);
            conn_handle = BLE_CONN_HANDLE_INVALID;
        }
        if (service) {
            delete service;
            service = nullptr;
        }
        if (lamp) {
            delete lamp;
            service = nullptr;
        }
    }

    void setLamp() {
        uint8_t data[] = {0x03, 0x00, 0x01, 0x01, 0xff, 0xff, 0xff};
        lamp->write_resp(data, sizeof(data));
    }

   private:
    uint16_t conn_handle;
    BLEClientService *service;
    BLEClientCharacteristic *lamp;
};

uint8_t Cube::SERVICE_UUID[] = {0xAE, 0xBB, 0xD7, 0xFC, 0x3E, 0xCF, 0x08, 0x95, 0x71, 0x45, 0x3B, 0x5B, 0x00, 0x01, 0xB2, 0x10};
uint8_t Cube::LAMP_CHARACTERISTIC_UUID[] = {0xAE, 0xBB, 0xD7, 0xFC, 0x3E, 0xCF, 0x08, 0x95, 0x71, 0x45, 0x3B, 0x5B, 0x03, 0x01, 0xB2, 0x10};
