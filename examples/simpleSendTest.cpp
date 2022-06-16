#include <Arduino.h>

// Lab@Home Pins
//#define TX_PIN 5
//#define RX_PIN 6


// esp32 NodeMCU
#define TX_PIN 10
#define RX_PIN 9

//HardwareSerial* mySerial = &Serial1;

const uint8_t buff[] = {0x16, 0x00, 0x00};

void setup() {
  Serial.begin(9600);
  Serial1.begin(4800, SERIAL_8E2, RX_PIN, TX_PIN); // RX, TX
  delay(2500); 
}

void loop() {
  Serial1.write(buff, sizeof(buff));
  delay(2500);
}