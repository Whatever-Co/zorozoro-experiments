#include <Ethernet2.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <bluefruit.h>

#include "cube.h"

//----------------------------------------

#define MAX_CUBES (10)

static EthernetClient client;
static PubSubClient mq(client);

//----------------------------------------

void startAcceptNewCube() {
    if (Cube::getNumCubes() == MAX_CUBES) {
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

    // int i = t.indexOf('/');
    // if (i == -1)
    // {
    //     return;
    // }
    // String address(t.substring(0, i));
    // t = t.substring(i + 1);
    // Serial.printf("address=%s, topic=%s\n", address.c_str(), t.c_str());
    // if (cubes.count(address) == 0)
    // {
    //     return;
    // }
    // auto cube = cubes.at(address);
    // if (t == "motor")
    // {
    //     cube->SetMotor(payload, length);
    // }
    // else if (t == "lamp")
    // {
    //     cube->SetLamp(payload, length);
    // }
}

//----------------------------------------

// ble_gap_addr_t convertToAddr(const char *s) {
//     ble_gap_addr_t addr;
//     char tmp[3] = {0};
//     for (int i = 0; i < 6; i++) {
//         memcpy(tmp, s + (5 - i) * 3, 2);
//         addr.addr[i] = strtol(tmp, NULL, 16);
//     }
//     addr.addr_id_peer = 0;
//     addr.addr_type = BLE_GAP_ADDR_TYPE_RANDOM_STATIC;
//     return addr;
// }

void connectToCube(String address) {
    Serial.println(address);

    // auto addr = convertToAddr(address.c_str());
    auto addr = Address::FromString(address);
    printHexList(addr.addr, BLE_GAP_ADDR_LEN);

    if (Cube::GetCube(addr)) {
        Serial.printf("Already connected to %s\n", address.c_str());
        return;
    }

    Bluefruit.Central.connect(&addr);

    // auto cube = std::make_shared<Cube>();
    // if (!cube->connect(address, &clientCallback, notifyCallback))
    // {
    //     Serial.printf("Connect to %s failed\n", address.c_str());
    //     return;
    // }

    // // subscribeTopics(address);

    // char topic[32];
    // sprintf(topic, "%s/connected", address.c_str());
    // mq.publish(topic, "", true);
    // Serial.printf("Published: %s\n", topic);

    // cubes[address] = cube;
}

//----------------------------------------

void connect_callback(uint16_t conn_handle) {
    Serial.println("");
    Serial.print("Connect Callback, conn_handle: ");
    Serial.println(conn_handle);

    Cube::NewCube(conn_handle);
}

//----------------------------------------

void disconnect_callback(uint16_t conn_handle, uint8_t reason) {
    Serial.printf("Disconnected, %d, reason = 0x%02X\n", conn_handle, reason);

    Cube::DeleteCube(conn_handle);
}

//----------------------------------------

void scan_callback(ble_gap_evt_adv_report_t *report) {
    Serial.println("\nscan_callback");
    auto addr = report->peer_addr;
    printHexList(report->peer_addr.addr, BLE_GAP_ADDR_LEN);
    Serial.printf("%d, %d\n", addr.addr_id_peer, addr.addr_type);

    Serial.println("Connecting to Peripheral ... ");
    Bluefruit.Central.connect(&report->peer_addr);
}

//----------------------------------------

void setup() {
    Serial.begin(115200);
    while (!Serial)
        delay(100);

    Serial.println("Start...");
    Bluefruit.begin(0, MAX_CUBES);
    Bluefruit.Central.setConnectCallback(connect_callback);
    Bluefruit.Central.setDisconnectCallback(disconnect_callback);

    auto mac = Bluefruit.getAddr().addr;
    Serial.printf("Bluetooth address: %02X:%02X:%02X:%02X:%02X:%02X\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    // start the Ethernet connection:
    Serial.println("Initialize Ethernet with DHCP:");
    if (Ethernet.begin(mac) == 0) {
        Serial.println("Failed to configure Ethernet using DHCP");
        while (true) {
            delay(1000);  // do nothing, no point running without Ethernet hardware
        }
    }
    Serial.print("  DHCP assigned IP ");
    Serial.println(Ethernet.localIP());

    mq.setServer("192.168.2.1", 1883);
    mq.setCallback(messageReceived);

    // Bluefruit.Scanner.setRxCallback(scan_callback);
    // Bluefruit.Scanner.restartOnDisconnect(true);
    // Bluefruit.Scanner.filterUuid(Cube::ServiceUUID);
    // Bluefruit.Scanner.useActiveScan(true);
    // Bluefruit.Scanner.start(0);

    Serial.println("ready...");
}

//----------------------------------------

void loop() {
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
                startAcceptNewCube();
            } else {
                Serial.print("failed, rc=");
                Serial.print(mq.state());
                Serial.println(" try again in 5 seconds");
                delay(5000);
            }
        }
    }
    mq.loop();
    delay(100);
}

//----------------------------------------

/* Prints a hex list to the Serial Monitor */
void printHexList(uint8_t *buffer, uint8_t len) {
    // print forward order
    for (int i = 0; i < len; i++) {
        Serial.printf("%02X-", buffer[i]);
    }
    Serial.println();
}
