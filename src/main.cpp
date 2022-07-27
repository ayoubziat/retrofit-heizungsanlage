#include <Arduino.h>
#include <Wire.h>
#include "util.h"

Communication comm(MQTT_CONFIG_EXAMPLE);

void setup() {
  Serial.begin(9600);
  comm.setup();
}

void loop() {
  comm.loop();
}