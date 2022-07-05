#include <Arduino.h>

// Lab@Home Pins
//#define TX_PIN 5
//#define RX_PIN 6

// esp32 NodeMCU
#define TX_PIN 14
#define RX_PIN 13

enum StateMachine { RESET, INIT, IDLE, READ, WRITE };
StateMachine currentState = RESET;

class Optolink {
  public:
    HardwareSerial* stream = &Serial1;
    uint16_t address = 0x00F8; // Gerätekennung zum Testen
    unsigned long lastMillis = 0;
    unsigned long debugMillis = 0;

    void clearInputStream(void){
       while(stream -> available() > 1) stream -> read();
    };

    uint8_t getChecksum(uint8_t array[], uint8_t length){
      uint8_t checksum = 0;
      for(uint8_t i = 1; i < length -1; i++) checksum += array[i];
      return checksum;
    };

    void debugPrinter(void){
      if(millis() - debugMillis > 1 * 1000UL){
        Serial.println("Current State: "); Serial.print(currentState);
        debugMillis = millis();
      }
    }
};

Optolink optolink;  


void setup() {
  Serial.begin(9600);
  optolink.stream -> begin(4800, SERIAL_8E2, RX_PIN, TX_PIN); // RX, TX
  delay(1000);
}

void loop() {
  //
  switch (currentState) {
    case RESET: {
      const uint8_t buff[] = {0x04};
      optolink.stream -> write(buff, sizeof(buff));
      if(optolink.stream -> available()) {
        if(optolink.stream -> read() == 0x05) {
          currentState = INIT;
        }else {
          optolink.clearInputStream();
        }
      }
    } break;
    case INIT: {        
      const uint8_t buff[] = {0x16, 0x00, 0x00};
      optolink.stream -> write(buff, sizeof(buff));
      if(optolink.stream -> available()) optolink.stream -> read() == 0x06 ? currentState = IDLE : currentState = RESET;  
    } break;
    case IDLE:
      if(millis() - optolink.lastMillis > 15 * 1000UL) {
        currentState = INIT;
        optolink.lastMillis = millis();
      }
      optolink.clearInputStream();
      break;
    case READ: {
      uint8_t buff[] = {0x41, 0x05, 0x00, 0x01, 0x00, 0x00, 0x02, 0x00}; //Stelle 4,5,7 zunächst nur Platzhalter
      buff[4] = (optolink.address >> 8) & 0xFF; // noch nicht ganz klar warum...
      buff[5] = optolink.address & 0xFF; // noch nicht ganz klar warum...
      buff[7] = optolink.getChecksum(buff, sizeof(buff));
      optolink.stream -> write(buff, sizeof(buff));
      // als nächstes Einlesen der Antwort
    } break;
    case WRITE:
      break;
  }

  optolink.debugPrinter();
  
}