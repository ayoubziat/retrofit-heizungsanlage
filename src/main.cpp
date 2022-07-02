#include <Arduino.h>

// Lab@Home Pins
//#define TX_PIN 5
//#define RX_PIN 6

// esp32 NodeMCU
#define TX_PIN 10
#define RX_PIN 9

//HardwareSerial* mySerial = &Serial1;

//const uint8_t buff[] = {0x16, 0x00, 0x00};

/*
Mögliche Zustände während der Kommunikation, ENTWURF
INIT
IDLE
SEND 
LISTEN
WRITE ?
RESET
*/

enum Optolink {
  RESET, INIT, IDLE, SEND, LISTEN
};



void setup() {
  Serial.begin(9600);
  Serial1.begin(4800, SERIAL_8E2, RX_PIN, TX_PIN); // RX, TX
  delay(2500);
}

void loop() {
  //
  Optolink currentState = RESET;
 
  switch (currentState) {
    case RESET: {
      const uint8_t buff[] = {0x04};
      Serial1.write(buff, sizeof(buff));
      if(Serial1.available()) if(Serial1.read() == 0x05) currentState = INIT;
    } break;
    case INIT: {
      const uint8_t buff[] = {0x16, 0x00, 0x00};
      Serial1.write(buff, sizeof(buff));
      if(Serial1.available()) Serial1.read() == 0x06 ? currentState = IDLE : currentState = RESET;
      
    } break;
    case IDLE:
      Serial.println("Finaly in State IDLE!");
      break;
    case SEND:
      break;
    case LISTEN:
      break;
  }

  /* if(Serial1.available() > 1){
    Serial.println();
    Serial.print("Serial Data is available! "); Serial.print("The Message is "); Serial.print(Serial1.read());
  }else{
    Serial.print("No Data received...");
  } */

  //delay(2500);
  
}