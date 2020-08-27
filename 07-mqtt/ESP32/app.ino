#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>

//----------------------------------------

const char *ssid = "WHEREVER";
const char *password = "0364276022";
// const char *controllerHost = "10.0.0.96";
// const int controllerPort = 12322;
const char *mqtt_server = "10.0.0.96";

WiFiClient wifiClient;
PubSubClient client(wifiClient);

char mTopic[] = "drone/001";
long mCount = 0;
long lastMsg = 0;

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

    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
}

//----------------------------------------

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();
    //pub
    long now = millis();
    if ((now - lastMsg) > 2000) {
        lastMsg = now;
        ++mCount;
        char msg[100];
        snprintf(msg, 75, "%ld", mCount);
        Serial.print("Publish message: ");
        Serial.println(msg);
        client.publish(mTopic, msg);
    }
}

//----------------------------------------

void reconnect() {
    // Loop until we're reconnected
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        if (client.connect("arduinoClient")) {
            Serial.println("connected");
            //client.publish("outTopic","hello world");
            client.subscribe("inTopic");
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

//----------------------------------------

void callback(char *topic, byte *payload, unsigned int length) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();
}
