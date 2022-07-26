#ifndef DHT_SENSOR_H
#define DHT_SENSOR_H

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN 19
#define DHTTYPE DHT11

class DHTSensor{
    public:
        //DHTSensor();
        void setup();
        void measureTemperature();
        void measureHumidity();
};

#endif // DHT_SENSOR_H