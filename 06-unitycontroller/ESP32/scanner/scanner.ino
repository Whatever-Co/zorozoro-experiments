#include <Arduino.h>
#include <NimBLEAdvertisedDevice.h>
#include <NimBLEDevice.h>
#include <WiFi.h>

//----------------------------------------

const char *ssid = "WHEREVER";
const char *password = "0364276022";
const char *controllerHost = "10.0.0.96";
const int controllerPort = 12322;

static WiFiClient controller;

//----------------------------------------

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice *advertisedDevice) {
        if (advertisedDevice->getName() == "toio Core Cube") {
            Serial.println(advertisedDevice->toString().c_str());
            controller.printf("advertised\t%s\t%d\t%s\n",
                              advertisedDevice->getAddress().toString().c_str(),
                              advertisedDevice->getAddress().getType(),
                              advertisedDevice->getName().c_str());
        }
    }
};

//----------------------------------------

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.disconnect(true, true);
    WiFi.begin(ssid, password);

    int checkCount = 0;
    int retryCount = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        if (checkCount++ > 5) {
            WiFi.disconnect(true, true);
            WiFi.begin(ssid, password);
            checkCount = 0;
            retryCount += 1;
        }
        if (retryCount > 3) {
            ESP.restart();
        }
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    NimBLEDevice::init("");

    auto scan = NimBLEDevice::getScan();
    scan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    scan->setActiveScan(true);
}

void loop() {
    if (!controller.connect(controllerHost, controllerPort)) {
        Serial.println("connection failed");
        delay(5000);
        return;
    }
    controller.printf("hello\tscanner\t%s\n", WiFi.localIP().toString().c_str());
    auto scan = NimBLEDevice::getScan();
    while (controller.connected()) {
        scan->start(3, false);

        int now = millis() / 1000;
        Serial.printf("ping\t%d\n", now);
        controller.printf("ping\t%d\n", now);
    }
    controller.stop();
    Serial.println("controller disconnected");
    delay(5000);
}
