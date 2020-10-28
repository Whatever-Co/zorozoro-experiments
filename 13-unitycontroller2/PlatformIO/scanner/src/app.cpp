#include <Ethernet2.h>
#include <bluefruit.h>
#include <map>

//----------------------------------------

class Address
{
public:
    static String ToString(ble_gap_addr_t &addr)
    {
        char tmp[18] = {0};
        sprintf(tmp, "%02x:%02x:%02x:%02x:%02x:%02x", addr.addr[5], addr.addr[4], addr.addr[3], addr.addr[2], addr.addr[1], addr.addr[0]);
        return String(tmp);
    }
    static ble_gap_addr_t FromString(String str)
    {
        ble_gap_addr_t addr;
        const char *s = str.c_str();
        char tmp[3] = {0};
        for (int i = 0; i < 6; i++)
        {
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

EthernetClient ethernet;
// static MQTTClient mqtt;
String ip_address_;
uint8_t tmp[256];

uint8_t toioServiceUUID[] = {0xAE, 0xBB, 0xD7, 0xFC, 0x3E, 0xCF, 0x08, 0x95, 0x71, 0x45, 0x3B, 0x5B, 0x00, 0x01, 0xB2, 0x10};

std::map<std::string, uint32_t> lastSentTime;

//----------------------------------------

void scanCallback(ble_gap_evt_adv_report_t *report)
{
    auto address = Address::ToString(report->peer_addr);
    std::string key = address.c_str();
    auto dt = millis() - lastSentTime[key];
    // Serial.printf("%s: dt=%d\n", address.c_str(), dt);
    if (dt > 3000)
    {
        Serial.printf("scanCallback: %s\n", address.c_str());

        auto topic = "newcube";
        auto payload = address.c_str();
        int length = 17;

        size_t topiclen = strlen(topic);
        tmp[0] = (uint8_t)topiclen;
        memcpy(tmp + 1, topic, topiclen);
        tmp[topiclen + 1] = (uint8_t)length;
        memcpy(tmp + topiclen + 2, payload, length);
        ethernet.write(tmp, topiclen + length + 2);

        // Serial.printBuffer(tmp, topiclen + length + 2, ' ', 16);
        // Serial.print('\n');

        lastSentTime[key] = millis();
    }

    Bluefruit.Scanner.resume();
}

//----------------------------------------

void setup()
{
    Serial.begin(115200);
#ifdef WAIT_SERIAL_CONNECTION
    while (!Serial)
    {
        delay(100);
    }
#endif

    ledOn(LED_RED);

    Serial.println("Start...");
    Bluefruit.begin(0, 0);

    // Use bluetooth address as Ethernet MAC address...
    auto mac = Bluefruit.getAddr().addr;
    Serial.print("Bluetooth address: ");
    Serial.printBuffer(mac, 6, ':');
    Serial.println("");

    Serial.println("Initialize Ethernet with DHCP:");
    if (Ethernet.begin(mac) == 0)
    {
        Serial.println("Failed to configure Ethernet using DHCP");
        delay(3000);
        while (true)
        {
        }
    }
    Serial.print("  DHCP assigned IP ");
    auto addr = Ethernet.localIP();
    char addr_str[32] = {0};
    sprintf(addr_str, "%d.%d.%d.%d", addr[0], addr[1], addr[2], addr[3]);
    ip_address_ = String(addr_str);
    Serial.println(ip_address_);

    // mqtt.begin(CONTROLLER_HOST, ethernet);
    // mqtt.onMessageAdvanced(OnMessage);

    Bluefruit.Scanner.setRxCallback(scanCallback);
    Bluefruit.Scanner.restartOnDisconnect(true);
    Bluefruit.Scanner.filterUuid(toioServiceUUID);
    Bluefruit.Scanner.useActiveScan(false);
    Bluefruit.Scanner.setIntervalMS(1000, 1000);
    Bluefruit.Scanner.start(0);

    Serial.println("ready...");
}

void loop()
{
    // mqtt.loop();

    if (!ethernet.connected())
    {
        ledOn(LED_RED);
        while (!ethernet.connected())
        {
            ethernet.stop();
            Serial.print("Attempting connection...");
            int ret = ethernet.connect(CONTROLLER_HOST, 11122);
            if (ret > 0)
            {
                Serial.println("connected");
            }
            else
            {
                Serial.println("failed, try again in 3 seconds");
                delay(3000);
            }
        }
        ledOff(LED_RED);
    }

    delay(10);
}
