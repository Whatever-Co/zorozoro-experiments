#include <Arduino.h>
#include <NimBLEDevice.h>
#include <WiFi.h>

const char *ssid = "WHEREVER";
const char *password = "0364276022";
const char *controllerHost = "10.0.0.96";
const int controllerPort = 12322;

static WiFiClient client;

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
}

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
            }
        }
        delay(10);
    }
    client.stop();
    Serial.println("Client disconnected");
    delay(5000);
}
