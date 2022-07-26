#include <Arduino.h>
#include "dht_sensor.h"

DHTSensor dhtSensor;

void setup() {
  Serial.begin(9600);
  delay(2500);
  dhtSensor.setup();
  
}

void loop() {
  // Wait 5 seconds between measurements.
  delay(5000);
  dhtSensor.measureHumidity();
  // Read temperature as Celsius (the default)
  dhtSensor.measureTemperature();
}