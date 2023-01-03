#ifndef __UTIL_H
#define __UTIL_H

#include <PubSubClient.h>
#include <iostream> 
#include <list>

using namespace std;

struct mqttConfiguration {
    /* data */
    const char* ssid;
    const char* password;
    const char* mqtt_server;
    int mqtt_port;
    list<string> mqtt_publish_topics;
    list<string> mqtt_subscribe_topics;
};

struct dataPoint{
  uint32_t value;
  uint16_t address;
  uint8_t length;
  int16_t factor;
  String name;
};

class Communication {
    public:
        struct mqttConfiguration comm_mqtt_config;
        Communication(struct mqttConfiguration config);
        void setup();
        void setup_wifi();
        void reconnect();
        static void mqttCallback(char* topic, byte* message, unsigned int length);
        void loop();
        void publish(struct dataPoint point);
};

// Function prototypes
extern void printSerialNumber();

#endif // __UTIL_H