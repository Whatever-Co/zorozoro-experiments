#include <Ethernet2.h>
#include <MQTT.h>
#include <SPI.h>
#include <bluefruit.h>

//----------------------------------------

class Address {
   public:
    static String ToString(ble_gap_addr_t& addr) {
        char tmp[18] = {0};
        sprintf(tmp, "%02x:%02x:%02x:%02x:%02x:%02x", addr.addr[5], addr.addr[4], addr.addr[3], addr.addr[2], addr.addr[1], addr.addr[0]);
        return String(tmp);
    }
    static ble_gap_addr_t FromString(String str) {
        ble_gap_addr_t addr;
        const char* s = str.c_str();
        char tmp[3] = {0};
        for (int i = 0; i < 6; i++) {
            memcpy(tmp, s + (5 - i) * 3, 2);
            addr.addr[i] = strtol(tmp, NULL, 16);
        }
        addr.addr_id_peer = 0;
        addr.addr_type = BLE_GAP_ADDR_TYPE_RANDOM_STATIC;
        return addr;
    }
};

//----------------------------------------

// #define WAIT_SERIAL_CONNECTION 1

#define CONTROLLER_HOST "192.168.2.1"
#define CONTROLLER_PORT 1883

static EthernetClient ethernet;
static MQTTClient mqtt;
String ip_address_;

uint8_t toioServiceUUID[] = {0xAE, 0xBB, 0xD7, 0xFC, 0x3E, 0xCF, 0x08, 0x95, 0x71, 0x45, 0x3B, 0x5B, 0x00, 0x01, 0xB2, 0x10};

//----------------------------------------

void setup() {
    Serial.begin(115200);
#ifdef WAIT_SERIAL_CONNECTION
    while (!Serial) {
        delay(100);
    }
#endif

    Serial.println("Start...");
    Bluefruit.begin(0, 0);
    // Bluefruit.Central.setConnectCallback(OnConnect);
    // Bluefruit.Central.setDisconnectCallback(OnDisconnect);

    // Use bluetooth address as Ethernet MAC address...
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
    auto addr = Ethernet.localIP();
    char addr_str[32] = {0};
    sprintf(addr_str, "%d.%d.%d.%d", addr[0], addr[1], addr[2], addr[3]);
    ip_address_ = String(addr_str);
    Serial.println(ip_address_);

    mqtt.begin(CONTROLLER_HOST, ethernet);
    // mqtt.onMessageAdvanced(OnMessage);

    Bluefruit.Scanner.setRxCallback(scanCallback);
    Bluefruit.Scanner.restartOnDisconnect(true);
    Bluefruit.Scanner.filterUuid(toioServiceUUID);
    Bluefruit.Scanner.useActiveScan(false);
    Bluefruit.Scanner.setIntervalMS(1000, 1000);
    Bluefruit.Scanner.start(0);

    Serial.println("ready...");
}

void loop() {
    mqtt.loop();

    if (!mqtt.connected()) {
        while (!mqtt.connected()) {
            Serial.print("Attempting MQTT connection...");
            ethernet.stop();
            if (mqtt.connect(ip_address_.c_str())) {
                Serial.println("connected");
            } else {
                Serial.print("failed, try again in 3 seconds");
                delay(3000);
            }
        }
    }

    delay(100);
}

void scanCallback(ble_gap_evt_adv_report_t* report) {
    Serial.print("scanCallback: ");
    Serial.printBufferReverse(report->peer_addr.addr, 6, ':');
    Serial.println("");
    mqtt.publish("newcube", Address::ToString(report->peer_addr));
    delay(100);
    Bluefruit.Scanner.resume();
}
