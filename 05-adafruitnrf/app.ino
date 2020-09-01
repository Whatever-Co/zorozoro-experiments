#include <bluefruit.h>

#include <memory>

#include "cube.h"

static std::shared_ptr<Cube> cubes[BLE_MAX_CONNECTION] = {nullptr};

void setup() {
    Serial.begin(115200);
    // while (!Serial) delay(10);

    Serial.println("Start...");

    Bluefruit.begin(0, BLE_MAX_CONNECTION);

    Bluefruit.setConnLedInterval(250);

    Bluefruit.Central.setConnectCallback(connect_callback);
    Bluefruit.Central.setDisconnectCallback(disconnect_callback);

    Bluefruit.Scanner.setRxCallback(scan_callback);
    Bluefruit.Scanner.restartOnDisconnect(true);
    Bluefruit.Scanner.filterUuid(Cube::SERVICE_UUID);
    Bluefruit.Scanner.useActiveScan(true);
    Bluefruit.Scanner.start(0);

    Serial.println("ready...");
}

static int previousCall = 0;
static int count = 0;
static uint8_t lampData[3][7] = {
    {0x03, 0x00, 0x01, 0x01, 0x00, 0xff, 0xff},
    {0x03, 0x00, 0x01, 0x01, 0xff, 0x00, 0xff},
    {0x03, 0x00, 0x01, 0x01, 0xff, 0xff, 0x00},
};

void loop() {
    int now = millis() / 1000;
    if (now - previousCall >= 2) {
        Serial.printf("now %d\n", now);
        previousCall = now;
        if (++count == 3) {
            count = 0;
        }
    }
}

void scan_callback(ble_gap_evt_adv_report_t* report) {
    Serial.println("\nscan_callback");
    printHexList(report->peer_addr.addr, BLE_GAP_ADDR_LEN);

    Serial.println("Connecting to Peripheral ... ");
    Bluefruit.Central.connect(report);
}

void connect_callback(uint16_t conn_handle) {
    Serial.println("");
    Serial.print("Connect Callback, conn_handle: ");
    Serial.println(conn_handle);

    auto cube = std::make_shared<Cube>();
    cube->setup(conn_handle);
    cubes[conn_handle] = cube;

    delay(100);
    // if (conn_handle < 3) {
    Bluefruit.Scanner.start(0);
    // }
}

void disconnect_callback(uint16_t conn_handle, uint8_t reason) {
    Serial.printf("Disconnected, %d, reason = 0x%02X\n", conn_handle, reason);
    Serial.printf("%p\n", cubes[conn_handle]);
    cubes[conn_handle] = nullptr;
    Serial.println("done.");
}

/* Prints a hex list to the Serial Monitor */
void printHexList(uint8_t* buffer, uint8_t len) {
    // print forward order
    for (int i = 0; i < len; i++) {
        Serial.printf("%02X-", buffer[i]);
    }
    Serial.println();
}
