#pragma once

#include <Arduino.h>
#include <bluefruit.h>

class MotorControl : public BLEClientCharacteristic {
   private:
    static uint8_t UUID[];

   public:
    MotorControl() : BLEClientCharacteristic(MotorControl::UUID) {}

    void Move() {
        uint8_t data[] = {0x02, 1, 1, 100, 2, 2, 100, 50};
        write(data, sizeof(data));
    }

    void Rotate() {
    }
};

uint8_t MotorControl::UUID[] = {0xAE, 0xBB, 0xD7, 0xFC, 0x3E, 0xCF, 0x08, 0x95, 0x71, 0x45, 0x3B, 0x5B, 0x02, 0x01, 0xB2, 0x10};
