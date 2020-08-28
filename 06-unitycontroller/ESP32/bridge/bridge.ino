#include <Arduino.h>
#include <NimBLEDevice.h>
#include <PubSubClient.h>
#include <WiFi.h>

#include "cube.h"

//----------------------------------------

const char *ssid = "WHEREVER";
const char *password = "0364276022";
const char *controllerHost = "10.0.0.96";
const int controllerPort = 1883;

static WiFiClient wifi;
static PubSubClient mq(wifi);

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
    auto address = characteristic->getRemoteService()->getClient()->getPeerAddress().toString();
    auto uuid = characteristic->getUUID();
    char payload[32];
    if (uuid == Cube::buttonInfoCharUUID) {
        uint8_t id = data[0];
        uint8_t state = data[1];
        sprintf(payload, "%s,%d,%d", address.c_str(), id, state);
        mq.publish("button", payload);
        Serial.printf("button,%s\n", payload);
    } else if (uuid == Cube::batteryInfoCharUUID) {
        uint8_t value = data[0];
        sprintf(payload, "%s,%d", address.c_str(), value);
        mq.publish("battery", payload);
        Serial.printf("battery,%s\n", payload);
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

    mq.setServer(controllerHost, controllerPort);
    mq.setCallback(callback);

    NimBLEDevice::init("");
}

//----------------------------------------

void loop() {
    if (!mq.connected()) {
        Serial.println("connection failed");
        reconnect();
    }
    mq.loop();
    delay(100);
}

//----------------------------------------

void reconnect() {
    while (!mq.connected()) {
        Serial.print("Attempting MQTT connection...");
        auto address = WiFi.localIP().toString();
        if (mq.connect(address.c_str())) {
            Serial.println("connected");
            mq.publish("hello", "bridge");
            auto topic = address + "/#";
            mq.subscribe(topic.c_str());
        } else {
            Serial.print("failed, rc=");
            Serial.print(mq.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

//----------------------------------------

void callback(char *topic, byte *payload, unsigned int length) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++) {
        Serial.printf("%02X ", (char)payload[i]);
    }
    Serial.println();

    String t(topic);
    int i = t.indexOf('/');
    if (i == -1) {
        return;
    }
    t = t.substring(i + 1);
    Serial.println(t);
    if (t == "connect") {
        char tmp[length + 1];
        memcpy(tmp, payload, length);
        tmp[length] = 0;
        String address(tmp);
        auto cube = connect(address);
        if (cube) {
            mq.publish("connected", tmp);
            Serial.printf("connected\t%s\n", tmp);
            reportConnected(true);
        } else {
            mq.publish("disconnected", tmp);
            Serial.printf("disconnected\t%s\n", tmp);
        }
    } else if (t == "lamp") {
        char tmp[18] = {0};
        memcpy(tmp, payload, 17);
        String address(tmp);
        for (auto cube : cubes) {
            if (cube && cube->getAddress() == address) {
                Serial.printf("found cube with address %s\n", tmp);
                cube->SetLamp(&payload[18], payload[17]);
            }
        }
    }
}

//----------------------------------------

Cube *connect(String address) {
    if (NimBLEDevice::getClientListSize() >= MAX_CUBES) {
        Serial.println("cannot connect no more cubes..");
        return nullptr;
    }

    auto cube = new Cube();
    if (!cube->connect(address, &clientCallback, notifyCallback)) {
        delete cube;
        Serial.println("connect failed");
        return nullptr;
    }
    for (int i = 0; i < MAX_CUBES; i++) {
        if (cubes[i] == nullptr) {
            cubes[i] = cube;
            break;
        }
    }
    return cube;
}

//----------------------------------------

void disconnect(NimBLEClient *client) {
    for (int i = 0; i < MAX_CUBES; i++) {
        auto cube = cubes[i];
        if (cube != nullptr && cube->getClient() == client) {
            // controller.printf("disconnected\t%s\n", cube->getAddress().c_str());
            mq.publish("disconnected", cube->getAddress().c_str());
            Serial.printf("disconnected\t%s\n", cube->getAddress().c_str());
            delete cube;
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
        // controller.println(list);
        Serial.println(list);
        prevReportTime = millis();
    }
}
