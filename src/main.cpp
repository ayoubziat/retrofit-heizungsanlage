#include <Arduino.h>

// Lab@Home Pins
//#define TX_PIN 5
//#define RX_PIN 6

// esp32 NodeMCU
#define TX_PIN 10
#define RX_PIN 9

HardwareSerial* mySerial = &Serial1;

void send();
void receive();

void setup() {
  Serial.begin(9600);
  Serial1.begin(4800, SERIAL_8E2, RX_PIN, TX_PIN); // RX, TX
  delay(2500);
}

void loop() {
  //send();
  receive();

  delay(2500);
}

void send(){
  const uint8_t buff[] = {0x16, 0x00, 0x00};
  Serial1.write(buff, sizeof(buff));
}

void receive(){
  if(Serial1.available() > 1){
    Serial.println();
    Serial.print("Serial Data is available! "); Serial.print("The Message is "); Serial.print(Serial1.read());
  }else{
    Serial.print("No Data received...");
  }
}