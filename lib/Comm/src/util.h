#ifndef __UTIL_H
#define __UTIL_H

#include <FastLED.h>
#include <PubSubClient.h>
#include <WiFiMulti.h>
#include <ClosedCube_HDC1080.h>
#include <iostream> 
#include <list>
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>
#include "optolink.h"
#include "credentials.h"
#include "dht_sensor.h"

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

class Communication {
    public:
        struct mqttConfiguration comm_mqtt_config;
        HardwareSerial* commSerial{nullptr};
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


// ------ Constants ------

// MQTT Configuration example
const struct mqttConfiguration MQTT_CONFIG_EXAMPLE = {
  WIFI_SSID,
  WIFI_PASSWORD,
  "broker.hivemq.com",
  1883,
  {
    "de/lab@home/hdcSensor/data",
    "de/lab@home/dht/data"
  },
  {
    "de/lab@home/lightControl",
    "de/lab@home/dht/publishData",
    "de/lab@home/hdc/publishData",
    "de/heizungsanlage/data/publishData"
  }
};

// Lab@Home Pins
//#define TX_PIN 5
//#define RX_PIN 6

// esp32 NodeMCU
#define TX_PIN 26
#define RX_PIN 25

#define TIME_INTERVAL 10000

constexpr int NUM_LEDS{4};
constexpr int DATA_PIN{15};

#endif // __UTIL_H