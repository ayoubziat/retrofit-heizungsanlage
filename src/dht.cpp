#include "dht_sensor.h"

DHT dht(DHTPIN, DHTTYPE);

// DHTSensor::DHTSensor(){
// }

void DHTSensor::setup(){
    dht.begin();
}

void DHTSensor::measureTemperature(){
    delay(250);
    float temp = dht.readTemperature();
    // Check if read failed.
    if (isnan(temp)) {
        Serial.println(F("Failed to read temperature from DHT sensor!"));
    }else{
        Serial.printf("Temperature: %F °C\n", temp);
    }
}

void DHTSensor::measureHumidity(){
    delay(250);
    float humidity = dht.readHumidity();
    // Check if read failed.
    if (isnan(humidity)) {
        Serial.println(F("Failed to read humidity from DHT sensor!"));
    }else{
        Serial.printf("Humidity: %F %\n", humidity);
    }
}