#include <Arduino.h>
#include <Wire.h>
#include <string.h>
#include "util.h"
#include "configuration.h"
#include "optolink.h"

using namespace std;

Optolink optolink;  
Communication comm(MQTT_CONFIG_EXAMPLE);

boolean SIMULATION_MODE = false;

void setup() {
  Serial.begin(57600);
  comm.setup();
  delay(2000);
  optolink.stream = &Serial1;
  optolink.stream -> begin(4800, SERIAL_8E2, RX_PIN, TX_PIN);
}

void loop() {
  if (SIMULATION_MODE) {
    // ----- Simulation code ----- 
    comm.loop();
  }
  else
  {
    optolink.loop(&comm);
  }
}