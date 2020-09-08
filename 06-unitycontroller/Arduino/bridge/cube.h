#pragma once

#include <Arduino.h>
#include <bluefruit.h>

#include <memory>

#include "id.h"
#include "lamp.h"
#include "motor.h"
#include "sensor.h"

class Address {
   public:
    static String ToString(ble_gap_addr_t &addr) {
        char tmp[18] = {0};
        sprintf(tmp, "%02x:%02x:%02x:%02x:%02x:%02x", addr.addr[5], addr.addr[4], addr.addr[3], addr.addr[2], addr.addr[1], addr.addr[0]);
        return String(tmp);
    }
    static ble_gap_addr_t FromString(String str) {
        ble_gap_addr_t addr;
        const char *s = str.c_str();
        char tmp[3] = {0};
        for (int i = 0; i < 6; i++) {
            memcpy(tmp, s + (5 - i) * 3, 2);
            addr.addr[i] = strtol(tmp, NULL, 16);
        }
        addr.addr_id_peer = 0;
        addr.addr_type = BLE_GAP_ADDR_TYPE_RANDOM_STATIC;
        return addr;
    }
};

class Cube {
   public:
    static uint8_t ServiceUUID[];
    static uint8_t BatteryCharacteristicUUID[];

   private:
    static std::shared_ptr<Cube> cubes_[];
    static size_t num_cubes_;

   public:
    static void NewCube(uint16_t conn_handle) {
        auto cube = std::make_shared<Cube>();
        cube->Setup(conn_handle);
        Cube::cubes_[conn_handle] = cube;
        num_cubes_++;
    }

    static void DeleteCube(uint16_t conn_handle) {
        Cube::cubes_[conn_handle] = nullptr;
        num_cubes_--;
    }

    static void UpdateAll() {
        for (int i = 0; i < BLE_MAX_CONNECTION; i++) {
            if (cubes_[i]) {
                cubes_[i]->Update();
            }
        }
    }

    static std::shared_ptr<Cube> GetCube(ble_gap_addr_t query) {
        for (int i = 0; i < BLE_MAX_CONNECTION; i++) {
            auto cube = cubes_[i];
            if (!cube) continue;
            auto connection = Bluefruit.Connection(cube->conn_handle_);
            auto addr = connection->getPeerAddr();
            bool match = true;
            for (int i = 0; i < 6 && match; i++) {
                match = query.addr[i] == addr.addr[i];
            }
            if (match) {
                return cube;
            }
        }
        return nullptr;
    }

    static size_t getNumCubes() {
        return num_cubes_;
    }

   private:
    static void NotifyCallback(BLEClientCharacteristic *chr, uint8_t *data, uint16_t length) {
        auto cube = Cube::cubes_[chr->connHandle()];
        if (chr == cube->id_info_.get()) {
            cube->NotifyId(data, length);
        } else if (chr == cube->sensor_info_.get()) {
            cube->NotifySensor(data);
        } else if (chr == cube->battery_info_.get()) {
            cube->NotifyBattery(data[0]);
        }
    }

   public:
    Cube()
        : conn_handle_(BLE_CONN_HANDLE_INVALID),
          service_(nullptr),
          id_info_(nullptr),
          sensor_info_(nullptr),
          motor_control_(nullptr),
          lamp_control_(nullptr),
          battery_info_(nullptr),
          battery_value_(-1) {}

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

        Serial.print("Discovering ID Characteristic ... ");
        id_info_ = std::make_shared<IdInfo>();
        id_info_->begin();
        if (!id_info_->discover()) {
            Serial.println("No Characteristic Found. Characteristic is mandatory but not found. ");
            Disconnect();
            return false;
        }
        id_info_->setNotifyCallback(&Cube::NotifyCallback);
        id_info_->enableNotify();
        Serial.println("Characteristic Found");

        // Serial.print("Discovering Sensor Characteristic ... ");
        // sensor_info_ = std::make_shared<SensorInfo>();
        // sensor_info_->begin();
        // if (!sensor_info_->discover()) {
        //     Serial.println("No Characteristic Found. Characteristic is mandatory but not found. ");
        //     Disconnect();
        //     return false;
        // }
        // sensor_info_->setNotifyCallback(&Cube::NotifyCallback);
        // sensor_info_->enableNotify();
        // Serial.println("Characteristic Found");

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
        motor_control_->Test();
        start = millis();
        return true;
    }

    uint32_t start;
    uint32_t previous = 0;

    void Update() {
        uint32_t now = millis();
        uint32_t d = now / 3000;
        if (previous != d) {
            previous = d;
            // motor_control_->Rotate((d * 135) & 0xffff);
            motor_control_->Grid(conn_handle_, (d * 135) & 0xffff);
        }
    }

    void NotifyId(uint8_t *data, uint16_t length) {
        switch (data[0]) {
            case 0x01:  // Position ID;
            {
                uint16_t *p = (uint16_t *)&data[1];
                uint16_t center_x = p[0];
                uint16_t center_y = p[1];
                uint16_t angle = p[2];
                // PrintAddress();
                // Serial.printf(" x=%d, y=%d, angle=%d\n", center_x, center_y, angle);
                break;
            }
            case 0x02:  // Standard ID
            {
                uint32_t id = *(uint32_t *)&data[1];
                uint16_t angle = *(uint16_t *)&data[5];
                Serial.printf(" id=%d(0x%x), angle=%d%d\n", id, id, angle);
                break;
            }
            case 0x03:  // Position ID missed
            {
                PrintAddress();
                Serial.println(" Position ID missed");
                break;
            }
            case 0x04:  // Standard ID missed
            {
                PrintAddress();
                Serial.println(" Standard ID missed");
                break;
            }
        }
    }

    void NotifySensor(uint8_t *data) {
        PrintAddress();
        Serial.printf(" Sensor: %d, %d, %d, %d, %d\n", data[0], data[1], data[2], data[3], data[4]);
    }

    void NotifyBattery(uint8_t value) {
        PrintAddress();
        Serial.printf(" Battery: %d\n", value);
        if (value == battery_value_) {
            return;
        }
        battery_value_ = value;
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
        id_info_ = nullptr;
        sensor_info_ = nullptr;
        motor_control_ = nullptr;
        lamp_control_ = nullptr;
        battery_info_ = nullptr;
        battery_value_ = -1;
    }

    void PrintAddress() {
        auto addr = Bluefruit.Connection(conn_handle_)->getPeerAddr().addr;
        Serial.printf("[%02X:%02X:%02X:%02X:%02X:%02X]", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
    }

   private:
    uint16_t conn_handle_;
    std::shared_ptr<BLEClientService> service_;
    std::shared_ptr<IdInfo> id_info_;
    std::shared_ptr<SensorInfo> sensor_info_;
    std::shared_ptr<MotorControl> motor_control_;
    std::shared_ptr<LampControl> lamp_control_;
    std::shared_ptr<BLEClientCharacteristic> battery_info_;

    int battery_value_;
};

uint8_t Cube::ServiceUUID[] = {0xAE, 0xBB, 0xD7, 0xFC, 0x3E, 0xCF, 0x08, 0x95, 0x71, 0x45, 0x3B, 0x5B, 0x00, 0x01, 0xB2, 0x10};
uint8_t Cube::BatteryCharacteristicUUID[] = {0xAE, 0xBB, 0xD7, 0xFC, 0x3E, 0xCF, 0x08, 0x95, 0x71, 0x45, 0x3B, 0x5B, 0x08, 0x01, 0xB2, 0x10};

std::shared_ptr<Cube> Cube::cubes_[BLE_MAX_CONNECTION];
size_t Cube::num_cubes_ = 0;