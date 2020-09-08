#include <Ethernet2.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <bluefruit.h>

#include "cube.h"
#include "cube_manager.h"
#include "hoge.h"

//----------------------------------------

#define WAIT_SERIAL_CONNECTION 1

#define CONTROLLER_HOST "192.168.2.1"
#define CONTROLLER_PORT 1883

#define MAX_CUBES (10)

static EthernetClient client;
static PubSubClient mq(client);

static const char *requiredTopics[] = {"motor", "lamp"};

//----------------------------------------

void App::Setup() {
    Serial.begin(115200);
#ifdef WAIT_SERIAL_CONNECTION
    while (!Serial) {
        delay(100);
    }
#endif

    Serial.println("Start...");
    Bluefruit.begin(0, MAX_CUBES);
    Bluefruit.Central.setConnectCallback(OnConnect);
    Bluefruit.Central.setDisconnectCallback(OnDisconnect);

    auto mac = Bluefruit.getAddr().addr;
    Serial.printf("Bluetooth address: %02X:%02X:%02X:%02X:%02X:%02X\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    Serial.println("Initialize Ethernet with DHCP:");
    if (Ethernet.begin(mac) == 0) {
        Serial.println("Failed to configure Ethernet using DHCP");
        delay(3000);
        while (true) {
        }
    }
    Serial.print("  DHCP assigned IP ");
    Serial.println(Ethernet.localIP());

    mq.setServer(CONTROLLER_HOST, CONTROLLER_PORT);
    mq.setCallback(OnMessage);

    Serial.println("ready...");
}

void App::Loop() {
    if (!mq.connected()) {
        Serial.println("Connecting to mqtt server");
        auto addr = Ethernet.localIP();
        char addr_str[32];
        sprintf(addr_str, "%d.%d.%d.%d", addr[0], addr[1], addr[2], addr[3]);
        while (!mq.connected()) {
            Serial.print("Attempting MQTT connection...");
            if (mq.connect(addr_str, "bye", 0, true, "")) {
                Serial.println("connected");
                mq.publish("hello", "", true);
                StartAcceptNewCube();
            } else {
                Serial.print("failed, rc=");
                Serial.print(mq.state());
                Serial.println(" try again in 5 seconds");
                delay(5000);
            }
        }
    }
    mq.loop();
    delay(10);
}

void App::OnBatteryInfo(BLEClientCharacteristic *chr, uint8_t *data, uint16_t length) {
    auto cube = CubeManager::GetCube(chr->connHandle());
    char topic[32];
    sprintf(topic, "%s/battery", cube->GetAddress().c_str());
    uint8_t value = data[0];
    mq.publish(topic, &value, 1);
    Serial.printf("Published: %s,%d\n", topic, value);
}

void App::OnMessage(char *topic, byte *payload, unsigned int length) {
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
        if (accept_new_cube_) {
            char tmp[length + 1];
            memcpy(tmp, payload, length);
            tmp[length] = 0;
            String address(tmp);
            StopAcceptNewCube();
            if (!ConnectToCube(address)) {
                StartAcceptNewCube();
            }
        }
        return;
    }

    int i = t.indexOf('/');
    if (i == -1) {
        return;
    }
    String address(t.substring(0, i));
    t = t.substring(i + 1);
    Serial.printf("address=%s, topic=%s\n", address.c_str(), t.c_str());
    auto cube = CubeManager::GetCube(address);
    if (!cube) {
        return;
    }
    if (t == "motor") {
        // cube->SetMotor(payload, length);
    } else if (t == "lamp") {
        cube->SetLamp(payload, length);
    }
}

void App::OnConnect(uint16_t conn_handle) {
    Serial.print("Connect Callback, conn_handle: ");
    Serial.println(conn_handle);

    auto cube = CubeManager::Setup(conn_handle);
    if (cube) {
        auto address = cube->GetAddress();
        SubscribeTopics(address);

        char topic[32];
        sprintf(topic, "%s/connected", address.c_str());
        mq.publish(topic, "", true);
        Serial.printf("Published: %s\n", topic);
    }

    StartAcceptNewCube();
}

void App::OnDisconnect(uint16_t conn_handle, uint8_t reason) {
    Serial.printf("Disconnected, %d, reason = 0x%02X\n", conn_handle, reason);

    auto address = CubeManager::GetAddress(conn_handle);

    UnsubscribeTopics(address);

    char topic[32];
    sprintf(topic, "%s/disconnected", address.c_str());
    mq.publish(topic, "", true);
    Serial.printf("Published: %s\n", topic);

    CubeManager::Cleanup(conn_handle);

    StartAcceptNewCube();
}

void App::StartAcceptNewCube() {
    if (CubeManager::GetNumCubes() == MAX_CUBES) {
        Serial.printf("Cannot start accept new cube... (max: %d)\n", MAX_CUBES);
        return;
    }
    mq.subscribe("newcube");
    Serial.println("Start accept new cube");
    accept_new_cube_ = true;
}

void App::StopAcceptNewCube() {
    accept_new_cube_ = false;
    mq.unsubscribe("newcube");
    Serial.println("Stop accept new cube");
}

bool App::ConnectToCube(String address) {
    Serial.println(address);
    if (CubeManager::GetCube(address)) {
        Serial.printf("Already connected to %s\n", address.c_str());
        return false;
    }
    auto addr = Address::FromString(address);
    Bluefruit.Central.connect(&addr);
    return true;
}

void App::SubscribeTopics(String address) {
    char topic[32];
    for (auto t : requiredTopics) {
        sprintf(topic, "%s/%s", address.c_str(), t);
        mq.subscribe(topic);
        Serial.printf("Subscribed: %s\n", topic);
    }
}

void App::UnsubscribeTopics(String address) {
    char topic[32];
    for (auto t : requiredTopics) {
        sprintf(topic, "%s/%s", address.c_str(), t);
        mq.unsubscribe(topic);
        Serial.printf("Unsubscribed: %s\n", topic);
    }
}

//----------------------------------------

void setup() {
    App::Setup();
}

void loop() {
    App::Loop();
}
