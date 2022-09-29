#include <Arduino.h>
#include <Wire.h>
#include <string.h>
#include <util.h>

using namespace std;

Optolink optolink;  
Communication comm(MQTT_CONFIG_EXAMPLE);

void setup() {
  Serial.begin(57600);
  comm.commSerial = &Serial;
  comm.setup();
  delay(2000);
  optolink.stream = &Serial1;
  optolink.stream -> begin(4800, SERIAL_8E2, RX_PIN, TX_PIN);
}

void loop() {

  switch(optolink.currentState){
    case INIT: { 
      optolink.init();
      break;
    }
    case IDLE: {
      optolink.idle();
      break;
    }
    case READ: {
      optolink.read();
      break;
    }
    case WRITE: { break; }
    case LISTEN: {
      optolink.listen();
      break;
    }
    case WAIT: {
      if(optolink.timeout(millis(), 10)) {
        comm.loop();
        optolink.clearInputStream();
        for(int j = 0; j < (sizeof(optolink.datapoints) / sizeof(optolink.datapoints[0])); j++){
          comm.publish(optolink.datapoints[j]);
        }
        optolink.setState(INIT);
      }
      break;
    }
  }

  optolink.debugPrinter();
}