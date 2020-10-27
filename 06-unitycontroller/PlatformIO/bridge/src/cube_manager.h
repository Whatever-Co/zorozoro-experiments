#pragma once

#include <Arduino.h>
#include <bluefruit.h>

#include <map>
#include <memory>
#include <vector>

#include "cube.h"

class CubeManager {
   public:
    static std::shared_ptr<Cube> Setup(const uint16_t conn_handle) {
        auto cube = std::make_shared<Cube>();
        if (cube->Setup(conn_handle)) {
            cubes_.emplace(cube->GetAddress(), cube);
            return cube;
        }
        return nullptr;
    }

    static void Cleanup(const uint16_t conn_handle) {
        auto address = GetAddress(conn_handle);
        cubes_.erase(address);
    }

    static String GetAddress(uint16_t conn_handle) {
        auto cube = GetCube(conn_handle);
        if (cube) {
            return cube->GetAddress();
        }
        return String("");
    }

    static std::shared_ptr<Cube> GetCube(uint16_t conn_handle) {
        for (const auto& x : cubes_) {
            auto& cube = x.second;
            if (cube->GetConnection() == conn_handle) {
                return cube;
            }
        }
        return nullptr;
    }

    static std::shared_ptr<Cube> GetCube(String address) {
        if (cubes_.count(address)) {
            return cubes_.at(address);
        }
        return nullptr;
    }

    static size_t GetNumCubes() {
        return cubes_.size();
    }

    static std::vector<String> GetAddresses() {
        std::vector<String> addresses;
        for (const auto& x : cubes_) {
            addresses.push_back(x.first);
        }
        return addresses;
    }

   private:
    static std::map<String, std::shared_ptr<Cube>> cubes_;
};

std::map<String, std::shared_ptr<Cube>> CubeManager::cubes_;
