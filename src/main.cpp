#include <Arduino.h>
#include <string.h>

using namespace std;

// Lab@Home Pins
//#define TX_PIN 5
//#define RX_PIN 6

// esp32 NodeMCU
#define TX_PIN 14
#define RX_PIN 13

struct dataPoint{
  string name;
  float value;
  uint16_t address;
  uint8_t length;
};

enum StateMachine { RESET, INIT, IDLE, READ, WRITE, LISTEN, ACK };
StateMachine currentState = RESET;

class Optolink {
  public:
    unsigned long lastMillis{0};
    unsigned long debugMillis{0};
    uint8_t rxBuffer[255];
    uint8_t rxMsgLen{0};
    HardwareSerial* stream = &Serial1;
    struct dataPoint datapoints[4] = {
      {"Gerätekennung", 0, 0x00F8, 0x02 },
      {"temp_aussen", 0, 0x0800, 0x02 },
      {"temp_kessel", 0, 0x0802, 0x02 },
      {"temp_speicher", 0, 0x0804, 0x02}
    };

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
      uint8_t buff[] = {0x41, 0x05, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00}; //Stelle 4,5,6,7 zunächst nur Platzhalter
      buff[4] = (optolink.datapoints[0].address >> 8) & 0xFF; // noch nicht ganz klar warum...
      buff[5] = optolink.datapoints[0].address & 0xFF; // noch nicht ganz klar warum...
      buff[6] = optolink.datapoints[0].length;
      buff[7] = optolink.getChecksum(buff, sizeof(buff));
      optolink.stream -> write(buff, sizeof(buff));
      // als nächstes Einlesen der Antwort
      optolink.rxMsgLen = 8 + optolink.datapoints[0].length;
      optolink.lastMillis = millis();
      currentState = ACK;
    } break;
    case WRITE:
      break;
    case LISTEN: {
      uint8_t index = 0;
      if(optolink.stream -> available()){
        uint8_t buff = optolink.stream -> read();
        if(buff == 0x06){   // acknowledged
          while(optolink.stream -> available() > 0 && index != optolink.rxMsgLen){
            optolink.rxBuffer[index] = optolink.stream -> read();
            ++index;
          }
          (optolink.rxBuffer[0] == 0x41) ? currentState = ACK : currentState = IDLE;

        } else if(buff == 0x15){    // not acknowledged
          currentState = IDLE;
          optolink.clearInputStream();
        }
      } 
      if(millis() - optolink.lastMillis > 1000UL){    // timeout
        currentState = RESET;
        optolink.clearInputStream();
      } 
    } break;
    case ACK: {
      const uint8_t buffer[] = {0x06};
      optolink.stream -> write(buffer, sizeof(buffer));
      optolink.lastMillis = millis();
      currentState = IDLE;
    } break;
  }

  optolink.debugPrinter();
  
}