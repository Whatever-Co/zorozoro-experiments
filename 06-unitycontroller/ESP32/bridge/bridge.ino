#include <Arduino.h>
#include <NimBLEDevice.h>
#include <PubSubClient.h>
#include <WiFi.h>

#include <map>
#include <memory>
#include <vector>

#include "cube.h"

//----------------------------------------

// const char *ssid = "WHEREVER";
// const char *password = "0364276022";
// const char *controllerHost = "10.0.0.96";
const char *ssid = "NETGEAR85";
const char *password = "09077518842";
const char *controllerHost = "10.77.1.141";
const int controllerPort = 1883;

static WiFiClient wifi;
static PubSubClient mq(wifi);

static std::map<String, std::shared_ptr<Cube>> cubes;

//----------------------------------------

void startAcceptNewCube();
void unsubscribeTopics(String address);

//----------------------------------------

class MyClientCallback : public NimBLEClientCallbacks {
   public:
    void onConnect(NimBLEClient *client) {
        Serial.printf("MyClientCallback: onConnect: %s\n", client->getPeerAddress().toString().c_str());
    }
    void onDisconnect(NimBLEClient *client) {
        Serial.printf("MyClientCallback: onDisconnect: %s\n", client->getPeerAddress().toString().c_str());
        for (const auto &kv : cubes) {
            auto &cube = kv.second;
            if (cube->getClient() == client) {
                char topic[32];
                sprintf(topic, "%s/disconnected", cube->getAddress().c_str());
                mq.publish(topic, "", true);
                Serial.printf("Published: %s\n", topic);
                unsubscribeTopics(cube->getAddress());
                cubes.erase(kv.first);

                startAcceptNewCube();
                break;
            }
        }
    }
};

static MyClientCallback clientCallback;

//----------------------------------------

void notifyCallback(NimBLERemoteCharacteristic *characteristic, uint8_t *data, size_t length, bool isNotify) {
    auto address = characteristic->getRemoteService()->getClient()->getPeerAddress().toString();
    auto uuid = characteristic->getUUID();
    char topic[32], payload[32];
    if (uuid == Cube::buttonInfoCharUUID) {
        sprintf(topic, "%s/button", address.c_str());
        uint8_t id = data[0];
        uint8_t state = data[1];
        mq.publish(topic, &state, 1);
        Serial.printf("Published: %s,%d\n", topic, state);
    } else if (uuid == Cube::batteryInfoCharUUID) {
        sprintf(topic, "%s/battery", address.c_str());
        uint8_t value = data[0];
        mq.publish(topic, &value, 1);
        Serial.printf("Published: %s,%d\n", topic, value);
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
    mq.setCallback(messageReceived);

    NimBLEDevice::init("");
}

//----------------------------------------

void loop() {
    if (!mq.connected()) {
        Serial.println("connection failed");
        reconnectToServer();
    }
    mq.loop();
    delay(100);
}

//----------------------------------------

void reconnectToServer() {
    while (!mq.connected()) {
        Serial.print("Attempting MQTT connection...");
        auto address = WiFi.localIP().toString();
        if (mq.connect(address.c_str(), "bye", 0, true, "")) {
            Serial.println("connected");
            mq.publish("hello", "", true);
            for (auto &kv : cubes) {
                subscribeTopics(kv.first);
            }
            startAcceptNewCube();
        } else {
            Serial.print("failed, rc=");
            Serial.print(mq.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}

//----------------------------------------

static const char *requiredTopics[] = {"lamp"};

void subscribeTopics(String address) {
    char topic[32];
    for (auto t : requiredTopics) {
        sprintf(topic, "%s/%s", address.c_str(), t);
        mq.subscribe(topic);
        Serial.printf("Subscribed: %s\n", topic);
    }
}

//----------------------------------------

void unsubscribeTopics(String address) {
    char topic[32];
    for (auto t : requiredTopics) {
        sprintf(topic, "%s/%s", address.c_str(), t);
        mq.unsubscribe(topic);
        Serial.printf("Unsubscribed: %s\n", topic);
    }
}

//----------------------------------------

void startAcceptNewCube() {
    if (cubes.size() == MAX_CUBES) {
        Serial.printf("Cannot start accept new cube... (max: %d)\n", MAX_CUBES);
        return;
    }
    mq.subscribe("newcube");
    Serial.println("Start accept new cube");
}

void stopAcceptNewCube() {
    mq.unsubscribe("newcube");
    Serial.println("Stop accept new cube");
}

//----------------------------------------

void messageReceived(char *topic, byte *payload, unsigned int length) {
    Serial.print("Message received [");
    Serial.print(topic);
    Serial.print("] ");
    if (length < 128) {
        for (int i = 0; i < length; i++) {
            Serial.printf("%02X ", (char)payload[i]);
        }
    }
    Serial.println();

    String t(topic);
    if (t == "newcube") {
        char tmp[length + 1];
        memcpy(tmp, payload, length);
        tmp[length] = 0;
        String address(tmp);
        stopAcceptNewCube();
        connectToCube(address);
        startAcceptNewCube();
        return;
    }

    int i = t.indexOf('/');
    if (i == -1) {
        return;
    }
    String address(t.substring(0, i));
    t = t.substring(i + 1);
    Serial.printf("address=%s, topic=%s\n", address.c_str(), t.c_str());
    if (cubes.count(address) == 0) {
        return;
    }
    auto cube = cubes.at(address);
    if (t == "lamp") {
        cube->SetLamp(payload, length);
    }
}

//----------------------------------------

void connectToCube(String address) {
    if (NimBLEDevice::getClientListSize() >= MAX_CUBES) {
        Serial.println("Cannot connect no more cubes..");
        return;
    }

    if (cubes.count(address) > 0) {
        Serial.printf("Already connected to %s\n", address.c_str());
        return;
    }

    auto cube = std::make_shared<Cube>();
    if (!cube->connect(address, &clientCallback, notifyCallback)) {
        Serial.printf("Connect to %s failed\n", address.c_str());
        return;
    }

    subscribeTopics(address);

    char topic[32];
    sprintf(topic, "%s/connected", address.c_str());
    mq.publish(topic, "", true);
    Serial.printf("Published: %s\n", topic);

    cubes[address] = cube;
}
