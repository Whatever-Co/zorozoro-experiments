#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include <Arduino.h>
#include <WiFi.h>

//----------------------------------------

const char *ssid = "WHEREVER";
const char *password = "0364276022";
const char *controllerHost = "10.0.0.96";
const int controllerPort = 1883;

WiFiClient client;

Adafruit_MQTT_Client mqtt(&client, controllerHost, controllerPort, "clientdd", "user", "pass");
Adafruit_MQTT_Publish newcube = Adafruit_MQTT_Publish(&mqtt, "newcube");

//----------------------------------------

void setup() {
    Serial.begin(115200);
    delay(10);

    Serial.print("\n\nConnecting to ");
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

    Serial.println("\nWiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

//----------------------------------------

void loop() {
    if (!mqtt.connected()) {
        MQTT_connect();
    }

    int x = millis() / 2000;
    Serial.print(F("\nSending photocell val "));
    Serial.print(x);
    Serial.print("...");
    if (!newcube.publish(x++)) {
        Serial.println(F("Failed"));
    } else {
        Serial.println(F("OK!"));
    }

    delay(2000);
}

//----------------------------------------

void MQTT_connect() {
    Serial.print("Connecting to MQTT... ");
    int8_t ret;
    // uint8_t retries = 3;
    while ((ret = mqtt.connect()) != 0) {  // connect will return 0 for connected
        Serial.println(mqtt.connectErrorString(ret));
        Serial.println("Retrying MQTT connection in 5 seconds...");
        mqtt.disconnect();
        delay(5000);  // wait 5 seconds
        // retries--;
        // if (retries == 0) {
        //     // basically die and wait for WDT to reset me
        //     while (1)
        //         ;
        // }
    }
    Serial.println("MQTT Connected!");
}
