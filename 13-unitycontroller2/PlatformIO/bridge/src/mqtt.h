#pragma once

#include <Arduino.h>
#include <Client.h>

class MQTTClient;

typedef void (*MQTTClientCallbackSimple)(String &topic, String &payload);
typedef void (*MQTTClientCallbackAdvanced)(MQTTClient *client, char topic[], char bytes[], int length);

typedef struct {
    MQTTClient *client = nullptr;
    MQTTClientCallbackSimple simple = nullptr;
    MQTTClientCallbackAdvanced advanced = nullptr;
} MQTTClientCallback;

uint8_t tmp[256];
uint8_t topic[1024];
uint8_t payload[1024];

uint8_t recvbuf[1024 * 16];
uint8_t *recvend = 0;

class MQTTClient {
   private:
    Client *netClient = nullptr;
    const char *hostname = nullptr;
    int port = 0;
    MQTTClientCallback callback;
    bool _connected = false;

   public:
    void begin(const char hostname[], Client &client) {
        this->hostname = strdup(hostname);
        this->netClient = &client;
        // this->netClient->setTimeout(10 * 1000);  // 10sec...
    }

    void onMessageAdvanced(MQTTClientCallbackAdvanced cb) {
        this->callback.client = this;
        this->callback.simple = nullptr;
        this->callback.advanced = cb;
    }

    bool publish(const String &topic) { return this->publish(topic.c_str(), ""); }
    bool publish(const char topic[]) { return this->publish(topic, ""); }
    bool publish(const String &topic, const String &payload) { return this->publish(topic.c_str(), payload.c_str()); }
    bool publish(const String &topic, const String &payload, bool retained, int qos) {
        return this->publish(topic.c_str(), payload.c_str(), retained, qos);
    }
    bool publish(const char topic[], const String &payload) { return this->publish(topic, payload.c_str()); }
    bool publish(const char topic[], const String &payload, bool retained, int qos) {
        return this->publish(topic, payload.c_str(), retained, qos);
    }
    bool publish(const char topic[], const char payload[]) {
        return this->publish(topic, (char *)payload, (int)strlen(payload));
    }
    bool publish(const char topic[], const char payload[], bool retained, int qos) {
        return this->publish(topic, (char *)payload, (int)strlen(payload), retained, qos);
    }
    bool publish(const char topic[], const char payload[], int length) {
        return this->publish(topic, payload, length, false, 0);
    }
    bool publish(const char topic[], const char payload[], int length, bool retained, int qos) {
        // return immediately if not connected
        if (!this->connected()) {
            return false;
        }

        size_t topiclen = strlen(topic);
        tmp[0] = (uint8_t)topiclen;
        memcpy(tmp + 1, topic, topiclen);
        tmp[topiclen + 1] = (uint8_t)length;
        if (length > 0) {
            memcpy(tmp + topiclen + 2, payload, length);
        }
        this->netClient->write(tmp, topiclen + length + 2);

        return true;
    }

    bool subscribe(const String &topic) { return this->subscribe(topic.c_str()); }
    bool subscribe(const String &topic, int qos) { return this->subscribe(topic.c_str(), qos); }
    bool subscribe(const char topic[]) { return this->subscribe(topic, 0); }
    bool subscribe(const char topic[], int qos) {
        // return immediately if not connected
        if (!this->connected()) {
            return false;
        }
        return true;
    }

    bool unsubscribe(const String &topic) { return this->unsubscribe(topic.c_str()); }
    bool unsubscribe(const char topic[]) {
        // return immediately if not connected
        if (!this->connected()) {
            return false;
        }
        return true;
    }

    bool connect(const char clientId[], bool skip = false) { return this->connect(clientId, nullptr, nullptr, skip); }
    bool connect(const char clientId[], const char username[], bool skip = false) {
        return this->connect(clientId, username, nullptr, skip);
    }
    bool connect(const char clientId[], const char username[], const char password[], bool skip) {
        // close left open connection if still connected
        if (!skip && this->connected()) {
            this->close();
        }

        // connect to host
        if (!skip) {
            int ret = this->netClient->connect(this->hostname, 11111);
            if (ret <= 0) {
                return false;
            }
        }

        // set flag
        this->_connected = true;

        return true;
    }

    bool loop() {
        // return immediately if not connected
        if (!this->connected()) {
            return false;
        }

        int available = this->netClient->available();
        if (available > 0) {
            Serial.printf("available: %d bytes\n", available);
            int readlen = this->netClient->read(recvbuf, min(available, sizeof(recvbuf)));
            Serial.printf("readlen: %d bytes\n", readlen);
            recvend = recvbuf + readlen;
            uint8_t *p = recvbuf;
            while (p < recvend) {
                int len = *p;
                p++;
                memcpy(topic, p, len);
                topic[len] = 0;
                p += len;
                Serial.printf("%d, %s\n", len, topic);
                len = *p;
                p++;
                memcpy(payload, p, len);
                p += len;
                Serial.printf("%d, ", len, payload);
                Serial.printBuffer(payload, len);
                Serial.println("");
                this->callback.advanced(this, (char *)topic, (char *)payload, len);
            }
            Serial.printf("%d, %d\n", p - recvbuf, recvend - recvbuf);
        }

        // while (this->netClient->available() > 0) {
        //     Serial.printf("available: %d\n", this->netClient->available());
        //     int topicLen = this->netClient->read();
        //     this->netClient->readBytes(topic, topicLen);
        //     topic[topicLen] = 0;
        //     int payloadLen = this->netClient->read();
        //     this->netClient->readBytes(payload, payloadLen);
        //     payload[payloadLen] = 0;
        //     Serial.printf("%d, %s\n", topicLen, topic);
        //     Serial.printf("%d, ", payloadLen, payload);
        //     Serial.printBuffer(payload, payloadLen);
        //     Serial.println("");
        //     this->callback.advanced(this, (char *)topic, (char *)payload, payloadLen);
        // }

        return true;
    }

    bool connected() {
        // a client is connected if the network is connected, a client is available and
        // the connection has been properly initiated
        return this->netClient != nullptr && this->netClient->connected() == 1 && this->_connected;
    }

    void close() {
        // set flag
        this->_connected = false;

        // close network
        this->netClient->stop();
    }
};
