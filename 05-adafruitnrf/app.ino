#include <bluefruit.h>

#include <memory>

#include "cube.h"

#define MAX_CUBES (10)

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(100);

    Serial.println("Start...");

    // Bluefruit.configAttrTableSize(1024 * 12);
    // Bluefruit.configUuid128Count(12);
    // Bluefruit.configCentralBandwidth(BANDWIDTH_HIGH);
    // Bluefruit.configCentralConn(50, 60, 20, 20);
    // Bluefruit.configPrphConn(BLE_GATT_ATT_MTU_DEFAULT, BLE_GAP_EVENT_LENGTH_DEFAULT * 2, BLE_GATTS_HVN_TX_QUEUE_SIZE_DEFAULT * 2, BLE_GATTC_WRITE_CMD_TX_QUEUE_SIZE_DEFAULT * 2);
    Bluefruit.begin(0, MAX_CUBES);

    // Bluefruit.setConnLedInterval(250);

    Bluefruit.Central.setConnectCallback(connect_callback);
    Bluefruit.Central.setDisconnectCallback(disconnect_callback);

    Bluefruit.Scanner.setRxCallback(scan_callback);
    Bluefruit.Scanner.restartOnDisconnect(true);
    Bluefruit.Scanner.filterUuid(Cube::ServiceUUID);
    Bluefruit.Scanner.useActiveScan(true);
    Bluefruit.Scanner.start(0);

    Serial.println("ready...");
}

static int previousCall = 0;

void loop() {
    int now = millis() / 1000;
    if (now - previousCall >= 1) {
        Serial.printf("now %d\n", now);
        previousCall = now;
    }
    Cube::UpdateAll();
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

    Cube::NewCube(conn_handle);

    delay(100);
    if (Cube::getNumCubes() < MAX_CUBES) {
        Bluefruit.Scanner.start(0);
    }
}

void disconnect_callback(uint16_t conn_handle, uint8_t reason) {
    Serial.printf("Disconnected, %d, reason = 0x%02X\n", conn_handle, reason);

    Cube::DeleteCube(conn_handle);

    delay(100);
    if (Cube::getNumCubes() < MAX_CUBES) {
        Bluefruit.Scanner.start(0);
    }
}

/* Prints a hex list to the Serial Monitor */
void printHexList(uint8_t* buffer, uint8_t len) {
    // print forward order
    for (int i = 0; i < len; i++) {
        Serial.printf("%02X-", buffer[i]);
    }
    Serial.println();
}
