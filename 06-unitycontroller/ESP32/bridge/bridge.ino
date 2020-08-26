#include <Arduino.h>
#include <NimBLEDevice.h>
#include <WiFi.h>

//----------------------------------------

const char *ssid = "WHEREVER";
const char *password = "0364276022";
const char *controllerHost = "10.0.0.96";
const int controllerPort = 12322;

static WiFiClient client;

//----------------------------------------

#define MAX_CUBES 8

static NimBLEUUID serviceUUID("10B20100-5B3B-4571-9508-CF3EFCD7BBAE");
static NimBLEUUID lampCharUUID("10B20103-5B3B-4571-9508-CF3EFCD7BBAE");
static NimBLEUUID batteryCharUUID("10B20108-5B3B-4571-9508-CF3EFCD7BBAE");

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

class Cube {
   public:
    Cube()
        : client(nullptr),
          service(nullptr),
          lamp(nullptr),
          battery(nullptr) {}

    ~Cube() {
        disconnect();
    }

    bool connect(String address) {
        Serial.print("Forming a connection to ");
        Serial.println(address);
        client = NimBLEDevice::createClient();
        client->setClientCallbacks(&clientCallback, false);
        Serial.println(" - Created client");
        client->connect(NimBLEAddress(address.c_str(), BLE_ADDR_RANDOM));
        auto service = client->getService(serviceUUID);
        if (service == nullptr) {
            Serial.print("Failed to find our service UUID: ");
            Serial.println(serviceUUID.toString().c_str());
            disconnect();
            return false;
        }
        Serial.println(" - Found our service");

#define GET_CHARACTERISTIC(x)                                     \
    x = service->getCharacteristic(x##CharUUID);                  \
    if (x == nullptr) {                                           \
        Serial.print("Failed to find our characteristic UUID: "); \
        Serial.println(x##CharUUID.toString().c_str());           \
        disconnect();                                             \
        return false;                                             \
    }                                                             \
    Serial.println(" - Found our characteristic " #x);

        GET_CHARACTERISTIC(lamp);
        GET_CHARACTERISTIC(battery);

        // uint8_t data[] = {0x04, 0x01, 0x04,
        //                   0x10, 0x01, 0x01, 0xff, 0xff, 0x00,
        //                   0x10, 0x01, 0x01, 0x00, 0xff, 0x00,
        //                   0x10, 0x01, 0x01, 0x00, 0xff, 0xff,
        //                   0x10, 0x01, 0x01, 0xff, 0x00, 0xff};
        // lamp->writeValue(data, sizeof(data), true);
        return true;
    }

    void disconnect() {
        if (client != nullptr) {
            client->disconnect();
            NimBLEDevice::deleteClient(client);
            client = nullptr;
            service = nullptr;
            lamp = nullptr;
            battery = nullptr;
        }
    }

    NimBLEClient *getClient() {
        return client;
    }

    String getAddress() {
        if (client == nullptr) {
            return String("");
        }
        return String(client->getPeerAddress().toString().c_str());
    }

   private:
    NimBLEClient *client;
    NimBLEService *service;
    NimBLERemoteCharacteristic *lamp, *battery;
};

static Cube *cubes[MAX_CUBES] = {nullptr};

//----------------------------------------

void setup() {
    Serial.begin(115200);
    delay(10);

    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
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

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    NimBLEDevice::init("");
}

//----------------------------------------

void loop() {
    if (!client.connect(controllerHost, controllerPort)) {
        Serial.println("connection failed");
        delay(5000);
        return;
    }
    client.printf("hello\tbridge\t%s\n", WiFi.localIP().toString().c_str());
    while (client.connected()) {
        while (client.available() > 0) {
            auto data = client.readStringUntil('\n');
            Serial.println(data);
            int i = data.indexOf('\t');
            if (i >= 0) {
                auto command = data.substring(0, i);
                auto arg = data.substring(i + 1);
                Serial.printf("command = \"%s\", arg = \"%s\"\n", command.c_str(), arg.c_str());
                if (command == "connect") {
                    connect(arg);
                }
            }
        }
        reportConnected(false);
        delay(10);
    }
    client.stop();
    Serial.println("Client disconnected");
    delay(5000);
}

//----------------------------------------

void connect(String address) {
    if (NimBLEDevice::getClientListSize() >= MAX_CUBES) {
        Serial.println("cannot connect no more cubes..");
        return;
    }

    auto cube = new Cube();
    if (!cube->connect(address)) {
        delete cube;
        Serial.println("connect failed");
        return;
    }
    Serial.println("connected");
    for (int i = 0; i < MAX_CUBES; i++) {
        if (cubes[i] == nullptr) {
            cubes[i] = cube;
            break;
        }
    }
    reportConnected(true);
}

//----------------------------------------

void disconnect(NimBLEClient *client) {
    for (int i = 0; i < MAX_CUBES; i++) {
        if (cubes[i] != nullptr && cubes[i]->getClient() == client) {
            delete cubes[i];
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
        Serial.println(list);
        client.println(list);
        prevReportTime = millis();
    }
}
