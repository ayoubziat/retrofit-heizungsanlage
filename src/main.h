#ifndef MAIN_H
#define MAIN_H

#include <iostream>
#include <list>
#include <string.h>


struct mqttConfiguration {
    /* data */
    const char* ssid;
    const char* password;
    const char* mqtt_server;
    int mqtt_port;
    // const char* mqtt_publish_topic;
    // const char* mqtt_subscribe_topic;
    std::list<std::string> mqtt_publish_topics;
    std::list<std::string> mqtt_subscribe_topics;
};

// Function prototypes
extern void printSerialNumber();


// ------ Constants ------

// MQTT Configuration example
// const struct mqttConfiguration MQTT_CONFIG_EXAMPLE = {
//   "ssid",
//   "pw",
//   "broker.hivemq.com",
//   1883,
//   {
//     "de/lab@home/data/temperature", 
//     "de/lab@home/data/humidity",
//     "de/heizungsanlage/data/temperature"
//   },
//   {
//     "de/lab@home/lightControl",
//     "de/heizungsanlage/data",
//   }
// };

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

#endif // MAIN_H