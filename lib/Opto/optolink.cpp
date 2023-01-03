#include "optolink.h"


void Optolink::init(){
  if(stream -> available()) {
        if(stream -> read() == 0x05){
          uint8_t buff[] = {0x01};
          stream -> write(buff, sizeof(buff));
          setState(IDLE);
        }
  }
};

void Optolink::idle(){
  clearInputStream();
  //memset(optolink.rxBuffer, 0, 4);
  setLastMillis(millis());
  delay(100);
  setState(READ);
};

void Optolink::read(){
  rxMsgLen = datapoints[datapointIndex].length;
  uint16_t adr = datapoints[datapointIndex].address;
  uint8_t buff[] = { 0xF7, (adr >> 8) & 0xFF, adr & 0xFF, rxMsgLen };
      //Serial.print(buff[1]); Serial.print("\t"); Serial.println(buff[2]);
  stream -> write(buff, sizeof(buff));
  setState(LISTEN);
}

void Optolink::listen(){
  uint8_t i = 0;
  uint8_t readBuffer[] = {0,0,0,0};

  while(stream -> available() > 0) {
        //Serial.println("receiving message!");
    readBuffer[i] = stream -> read();
    ++i;
  }
  
  if(rxMsgLen == i){
        //Serial.println("Message received!");
    uint32_t value = readBuffer[3] << 24 | readBuffer[2] << 16 | readBuffer[1] << 8 | readBuffer[0];  
    
    if(datapoints[datapointIndex].factor == -1){
      datapoints[datapointIndex].value = value;
    } else if(datapoints[datapointIndex].factor == 10){
        datapoints[datapointIndex].value = value / datapoints[datapointIndex].factor;
      }
        
    setState(IDLE);
    // Nächsten Datenpunkt setzen
    if(datapointIndex < sizeof(datapoints) / sizeof(datapoints[0])){
      ++datapointIndex;
    } else { // Datenpunkte wurden einmal komplett durchlaufen
      datapointIndex = 0;
      print = true;
      setState(WAIT);
      }

  }
  // Timeout
  if(timeout(millis(), 2)) {
    clearInputStream();
    setLastMillis(millis());
    setState(INIT);
  }
}

void Optolink::wait(){

}

void Optolink::setState(StateMachine STATE) { currentState = STATE; };
void Optolink::setLastMillis(unsigned long milliseconds){ lastMillis = milliseconds; };
void Optolink::clearInputStream(){ while(stream -> available() > 1) stream -> read(); };

void Optolink::debugPrinter(){
  if(print){
    Serial.println();
    for(int i = 0; i < sizeof(datapoints) / sizeof(datapoints[0]); i++){
      //Serial.print("\t"); Serial.print(datapoints[i].name); Serial.print(": "); Serial.print(datapoints[i].value);
      if(i % 3 == 2) {
        Serial.print("\t\t"); Serial.print(datapoints[i].name); Serial.print(": "); Serial.print(datapoints[i].value); Serial.println();
      }
      if(i % 3 == 1) {
        Serial.print("\t\t"); Serial.print(datapoints[i].name); Serial.print(": "); Serial.print(datapoints[i].value);
      }
      if(i % 3 == 0) {
        Serial.print("\t"); Serial.print(datapoints[i].name); Serial.print(": "); Serial.print(datapoints[i].value);
      }
    }
    Serial.println();
    Serial.println("\t------------------------------------------------------------------------------");
    print = false;
  }
}

bool Optolink::timeout(unsigned long milliseconds, uint16_t factor) {
  return (milliseconds - lastMillis > factor * 1000UL) ? true : false;
}

void Optolink::loop(Communication* comm){
  switch(currentState){
      case INIT: { 
        Optolink::init();
        break;
      }
      case IDLE: {
        Optolink::idle();
        break;
      }
      case READ: {
        Optolink::read();
        break;
      }
      case WRITE: { break; }
      case LISTEN: {
        Optolink::listen();
        break;
      }
      case WAIT: {
        if(Optolink::timeout(millis(), 10)) {
          comm->loop();
          Optolink::clearInputStream();
          for(int j = 0; j < (sizeof(datapoints) / sizeof(datapoints[0])); j++){
            comm->publish(datapoints[j]);
          }
          Optolink::setState(INIT);
        }
        break;
      }
  }

  Optolink::debugPrinter();
}