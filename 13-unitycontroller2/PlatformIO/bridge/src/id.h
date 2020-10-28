#pragma once

#include <Arduino.h>
#include <bluefruit.h>

class IdInfo : public BLEClientCharacteristic {
   private:
    static uint8_t UUID[];

   public:
    IdInfo() : BLEClientCharacteristic(IdInfo::UUID) {}

    // void SetColor(uint8_t red, uint8_t green, uint8_t blue) {
    //     uint8_t data[] = {0x03, 0x00, 0x01, 0x01, red, green, blue};
    //     write_resp(data, sizeof(data));
    // }

    // void SetBlink(uint8_t red, uint8_t green, uint8_t blue, uint16_t interval) {
    //     uint8_t d = interval / 10;
    //     uint8_t data[] = {0x04, 0x00, 0x02,
    //                       d, 1, 1, red, green, blue,
    //                       d, 1, 1, 0, 0, 0};
    //     write_resp(data, sizeof(data));
    // }
};

uint8_t IdInfo::UUID[] = {0xAE, 0xBB, 0xD7, 0xFC, 0x3E, 0xCF, 0x08, 0x95, 0x71, 0x45, 0x3B, 0x5B, 0x01, 0x01, 0xB2, 0x10};
