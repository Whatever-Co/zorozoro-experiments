#include <bluefruit.h>

// #include <vector>

const uint8_t TOIO_SERVICE_UUID[] = {0xAE, 0xBB, 0xD7, 0xFC, 0x3E, 0xCF, 0x08, 0x95, 0x71, 0x45, 0x3B, 0x5B, 0x00, 0x01, 0xB2, 0x10};
const uint8_t TOIO_LAMP_CHARACTERISTIC_UUID[] = {0xAE, 0xBB, 0xD7, 0xFC, 0x3E, 0xCF, 0x08, 0x95, 0x71, 0x45, 0x3B, 0x5B, 0x03, 0x01, 0xB2, 0x10};

// BLEClientService toioService(TOIO_SERVICE_UUID);
// BLEClientCharacteristic toioLampCharacteristic(TOIO_LAMP_CHARACTERISTIC_UUID);

// BLEClientService* services[BLE_MAX_CONNECTION];
BLEClientCharacteristic* characteristics[BLE_MAX_CONNECTION] = {0};

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(10);

    Serial.println("Start...");

    Bluefruit.begin(0, BLE_MAX_CONNECTION);

    Bluefruit.setConnLedInterval(250);

    Bluefruit.Central.setConnectCallback(connect_callback);
    Bluefruit.Central.setDisconnectCallback(disconnect_callback);

    Bluefruit.Scanner.setRxCallback(scan_callback);
    Bluefruit.Scanner.restartOnDisconnect(true);
    Bluefruit.Scanner.filterUuid(TOIO_SERVICE_UUID);
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
        Serial.printf("now %\n", now);
        previousCall = now;
        for (auto ch : characteristics) {
            Serial.printf("ch %p\n", ch);
            if (ch) {
                ch->write_resp(lampData[count], 7);
            }
        }
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

    Serial.print("Discovering toio Service ... ");
    auto service = new BLEClientService(TOIO_SERVICE_UUID);
    service->begin();
    if (service->discover(conn_handle)) {
        Serial.println("Service Found");
        Serial.print("Discovering Lamp Characteristic ... ");
        auto characteristic = new BLEClientCharacteristic(TOIO_LAMP_CHARACTERISTIC_UUID);
        characteristic->begin();
        if (characteristic->discover()) {
            Serial.println("Characteristic Found");
            // uint8_t data[] = {0x04, 0x00, 0x04,
            //                   0x0a, 0x01, 0x01, 0xff, 0xff, 0x00,
            //                   0x0a, 0x01, 0x01, 0x00, 0xff, 0x00,
            //                   0x0a, 0x01, 0x01, 0x00, 0xff, 0xff,
            //                   0x0a, 0x01, 0x01, 0xff, 0x00, 0xff};
            uint8_t data[] = {0x03, 0x00, 0x01, 0x01, 0xff, 0xff, 0xff};
            characteristic->write_resp(data, sizeof(data));
            characteristics[conn_handle] = characteristic;
        } else {
            Serial.println("No Characteristic Found. Characteristic is mandatory but not found. ");
            Bluefruit.disconnect(conn_handle);
        }
    } else {
        Serial.println("No Service Found");
        Bluefruit.disconnect(conn_handle);
    }

    delay(100);
    // if (conn_handle < 3) {
    Bluefruit.Scanner.start(0);
    // }
}

void disconnect_callback(uint16_t conn_handle, uint8_t reason) {
    Serial.print("Disconnected, reason = 0x");
    Serial.println(reason, HEX);
}

/* Prints a hex list to the Serial Monitor */
void printHexList(uint8_t* buffer, uint8_t len) {
    // print forward order
    for (int i = 0; i < len; i++) {
        Serial.printf("%02X-", buffer[i]);
    }
    Serial.println();
}
