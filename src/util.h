#ifndef UTIL_H
#define UTIL_H

#include <PubSubClient.h>
#include <WiFi.h>

struct mqttConfiguration{
    /* data */
    const char* ssid;
    const char* password;
    const char* mqtt_server;
    const char* mqtt_topic;
};

class Communication {
    private:
        mqttConfiguration mqtt_config;
    public:
        Communication(mqttConfiguration config);
        void setup_wifi();
        void callback(char* topic, byte* message, unsigned int length);
};

#endif // UTIL_H