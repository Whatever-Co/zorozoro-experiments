#pragma once

#include <Arduino.h>
#include <bluefruit.h>

#include <memory>

#include "lamp.h"
#include "motor.h"

class Cube {
   public:
    static uint8_t ServiceUUID[];
    static uint8_t BatteryCharacteristicUUID[];

   private:
    static std::shared_ptr<Cube> cubes_[];

   public:
    static void NewCube(uint16_t conn_handle) {
        auto cube = std::make_shared<Cube>();
        cube->Setup(conn_handle);
        Cube::cubes_[conn_handle] = cube;
    }

    static void DeleteCube(uint16_t conn_handle) {
        Cube::cubes_[conn_handle] = nullptr;
    }

   private:
    static void NotifyCallback(BLEClientCharacteristic *chr, uint8_t *data, uint16_t len) {
        Serial.printf("notify_callback: %d, %d\n", len, data[0]);
        auto cube = Cube::cubes_[chr->connHandle()];
        if (chr == cube->battery_info_.get()) {
            cube->NotifyBattery(data[0]);
        }
    }

   public:
    Cube()
        : conn_handle_(BLE_CONN_HANDLE_INVALID),
          service_(nullptr),
          motor_control_(nullptr),
          lamp_control_(nullptr),
          battery_info_(nullptr) {}

    ~Cube() {
        Disconnect();
    }

    bool Setup(uint8_t conn_handle) {
        this->conn_handle_ = conn_handle;

        Serial.print("Discovering toio Service ... ");
        service_ = std::make_shared<BLEClientService>(ServiceUUID);
        service_->begin();
        if (!service_->discover(conn_handle)) {
            Serial.println("No Service Found");
            Disconnect();
            return false;
        }
        Serial.println("Service Found");

        Serial.print("Discovering Motor Characteristic ... ");
        motor_control_ = std::make_shared<MotorControl>();
        motor_control_->begin();
        if (!motor_control_->discover()) {
            Serial.println("No Characteristic Found. Characteristic is mandatory but not found. ");
            Disconnect();
            return false;
        }
        Serial.println("Characteristic Found");

        Serial.print("Discovering Lamp Characteristic ... ");
        lamp_control_ = std::make_shared<LampControl>();
        lamp_control_->begin();
        if (!lamp_control_->discover()) {
            Serial.println("No Characteristic Found. Characteristic is mandatory but not found. ");
            Disconnect();
            return false;
        }
        Serial.println("Characteristic Found");

        Serial.print("Discovering Battery Characteristic ... ");
        battery_info_ = std::make_shared<BLEClientCharacteristic>(BatteryCharacteristicUUID);
        battery_info_->begin();
        if (!battery_info_->discover()) {
            Serial.println("No Characteristic Found. Characteristic is mandatory but not found. ");
            Disconnect();
            return false;
        }
        battery_info_->setNotifyCallback(&Cube::NotifyCallback);
        battery_info_->enableNotify();
        Serial.println("Characteristic Found");

        lamp_control_->SetColor(255, 255, 255);
        motor_control_->Move();
        return true;
    }

    void NotifyBattery(uint8_t value) {
        if (value <= 10) {
            lamp_control_->SetBlink(255, 0, 0, 300);
        } else if (value <= 20) {
            lamp_control_->SetColor(255, 0, 0);
        } else if (value <= 50) {
            lamp_control_->SetColor(255, 176, 25);
        } else {
            lamp_control_->SetColor(0, 255, 0);
        }
    }

    void Disconnect() {
        Serial.printf("disconnect: %d, %p, %p\n", conn_handle_, service_, lamp_control_);
        if (conn_handle_ != BLE_CONN_HANDLE_INVALID) {
            Bluefruit.disconnect(conn_handle_);
            conn_handle_ = BLE_CONN_HANDLE_INVALID;
        }
        service_ = nullptr;
        motor_control_ = nullptr;
        lamp_control_ = nullptr;
        battery_info_ = nullptr;
    }

   private:
    uint16_t conn_handle_;
    std::shared_ptr<BLEClientService> service_;
    std::shared_ptr<MotorControl> motor_control_;
    std::shared_ptr<LampControl> lamp_control_;
    std::shared_ptr<BLEClientCharacteristic> battery_info_;
};

uint8_t Cube::ServiceUUID[] = {0xAE, 0xBB, 0xD7, 0xFC, 0x3E, 0xCF, 0x08, 0x95, 0x71, 0x45, 0x3B, 0x5B, 0x00, 0x01, 0xB2, 0x10};
uint8_t Cube::BatteryCharacteristicUUID[] = {0xAE, 0xBB, 0xD7, 0xFC, 0x3E, 0xCF, 0x08, 0x95, 0x71, 0x45, 0x3B, 0x5B, 0x08, 0x01, 0xB2, 0x10};

std::shared_ptr<Cube> Cube::cubes_[BLE_MAX_CONNECTION];
