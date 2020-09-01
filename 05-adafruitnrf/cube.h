#pragma once

#include <Arduino.h>
#include <bluefruit.h>

#include <memory>

#include "lamp.h"

class Cube {
   public:
    static uint8_t ServiceUUID[];
    static uint8_t LampCharacteristicUUID[];
    static uint8_t BatteryCharacteristicUUID[];

    static std::shared_ptr<Cube> cubes[];

    static void new_cube(uint16_t conn_handle) {
        auto cube = std::make_shared<Cube>();
        cube->setup(conn_handle);
        Cube::cubes[conn_handle] = cube;
    }

    static void delete_cube(uint16_t conn_handle) {
        Cube::cubes[conn_handle] = nullptr;
    }

   private:
    static void notify_callback(BLEClientCharacteristic *chr, uint8_t *data, uint16_t len) {
        Serial.printf("notify_callback: %d, %d\n", len, data[0]);
        auto cube = Cube::cubes[chr->connHandle()];
        if (chr == cube->battery.get()) {
            cube->notify_battery(data[0]);
        }
    }

   public:
    Cube()
        : conn_handle(BLE_CONN_HANDLE_INVALID),
          service(nullptr),
          lamp(nullptr),
          battery(nullptr) {}

    ~Cube() {
        disconnect();
    }

    bool setup(uint8_t conn_handle) {
        this->conn_handle = conn_handle;

        Serial.print("Discovering toio Service ... ");
        service = std::make_shared<BLEClientService>(ServiceUUID);
        service->begin();
        if (!service->discover(conn_handle)) {
            Serial.println("No Service Found");
            disconnect();
            return false;
        }
        Serial.println("Service Found");

        Serial.print("Discovering Lamp Characteristic ... ");
        lamp = std::make_shared<LampControl>();
        lamp->begin();
        if (!lamp->discover()) {
            Serial.println("No Characteristic Found. Characteristic is mandatory but not found. ");
            disconnect();
            return false;
        }
        Serial.println("Characteristic Found");

        Serial.print("Discovering Battery Characteristic ... ");
        battery = std::make_shared<BLEClientCharacteristic>(BatteryCharacteristicUUID);
        battery->begin();
        if (!battery->discover()) {
            Serial.println("No Characteristic Found. Characteristic is mandatory but not found. ");
            disconnect();
            return false;
        }
        battery->setNotifyCallback(&Cube::notify_callback);
        battery->enableNotify();
        Serial.println("Characteristic Found");

        lamp->set_color(255, 255, 255);
        return true;
    }

    void notify_battery(uint8_t value) {
        if (value <= 10) {
            lamp->set_blink(255, 0, 0, 300);
        } else if (value <= 20) {
            lamp->set_color(255, 0, 0);
        } else if (value <= 50) {
            lamp->set_color(255, 176, 25);
        } else {
            lamp->set_color(0, 255, 0);
        }
    }

    void disconnect() {
        Serial.printf("disconnect: %d, %p, %p\n", conn_handle, service, lamp);
        if (conn_handle != BLE_CONN_HANDLE_INVALID) {
            Bluefruit.disconnect(conn_handle);
            conn_handle = BLE_CONN_HANDLE_INVALID;
        }
        service = nullptr;
        service = nullptr;
        service = nullptr;
    }

   private:
    uint16_t conn_handle;
    std::shared_ptr<BLEClientService> service;
    std::shared_ptr<LampControl> lamp;
    std::shared_ptr<BLEClientCharacteristic> battery;
};

uint8_t Cube::ServiceUUID[] = {0xAE, 0xBB, 0xD7, 0xFC, 0x3E, 0xCF, 0x08, 0x95, 0x71, 0x45, 0x3B, 0x5B, 0x00, 0x01, 0xB2, 0x10};
uint8_t Cube::LampCharacteristicUUID[] = {0xAE, 0xBB, 0xD7, 0xFC, 0x3E, 0xCF, 0x08, 0x95, 0x71, 0x45, 0x3B, 0x5B, 0x03, 0x01, 0xB2, 0x10};
uint8_t Cube::BatteryCharacteristicUUID[] = {0xAE, 0xBB, 0xD7, 0xFC, 0x3E, 0xCF, 0x08, 0x95, 0x71, 0x45, 0x3B, 0x5B, 0x08, 0x01, 0xB2, 0x10};

std::shared_ptr<Cube> Cube::cubes[BLE_MAX_CONNECTION];
