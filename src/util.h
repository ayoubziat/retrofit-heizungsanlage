#ifndef UTIL_H
#define UTIL_H

#include <PubSubClient.h>
#include <WiFi.h>
#include<iostream> 
#include <list>

using namespace std;


struct mqttConfiguration{
    /* data */
    const char* ssid;
    const char* password;
    const char* mqtt_server;
    int mqtt_port;
    // const char* mqtt_publish_topic;
    // const char* mqtt_subscribe_topic;
    list<string> mqtt_publish_topics;
    list<string> mqtt_subscribe_topics;
};

class Communication {
    public:
        mqttConfiguration mqtt_config;
        Communication();
        // Communication(mqttConfiguration config, PubSubClient* client);
        void setup();
        void setup_wifi();
        void callback(char* topic, byte* message, unsigned int length);
        void reconnect(PubSubClient client);
        mqttConfiguration getMQTTConfig();
        PubSubClient* getPubSubClient();
};

#endif // UTIL_H