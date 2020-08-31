#include <Arduino.h>
#include <ESPmDNS.h>
#include <NimBLEAdvertisedDevice.h>
#include <NimBLEDevice.h>
#include <PubSubClient.h>
#include <WiFi.h>

//----------------------------------------

const char *ssid = "WHEREVER";
const char *password = "0364276022";
const char *controllerHost = "10.0.0.123";
// const char *ssid = "NETGEAR85";
// const char *password = "09077518842";
// const char *controllerHost = "10.77.1.141";
const int controllerPort = 1883;

static WiFiClient wifi;
static PubSubClient client(wifi);

//----------------------------------------

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice *advertisedDevice) {
        if (advertisedDevice->getName() == "toio Core Cube") {
            Serial.println(advertisedDevice->toString().c_str());
            if (client.connected()) {
                client.publish("newcube", advertisedDevice->getAddress().toString().c_str());
            }
        }
    }
};

void scan(void *param) {
    auto scan = NimBLEDevice::getScan();
    scan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    scan->setActiveScan(true);
    while (true) {
        scan->start(3, false);
        delay(1000);
    }
}

//----------------------------------------

void setup() {
    Serial.begin(115200);
    delay(10);

    Serial.print("\n\nConnecting to ");
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

    Serial.println("\nWiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    client.setServer(controllerHost, controllerPort);

    NimBLEDevice::init("");

    xTaskCreateUniversal(scan, "scan", 8192, nullptr, 1, nullptr, 0);
}

//----------------------------------------

void loop() {
    if (!client.connected()) {
        Serial.println("connection failed");
        reconnect();
    }
    client.loop();
    delay(100);
}

//----------------------------------------

void reconnect() {
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        if (client.connect(WiFi.localIP().toString().c_str())) {
            Serial.println("connected");
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}
