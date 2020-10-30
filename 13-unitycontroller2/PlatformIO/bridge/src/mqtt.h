#pragma once

#include <Arduino.h>
#include <Client.h>

typedef void (*MQTTClientCallback)(char topic[], char bytes[], int length);

uint8_t tmp[256];
uint8_t topic[1024];
uint8_t payload[1024];

uint8_t recvbuf[1024 * 16];
uint8_t *recvend = 0;

class MQTTClient {
   private:
    Client *netClient = nullptr;
    const char *hostname = nullptr;
    MQTTClientCallback callback;
    bool _connected = false;

   public:
    void begin(const char hostname[], Client &client) {
        this->hostname = strdup(hostname);
        this->netClient = &client;
    }

    void setCallback(MQTTClientCallback cb) {
        this->callback = cb;
    }

    bool publish(const char topic[], const char payload[], int length) {
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

    bool connect() {
        if (this->connected()) {
            this->close();
        }

        int ret = this->netClient->connect(this->hostname, 11111);
        if (ret <= 0) {
            return false;
        }

        this->_connected = true;

        return true;
    }

    bool loop() {
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
                this->callback((char *)topic, (char *)payload, len);
            }
            Serial.printf("%d, %d\n", p - recvbuf, recvend - recvbuf);
        }
        return true;
    }

    bool connected() {
        return this->netClient != nullptr && this->netClient->connected() == 1 && this->_connected;
    }

    void close() {
        this->_connected = false;
        this->netClient->stop();
    }
};
