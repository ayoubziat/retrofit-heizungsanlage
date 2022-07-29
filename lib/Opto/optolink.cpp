#include <Arduino.h>
#include <string.h>
#include <optolink.h>

void Optolink::clearInputStream(){ while(stream -> available() > 1) stream -> read(); }

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
};