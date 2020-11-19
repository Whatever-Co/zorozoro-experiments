#include "app.h"

#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include <Chrono.h>
#include <Ethernet.h>
#include <SPI.h>
#include <bluefruit.h>

#include "SEGGER_RTT.h"
#include "cube.h"
#include "cube_manager.h"
#include "mqtt.h"

//----------------------------------------

// #define WAIT_SERIAL_CONNECTION 1

#define CONTROLLER_HOST "192.168.2.1"
#define CONTROLLER_PORT 1883

#define MAX_CUBES (10)

static EthernetClient ethernet;
static MQTTClient mqtt;

static Adafruit_NeoPixel pixels(1, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);
static uint32_t colors[MAX_CUBES + 1] = {0xff0000, 0xf7215b, 0x15f8c2, 0xf7e411, 0xa312f7,
                                         0x4970ff, 0xfc8c00, 0x2ee236, 0xf954d7, 0xa7f9ff, 0x000000};

static Chrono myChrono;

//----------------------------------------

String App::mac_address_;
String App::ip_address_;
bool App::accept_new_cube_ = false;
bool App::connecting_ = false;

//----------------------------------------

void blinkTask(void *pvParameters) {
    while (1) {
        while (mqtt.connected()) {
            digitalWrite(LED_RED, HIGH);
            vTaskDelay(50);
            digitalWrite(LED_RED, LOW);
            vTaskDelay(950);
        }
        vTaskDelay(3000);
    }
}

void App::Setup() {
    // SEGGER_RTT_Init();
    // SEGGER_RTT_printf(0, "Hello world\n");

    Serial.begin(115200);
#ifdef WAIT_SERIAL_CONNECTION
    while (!Serial) {
        delay(100);
    }
#endif

    ledOn(LED_RED);
    pixels.begin();

    Serial.println("Start...");
    // Bluefruit.configUuid128Count(32);
    // Bluefruit.configAttrTableSize(0x3000);
    Bluefruit.begin(0, MAX_CUBES);
    Bluefruit.setTxPower(8);
    Bluefruit.Central.setConnectCallback(OnConnect);
    Bluefruit.Central.setDisconnectCallback(OnDisconnect);

    // Use bluetooth address as Ethernet MAC address...
    auto bladdr = Bluefruit.getAddr();
    auto mac = &(bladdr.addr[0]);
    {
        char addr_str[32];
        sprintf(addr_str, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        mac_address_ = String(addr_str);
        Serial.printf("Bluetooth address: %s\n", mac_address_.c_str());
    }

    Serial.println("Initialize Ethernet with DHCP:");
    if (Ethernet.begin(mac) == 0) {
        Serial.println("Failed to configure Ethernet using DHCP");
        while (true) {
            digitalToggle(LED_RED);
            delay(300);
        }
    }
    Serial.print("  DHCP assigned IP ");
    // Serial.println(Ethernet.localIP());
    {
        auto addr = Ethernet.localIP();
        char addr_str[32];
        sprintf(addr_str, "%d.%d.%d.%d", addr[0], addr[1], addr[2], addr[3]);
        ip_address_ = String(addr_str);
        Serial.println(ip_address_);
    }

    mqtt.begin(CONTROLLER_HOST, ethernet);
    mqtt.setCallback(OnMessage);

    Serial.println("ready...");

    xTaskCreate(blinkTask, "blinkTask", 1024, NULL, 1, NULL);

    UpdateStatusLED();
}

void App::Loop() {
    mqtt.loop();

    if (!mqtt.connected()) {
        ledOn(LED_RED);
        Serial.printf("disconnected? %d, %d\n", ethernet.connected(), ethernet.available());
        while (!mqtt.connected()) {
            Serial.print("Attempting connection...");
            ethernet.stop();
            if (mqtt.connect()) {
                Serial.println("connected");
                for (const auto &address : CubeManager::GetAddresses()) {
                    char topic[32];
                    sprintf(topic, "%s/connected", address.c_str());
                    mqtt.publish(topic, nullptr, 0);
                    Serial.printf("Published: %s\n", topic);
                }
                StartAcceptNewCube();
            } else {
                Serial.println("failed, try again in 3 seconds");
                delay(3000);
            }
        }
        ledOff(LED_RED);
    }

    if (!connecting_ && myChrono.hasPassed(3000)) {
        StartAcceptNewCube();
    }

    delay(1);
}

void App::OnMotor(BLEClientCharacteristic *chr, uint8_t *data, uint16_t length) {
    Serial.print("OnMotor: ");
    for (int i = 0; i < length; i++) {
        Serial.printf("%02X ", (char)data[i]);
    }
    Serial.println();
}

void App::OnIdInfo(BLEClientCharacteristic *chr, uint8_t *data, uint16_t length) {
    auto cube = CubeManager::GetCube(chr->connHandle());
    char topic[32];
    sprintf(topic, "%s/position", cube->GetAddress().c_str());
    mqtt.publish(topic, (char *)data, length);
    // Serial.printf("Published: %dbytes\n", length);
}

void App::OnBatteryInfo(BLEClientCharacteristic *chr, uint8_t *data, uint16_t length) {
    auto cube = CubeManager::GetCube(chr->connHandle());
    char topic[32];
    sprintf(topic, "%s/battery", cube->GetAddress().c_str());
    char value = data[0];
    mqtt.publish(topic, &value, 1);
    Serial.printf("Published: %s,%d\n", topic, value);
}

void App::OnMessage(char topic[], char payload[], int length) {
    // Serial.print("Message received [");
    // Serial.print(topic);
    // Serial.print("] ");
    // if (length <= 32) {
    //     for (int i = 0; i < length; i++) {
    //         Serial.printf("%02X ", (char)payload[i]);
    //     }
    //     Serial.println();
    // } else {
    //     Serial.printf("\nPayload is too long: %d bytes... (or something wrong...?\n", length);
    //     return;
    // }

    String t(topic);
    int i = t.indexOf('/');
    if (i == -1) {
        return;
    }
    String address(t.substring(0, i));
    t = t.substring(i + 1);
    // Serial.printf("address=%s, topic=%s\n", address.c_str(), t.c_str());
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
    auto cube = CubeManager::GetCube(address);
    if (!cube) {
        return;
    }
    if (t == "motor") {
        cube->SetMotor((uint8_t *)payload, length);
    } else if (t == "lamp") {
        cube->SetLamp((uint8_t *)payload, length);
    }
}

void App::OnConnect(uint16_t conn_handle) {
    Serial.print("Connect Callback, conn_handle: ");
    Serial.println(conn_handle);

    auto cube = CubeManager::Setup(conn_handle);
    if (cube) {
        auto address = cube->GetAddress();
        char topic[32];
        sprintf(topic, "%s/connected", address.c_str());
        mqtt.publish(topic, nullptr, 0);
        Serial.printf("Published: %s\n", topic);
    }

    connecting_ = false;

    StartAcceptNewCube();
    UpdateStatusLED();
}

void App::OnDisconnect(uint16_t conn_handle, uint8_t reason) {
    Serial.printf("Disconnected, %d, reason = 0x%02X\n", conn_handle, reason);

    if (reason != 0x3e) {
        auto address = CubeManager::GetAddress(conn_handle);
        char topic[32];
        sprintf(topic, "%s/disconnected", address.c_str());
        mqtt.publish(topic, nullptr, 0);
        Serial.printf("Published: %s\n", topic);
    }

    CubeManager::Cleanup(conn_handle);

    StartAcceptNewCube();
    UpdateStatusLED();
}

void App::StartAcceptNewCube() {
    // if (CubeManager::GetNumCubes() == MAX_CUBES) {
    //     // Serial.printf("Cannot start accept new cube... (max: %d)\n", MAX_CUBES);
    //     return;
    // }
    auto topic = mac_address_ + "/available";
    char available = MAX_CUBES - CubeManager::GetNumCubes();
    mqtt.publish(topic.c_str(), &available, 1);
    accept_new_cube_ = CubeManager::GetNumCubes() < MAX_CUBES;
    myChrono.restart();
    Serial.println("Start accept new cube");
}

void App::StopAcceptNewCube() {
    accept_new_cube_ = false;
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
    connecting_ = true;
    return true;
}

void App::UpdateStatusLED() {
    pixels.setPixelColor(0, colors[CubeManager::GetNumCubes()]);
    pixels.setBrightness(8);
    pixels.show();
}

//----------------------------------------

void setup() {
    App::Setup();
}

void loop() {
    App::Loop();
}
