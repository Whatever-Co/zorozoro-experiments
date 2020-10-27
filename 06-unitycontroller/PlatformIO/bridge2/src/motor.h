#pragma once

#include <Arduino.h>
#include <bluefruit.h>

class MotorControl : public BLEClientCharacteristic {
   private:
    static uint8_t UUID[];

   public:
    MotorControl() : BLEClientCharacteristic(MotorControl::UUID) {}

    void Rotate(uint8_t angle) {
        MoveTo(0, 5, 0, 80, 0, 0xffff, 0xffff, angle, 1);
    }

    void Grid(uint16_t id, uint16_t angle) {
        uint8_t x = id % 3;
        uint8_t y = id / 3;
        uint16_t px = 250 + (x * 60) - 60;
        uint16_t py = 250 + (y * 60) - 60;
        MoveTo(0, 5, 0, 30, 0, px, py, angle, 1);
    }

    void MoveTo(uint8_t id, uint8_t timeout, uint8_t move_type, uint8_t max_speed, uint8_t acc_type, uint16_t target_x, uint16_t target_y, uint16_t target_angle, uint8_t angle_type) {
        uint8_t data[13];
        data[0] = 0x03;
        data[1] = id;
        data[2] = timeout;
        data[3] = move_type;
        data[4] = max_speed;
        data[5] = acc_type;
        data[6] = 0x00;
        *(uint16_t*)&data[7] = target_x;
        *(uint16_t*)&data[9] = target_y;
        *(uint16_t*)&data[11] = (angle_type << 13) | target_angle;
        write(data, sizeof(data));
    }

    void Test() {
        uint8_t data[] = {0x02, 1, 1, 100, 2, 2, 100, 50};
        write(data, sizeof(data));
    }
};

uint8_t MotorControl::UUID[] = {0xAE, 0xBB, 0xD7, 0xFC, 0x3E, 0xCF, 0x08, 0x95, 0x71, 0x45, 0x3B, 0x5B, 0x02, 0x01, 0xB2, 0x10};
