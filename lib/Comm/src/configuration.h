#include "credentials.h"
#include "util.h"


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
    "de/heizungsanlage/data/publishData"
  }
};

// Lab@Home Pins
#define TX_PIN 0
#define RX_PIN 4

// esp32 NodeMCU
// #define TX_PIN 26
// #define RX_PIN 25

#define TIME_INTERVAL 30000

constexpr int NUM_LEDS{4};
constexpr int DATA_PIN{15};