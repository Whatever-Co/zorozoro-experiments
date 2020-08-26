#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>

#include "NimBLEDevice.h"

// #define ENABLE_OTA

// static WiFiClient wifi;
static WiFiUDP udp;

#define HOST "10.0.0.96"
#define PORT 12322

void print(const char *text) {
    Serial.print(text);
    // wifi.print(text);
    udp.beginPacket(HOST, PORT);
    udp.write((const uint8_t *)text, strlen(text));
    udp.endPacket();
}

void println(const char *text) {
    print(text);
    print("\n");
}

void println(const String &text) {
    println(text.c_str());
}

#define MAX_CUBES 8

static NimBLEUUID serviceUUID("10B20100-5B3B-4571-9508-CF3EFCD7BBAE");
static NimBLEUUID lampCharUUID("10B20103-5B3B-4571-9508-CF3EFCD7BBAE");
static NimBLEUUID batteryCharUUID("10B20108-5B3B-4571-9508-CF3EFCD7BBAE");

class toioCube : NimBLEClientCallbacks {
   public:
    toioCube(NimBLEAdvertisedDevice *device)
        : connected(false),
          advertisedDevice(device),
          client(nullptr),
          lamp(nullptr) {}

    boolean connect() {
        address = advertisedDevice->getAddress().toString();
        log("Forming a connection...");

        client = NimBLEDevice::createClient();
        log(" - Created client");
        client->setClientCallbacks(this);

        client->connect(advertisedDevice);
        log(" - Connected to server");

        auto service = client->getService(serviceUUID);
        if (service == nullptr) {
            print("Failed to find our service UUID: ");
            log(serviceUUID.toString().c_str());
            disconnect();
            return false;
        }
        log(" - Found our service");

        lamp = service->getCharacteristic(lampCharUUID);
        if (lamp == nullptr) {
            print("Failed to find our characteristic UUID: ");
            log(lampCharUUID.toString().c_str());
            disconnect();
            return false;
        }
        battery = service->getCharacteristic(batteryCharUUID);
        if (battery == nullptr) {
            print("Failed to find our characteristic UUID: ");
            log(batteryCharUUID.toString().c_str());
            disconnect();
            return false;
        }
        log(" - Found our characteristic");

        if (battery->canRead()) {
            log("Battery: " + String(battery->readUInt8()));
        }
        if (battery->canNotify()) {
            battery->subscribe(true, [](NimBLERemoteCharacteristic *characteristic, uint8_t *data, size_t length, bool isNotify) {
                // log("Notification: length = " + String(length));
                // log("Battery: " + String(data[0]));
            });
        }

        connected = true;

        sendCommand();

        return true;
    }

    void disconnect() {
        client->disconnect();
        client = nullptr;
        lamp = nullptr;
        connected = false;
    }

    boolean isConnected() {
        return connected;
    }

    void sendCommand() {
        uint8_t data[] = {0x04, 0x01, 0x04,
                          0x10, 0x01, 0x01, 0xff, 0xff, 0x00,
                          0x10, 0x01, 0x01, 0x00, 0xff, 0x00,
                          0x10, 0x01, 0x01, 0x00, 0xff, 0xff,
                          0x10, 0x01, 0x01, 0xff, 0x00, 0xff};
        // printf("data len = %d\n", sizeof(data));
        lamp->writeValue(data, sizeof(data), true);

        log("Battery: " + String(battery->readUInt8()));
    }

    void onConnect(NimBLEClient *client) {
        log("onConnect");
    }

    void onDisconnect(NimBLEClient *client) {
        log("onDisconnect");
        disconnect();
    }

   private:
    void log(const char *text) {
        print("[");
        print(address.c_str());
        print("] ");
        println(text);
    }

    void log(const String &text) {
        log(text.c_str());
    }

    std::string address;
    boolean connected;
    NimBLEAdvertisedDevice *advertisedDevice;
    NimBLEClient *client;
    NimBLERemoteCharacteristic *lamp;
    NimBLERemoteCharacteristic *battery;
};

class App : NimBLEAdvertisedDeviceCallbacks {
   public:
    App()
        : scan(nullptr),
          pendingDevice(nullptr),
          connectedCubes(0) {}

    void setup() {
        Serial.println("Starting Arduino BLE Client application.....");

        NimBLEDevice::init("");

        scan = NimBLEDevice::getScan();
        scan->setAdvertisedDeviceCallbacks(this);
        scan->setActiveScan(true);
        scan->start(0, false);
    }

    void loop() {
        if (pendingDevice != nullptr) {
            auto cube = new toioCube(pendingDevice);
            if (cube->connect()) {
                addCube(cube);
            }
            pendingDevice = nullptr;
        }
        if (connectedCubes < MAX_CUBES) {
            scan->start(1, false);
            // } else {
            //     delay(1000);
        }
        for (int i = 0; i < MAX_CUBES; i++) {
            if (cubes[i] != nullptr) {
                if (cubes[i]->isConnected()) {
                    cubes[i]->sendCommand();
                } else {
                    println(String(i) + " disconnected");
                    cubes[i] = nullptr;
                    connectedCubes--;
                }
            }
        }
    }

    void onResult(NimBLEAdvertisedDevice *advertisedDevice) {
        if (advertisedDevice->isAdvertisingService(serviceUUID)) {
            print("toio Core Cube found: ");
            println(advertisedDevice->toString().c_str());
            scan->stop();
            pendingDevice = advertisedDevice;
        }
    }

    void addCube(toioCube *cube) {
        boolean added = false;
        for (int i = 0; i < MAX_CUBES; i++) {
            if (cubes[i] == nullptr) {
                cubes[i] = cube;
                added = true;
                break;
            }
        }
        if (!added) {
            println("Cannot add no more cubes...");
        }
        connectedCubes++;
    }

   private:
    NimBLEScan *scan;
    NimBLEAdvertisedDevice *pendingDevice;
    int connectedCubes;
    toioCube *cubes[MAX_CUBES];
};

static App app;

void setup() {
    Serial.begin(112500);
    delay(1000);

    WiFi.mode(WIFI_STA);
    WiFi.begin("WHEREVER", "0364276022");
    while (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.println("Connection Failed! Rebooting...");
        delay(5000);
        ESP.restart();
    }
    Serial.println("Ready");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    delay(1000);

    // if (wifi.connect("10.0.0.96", 12322)) {
    println("SUccessfully connected to the server");
    println("Hellooo!!!!!!!!!!!!");
    // } else {
    //     Serial.println("Failed to connect to the server");
    // }

#ifdef ENABLE_OTA
    ArduinoOTA
        .onStart([]() {
            String type;
            if (ArduinoOTA.getCommand() == U_FLASH)
                type = "sketch";
            else  // U_SPIFFS
                type = "filesystem";

            // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
            Serial.println("Start updating " + type);
        })
        .onEnd([]() {
            Serial.println("\nEnd");
        })
        .onProgress([](unsigned int progress, unsigned int total) {
            Serial.printf("Progress: %u%% (%u/%u)\r", (progress / (total / 100)), progress, total);
        })
        .onError([](ota_error_t error) {
            Serial.printf("Error[%u]: ", error);
            if (error == OTA_AUTH_ERROR)
                Serial.println("Auth Failed");
            else if (error == OTA_BEGIN_ERROR)
                Serial.println("Begin Failed");
            else if (error == OTA_CONNECT_ERROR)
                Serial.println("Connect Failed");
            else if (error == OTA_RECEIVE_ERROR)
                Serial.println("Receive Failed");
            else if (error == OTA_END_ERROR)
                Serial.println("End Failed");
        });

    ArduinoOTA.begin();
#endif

    app.setup();
}

void loop() {
#ifdef ENABLE_OTA
    ArduinoOTA.handle();
#endif
    app.loop();
}
