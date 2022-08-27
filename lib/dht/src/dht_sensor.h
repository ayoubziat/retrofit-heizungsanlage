#ifndef DHT_SENSOR_H
#define DHT_SENSOR_H

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN 0
#define DHTTYPE DHT11

class DHTSensor{
    public:
        void setup();
        float measureTemperature();
        float measureHumidity();
};

#endif // DHT_SENSOR_H