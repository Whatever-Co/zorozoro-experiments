#include <Arduino.h>
#include <NimBLEDevice.h>
#include <WiFi.h>

#include "cube.h"

//----------------------------------------

const char *ssid = "WHEREVER";
const char *password = "0364276022";
const char *controllerHost = "10.0.0.96";
const int controllerPort = 12322;

static WiFiClient controller;
static Cube *cubes[MAX_CUBES] = {nullptr};

//----------------------------------------

void disconnect(NimBLEClient *client);

//----------------------------------------

class MyClientCallback : public NimBLEClientCallbacks {
   public:
    void onConnect(NimBLEClient *client) {
        Serial.printf("MyClientCallback: onConnect: %s\n", client->getPeerAddress().toString().c_str());
    }
    void onDisconnect(NimBLEClient *client) {
        Serial.printf("MyClientCallback: onDisconnect: %s\n", client->getPeerAddress().toString().c_str());
        disconnect(client);
    }
};

static MyClientCallback clientCallback;

//----------------------------------------

void notifyCallback(NimBLERemoteCharacteristic *characteristic, uint8_t *data, size_t length, bool isNotify) {
    auto client = characteristic->getRemoteService()->getClient();
    auto address = client->getPeerAddress().toString();
    auto uuid = characteristic->getUUID();
    if (uuid == Cube::batteryCharUUID) {
        uint8_t value = data[0];
        controller.printf("battery\t%s\t%d\n", address.c_str(), value);
        Serial.printf("battery\t%s\t%d\n", address.c_str(), value);
    } else if (uuid == Cube::buttonCharUUID) {
        uint8_t id = data[0];
        uint8_t state = data[1];
        controller.printf("button\t%d\t%d\n", id, state);
        Serial.printf("button\t%d\t%d\n", id, state);
    }
}

//----------------------------------------

void setup() {
    Serial.begin(115200);
    delay(10);

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
}

//----------------------------------------

void loop() {
    if (!controller.connect(controllerHost, controllerPort)) {
        Serial.println("connection failed");
        delay(5000);
        return;
    }
    controller.printf("hello\tbridge\t%s\n", WiFi.localIP().toString().c_str());
    while (controller.connected()) {
        while (controller.available() > 0) {
            auto data = controller.readStringUntil('\n');
            Serial.println(data);
            int i = data.indexOf('\t');
            if (i >= 0) {
                auto command = data.substring(0, i);
                auto arg = data.substring(i + 1);
                Serial.printf("command = \"%s\", arg = \"%s\"\n", command.c_str(), arg.c_str());
                if (command == "connect") {
                    connect(arg);
                }
            }
        }
        reportConnected(false);
        delay(10);
    }
    controller.stop();
    Serial.println("controller disconnected");
    delay(5000);
}

//----------------------------------------

void connect(String address) {
    if (NimBLEDevice::getClientListSize() >= MAX_CUBES) {
        Serial.println("cannot connect no more cubes..");
        return;
    }

    auto cube = new Cube();
    if (!cube->connect(address, &clientCallback, notifyCallback)) {
        delete cube;
        Serial.println("connect failed");
        return;
    }
    Serial.println("connected");
    for (int i = 0; i < MAX_CUBES; i++) {
        if (cubes[i] == nullptr) {
            cubes[i] = cube;
            break;
        }
    }
    reportConnected(true);
}

//----------------------------------------

void disconnect(NimBLEClient *client) {
    for (int i = 0; i < MAX_CUBES; i++) {
        if (cubes[i] != nullptr && cubes[i]->getClient() == client) {
            delete cubes[i];
            cubes[i] = nullptr;
            break;
        }
    }
    reportConnected(true);
}

//----------------------------------------

static int prevReportTime = 0;
static int reportInterval = 5000;

void reportConnected(bool force) {
    int dt = millis() - prevReportTime;
    if (force || dt > reportInterval) {
        String list = "cubes\t";
        int count = 0;
        for (int i = 0; i < MAX_CUBES; i++) {
            if (cubes[i] != nullptr) {
                if (count++ > 0) {
                    list += ",";
                }
                list += cubes[i]->getAddress();
            }
        }
        controller.println(list);
        Serial.println(list);
        prevReportTime = millis();
    }
}
