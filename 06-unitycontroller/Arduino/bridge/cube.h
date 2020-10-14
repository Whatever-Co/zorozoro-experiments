#pragma once

#include <Arduino.h>
#include <bluefruit.h>

#include <memory>

// #define ENABLE_SENSOR
// #define ENABLE_LAMP
// #define ENABLE_BATTERY

#include "hoge.h"
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
    static uint8_t ConfigurationCharacteristicUUID[];

    Cube()
        : conn_handle_(BLE_CONN_HANDLE_INVALID),
          service_(nullptr),
          id_info_(nullptr),
          sensor_info_(nullptr),
          motor_control_(nullptr),
          lamp_control_(nullptr),
          battery_info_(nullptr),
          configuration_(nullptr),
          battery_value_(-1) {}

    ~Cube() {
        Serial.println("~Cube...");
    }

    bool Setup(uint8_t conn_handle) {
        this->conn_handle_ = conn_handle;
        auto addr = Bluefruit.Connection(conn_handle)->getPeerAddr();
        address_ = Address::ToString(addr);

        Serial.print("Discovering toio Service ... ");
        service_ = std::make_shared<BLEClientService>(ServiceUUID);
        service_->begin();
        if (!service_->discover(conn_handle)) {
            Serial.println("No Service Found");
            return false;
        }
        Serial.println("Service Found");

        Serial.print("Discovering ID Characteristic ... ");
        id_info_ = std::make_shared<IdInfo>();
        id_info_->begin();
        if (!id_info_->discover()) {
            Serial.println("No Characteristic Found. Characteristic is mandatory but not found. ");
            return false;
        }
        id_info_->setNotifyCallback(&App::OnIdInfo);
        id_info_->enableNotify();
        Serial.println("Characteristic Found");

#ifdef ENABLE_SENSOR
        Serial.print("Discovering Sensor Characteristic ... ");
        sensor_info_ = std::make_shared<SensorInfo>();
        sensor_info_->begin();
        if (!sensor_info_->discover()) {
            Serial.println("No Characteristic Found. Characteristic is mandatory but not found. ");
            Disconnect();
            return false;
        }
        sensor_info_->setNotifyCallback(&Cube::NotifyCallback);
        sensor_info_->enableNotify();
        Serial.println("Characteristic Found");
#endif

        Serial.print("Discovering Motor Characteristic ... ");
        motor_control_ = std::make_shared<MotorControl>();
        motor_control_->begin();
        if (!motor_control_->discover()) {
            Serial.println("No Characteristic Found. Characteristic is mandatory but not found. ");
            return false;
        }
        // motor_control_->setNotifyCallback(&App::OnMotor);
        // motor_control_->enableNotify();
        Serial.println("Characteristic Found");

#ifdef ENABLE_LAMP
        Serial.print("Discovering Lamp Characteristic ... ");
        lamp_control_ = std::make_shared<LampControl>();
        lamp_control_->begin();
        if (!lamp_control_->discover()) {
            Serial.println("No Characteristic Found. Characteristic is mandatory but not found. ");
            return false;
        }
        Serial.println("Characteristic Found");
#endif

#ifdef ENABLE_BATTERY
        Serial.print("Discovering Battery Characteristic ... ");
        battery_info_ = std::make_shared<BLEClientCharacteristic>(BatteryCharacteristicUUID);
        battery_info_->begin();
        if (!battery_info_->discover()) {
            Serial.println("No Characteristic Found. Characteristic is mandatory but not found. ");
            return false;
        }
        battery_info_->setNotifyCallback(&App::OnBatteryInfo);
        battery_info_->enableNotify();
        Serial.println("Characteristic Found");
#endif

        Serial.print("Discovering Configuration Characteristic ... ");
        configuration_ = std::make_shared<BLEClientCharacteristic>(ConfigurationCharacteristicUUID);
        configuration_->begin();
        if (!configuration_->discover()) {
            Serial.println("No Characteristic Found. Characteristic is mandatory but not found. ");
            return false;
        }
        configuration_->setNotifyCallback(&App::OnMotor);
        configuration_->enableNotify();
        Serial.println("Characteristic Found");
        {
            uint8_t data[] = {0x18, 0x00, 0x20, 0x01};
            configuration_->write_resp(data, sizeof(data));
        }

#ifdef ENABLE_LAMP
        lamp_control_->SetColor(255, 255, 255);
#endif
        // motor_control_->Test();
        return true;
    }

    void SetLamp(uint8_t *data, size_t length) {
#ifdef ENABLE_LAMP
        lamp_control_->write_resp(data, length);
#endif
    }

    void SetMotor(uint8_t *data, size_t length) {
        motor_control_->write(data, length);
    }

    uint16_t GetConnection() { return conn_handle_; }

    String GetAddress() { return address_; }

    void PrintAddress() {
        auto addr = Bluefruit.Connection(conn_handle_)->getPeerAddr().addr;
        Serial.printf("[%02X:%02X:%02X:%02X:%02X:%02X]", addr[5], addr[4], addr[3], addr[2], addr[1], addr[0]);
    }

   private:
    uint16_t conn_handle_;
    String address_;
    std::shared_ptr<BLEClientService> service_;
    std::shared_ptr<IdInfo> id_info_;
    std::shared_ptr<SensorInfo> sensor_info_;
    std::shared_ptr<MotorControl> motor_control_;
    std::shared_ptr<LampControl> lamp_control_;
    std::shared_ptr<BLEClientCharacteristic> battery_info_;
    std::shared_ptr<BLEClientCharacteristic> configuration_;

    int battery_value_;
};

uint8_t Cube::ServiceUUID[] = {0xAE, 0xBB, 0xD7, 0xFC, 0x3E, 0xCF, 0x08, 0x95, 0x71, 0x45, 0x3B, 0x5B, 0x00, 0x01, 0xB2, 0x10};
uint8_t Cube::BatteryCharacteristicUUID[] = {0xAE, 0xBB, 0xD7, 0xFC, 0x3E, 0xCF, 0x08, 0x95, 0x71, 0x45, 0x3B, 0x5B, 0x08, 0x01, 0xB2, 0x10};
uint8_t Cube::ConfigurationCharacteristicUUID[] = {0xAE, 0xBB, 0xD7, 0xFC, 0x3E, 0xCF, 0x08, 0x95, 0x71, 0x45, 0x3B, 0x5B, 0xFF, 0x01, 0xB2, 0x10};
