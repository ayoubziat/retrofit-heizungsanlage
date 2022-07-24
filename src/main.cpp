#include <Arduino.h>
#include <Wire.h>
#include "util.h"

//HardwareSerial* mySerial = &Serial1;
Communication comm(MQTT_CONFIG_EXAMPLE);

// variables
const uint8_t buff[] = {0x16, 0x00, 0x00};

void setup() {
  Serial.begin(9600);
  // Serial1.begin(4800, SERIAL_8E2, RX_PIN, TX_PIN); // RX, TX
  // delay(2500);
  comm.setup();
}

void loop() {
  //const uint8_t buff[] = {0x16, 0x00, 0x00};
  //Serial1.write(buff, sizeof(buff));
 
  if(Serial1.available() > 1){
    Serial.printf("Serial Data is available! The Message is: %d\n", Serial1.read()); 
  }else{
    // Serial.print("No Data received...\n");
  }

  comm.loop();
}