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
    public:
        mqttConfiguration mqtt_config;
        Communication();
        // Communication(mqttConfiguration config, PubSubClient* client);
        void setup_wifi();
        void callback(char* topic, byte* message, unsigned int length);
        void reconnect(PubSubClient client);
        mqttConfiguration getMQTTConfig();
        PubSubClient* getPubSubClient();
};

#endif // UTIL_H