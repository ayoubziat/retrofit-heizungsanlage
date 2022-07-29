#include <Arduino.h>
#include <Wire.h>
#include <string.h>
#include <util.h>
#include <optolink.h>

using namespace std;

enum StateMachine { INIT = 0, IDLE, READ, WRITE, LISTEN, WAIT };
StateMachine currentState = INIT;

Optolink optolink;  
Communication comm(MQTT_CONFIG_EXAMPLE);

void setup() {
  Serial.begin(57600);
  comm.commSerial = &Serial;
  comm.setup();
  delay(2000);
  optolink.stream = &Serial1;
  optolink.stream -> begin(4800, SERIAL_8E2);
}

void loop() {

  switch(currentState){
    case INIT: {
      if(optolink.stream -> available()) {
        if(optolink.stream -> read() == 0x05){
          uint8_t buff[] = {0x01};
          optolink.stream -> write(buff, sizeof(buff));
          currentState = IDLE;
        }
      }
      break;
    }
    case IDLE: {
      optolink.clearInputStream();
      //memset(optolink.rxBuffer, 0, 4);
      optolink.lastMillis = millis();
      delay(100);
      currentState = READ;
    }
    case READ: {
      optolink.rxMsgLen = optolink.datapoints[optolink.datapointIndex].length;
      uint16_t adr = optolink.datapoints[optolink.datapointIndex].address;
      uint8_t buff[] = { 0xF7, (adr >> 8) & 0xFF, adr & 0xFF, optolink.rxMsgLen };
      //Serial.print(buff[1]); Serial.print("\t"); Serial.println(buff[2]);
      optolink.stream -> write(buff, sizeof(buff));
      currentState = LISTEN;
      break;
    }
    case WRITE: {
      break;
    }
    case LISTEN: {
      uint8_t i = 0;
      uint8_t readBuffer[] = {0,0,0,0};
      while(optolink.stream -> available() > 0) {
        //Serial.println("receiving message!");
        readBuffer[i] = optolink.stream -> read();
        ++i;
      }
      if(optolink.rxMsgLen == i){
        //Serial.println("Message received!");
        uint32_t value = readBuffer[3] << 24 | readBuffer[2] << 16 | readBuffer[1] << 8 | readBuffer[0];
        
        if(optolink.datapoints[optolink.datapointIndex].factor == -1){
          optolink.datapoints[optolink.datapointIndex].value = value;
        } else{
          optolink.datapoints[optolink.datapointIndex].value = value / optolink.datapoints[optolink.datapointIndex].factor;
        }
        
        // Nächsten Datenpunkt setzen
        if(optolink.datapointIndex < sizeof(optolink.datapoints) / sizeof(optolink.datapoints[0])){
          ++optolink.datapointIndex;
        } else {
          // Datenpunkte wurden einmal komplett durchlaufen
          optolink.datapointIndex = 0;
          optolink.print = true;
          currentState = WAIT;
        }

        currentState = IDLE;
      }
      // Timeout
      if(millis() - optolink.lastMillis > 2 * 1000UL ) {
        optolink.clearInputStream();
        optolink.lastMillis = millis();
        currentState = INIT;
      }
      break;
    }
    case WAIT: {
      if(millis() - optolink.lastMillis > 10 * 1000UL) {
        optolink.clearInputStream();
        currentState = INIT;
      }
      break;
    }
  }

  comm.loop();
  optolink.debugPrinter();
  
}