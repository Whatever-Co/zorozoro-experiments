#include <Arduino.h>
#include <NimBLEAdvertisedDevice.h>
#include <NimBLEDevice.h>
#include <WiFi.h>

const char *ssid = "WHEREVER";
const char *password = "0364276022";

static WiFiClient client;

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice *advertisedDevice) {
        Serial.println(advertisedDevice->toString().c_str());
        client.printf("advertised\t%s\t%s\n", advertisedDevice->getAddress().toString().c_str(), advertisedDevice->getName().c_str());
    }
};

void scan(void *param) {
    auto scan = NimBLEDevice::getScan();
    scan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    scan->setActiveScan(true);
    scan->start(0, false);
}

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

    if (!client.connect("10.0.0.96", 12322)) {
        Serial.println("connection failed");
    }
    client.printf("hello\tscanner\t%s\n", WiFi.localIP().toString().c_str());

    NimBLEDevice::init("");

    xTaskCreateUniversal(scan, "scan", 8192, nullptr, 1, nullptr, 0);
}

static int prevPing = 0;

void loop() {
    int now = millis() / 5000;
    if (now > prevPing) {
        prevPing = now;
        Serial.printf("ping\t%d\n", now);
        client.printf("ping\t%d\n", now);
    }
    delay(1000);
}
