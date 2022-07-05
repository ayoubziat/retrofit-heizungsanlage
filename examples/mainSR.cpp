#include <Arduino.h>

// Lab@Home Pins
//#define TX_PIN 5
//#define RX_PIN 6


// esp32 NodeMCU
#define TX_PIN 14
#define RX_PIN 13

//HardwareSerial* mySerial = &Serial1;



void setup() {
  Serial.begin(9600);
  Serial1.begin(4800, SERIAL_8E2, RX_PIN, TX_PIN); // RX, TX
  delay(2500); 
}

void loop() {
    uint8_t buff[] = {0x05};
    uint8_t ack[] = {0x06};

    if(Serial1.available()){
        if(Serial1.read() == 0x04) {
            Serial1.write(buff, sizeof(buff));
        }else if(Serial1.read() == 0x16){
            Serial1.write(ack, sizeof(ack));
        }
    }

  //delay(2500);
}