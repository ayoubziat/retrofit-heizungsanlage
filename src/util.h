#ifndef UTIL_H
#define UTIL_H

#include <FastLED.h>
#include <PubSubClient.h>
#include <ClosedCube_HDC1080.h>
#include <WiFi.h>
#include <iostream> 
#include <list>

using namespace std;

struct mqttConfiguration {
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
        mqttConfiguration comm_mqtt_config;
        Communication(mqttConfiguration config);
        void setup();
        void setup_wifi();
        void reconnect();
        mqttConfiguration getMQTTConfig();
        static void mqttCallback(char* topic, byte* message, unsigned int length);
        void loop();
};

// Function prototypes
extern void printSerialNumber();


// ------ Constants ------

// MQTT Configuration example
const struct mqttConfiguration MQTT_CONFIG_EXAMPLE = {
  "Honolulu",
  "$D9cA@1#46*Z%$O7%",
  "broker.hivemq.com",
  1883,
  {
    "de/heizungsanlage/data/temperature", 
    "de/heizungsanlage/data/humidity"
  },
  {"de/heizungsanlage/lightControl"}
};

// Lab@Home Pins
//#define TX_PIN 5
//#define RX_PIN 6

// esp32 NodeMCU
#define TX_PIN 10
#define RX_PIN 9

#define TIME_INTERVAL 5000

// LED Pin
constexpr int LED_PIN = 4;

constexpr int NUM_LEDS{4};
constexpr int DATA_PIN{15};

#endif // UTIL_H