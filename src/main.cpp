#include <Arduino.h>
#include "util.h"
#include <Wire.h>
#include "ClosedCube_HDC1080.h"
#include <FastLED.h>

// Lab@Home Pins
//#define TX_PIN 5
//#define RX_PIN 6

// esp32 NodeMCU
#define TX_PIN 10
#define RX_PIN 9

#define TIME_INTERVAL 5000
//HardwareSerial* mySerial = &Serial1;

ClosedCube_HDC1080 hdc1080;
WiFiClient deviceClient;
PubSubClient client(deviceClient);
Communication comm;

// constants
constexpr int NUM_LEDS{4};
constexpr int DATA_PIN{15};

// variables
//const uint8_t buff[] = {0x16, 0x00, 0x00};
int64_t nextTimeHDC1080{0};
char msg[50];
int value = 0;
float temperature = 0;
float humidity = 0;
CRGB leds[NUM_LEDS];

// Function prototypes
void printSerialNumber();
void callback(char*, byte*, unsigned int);
void reconnect();

void setup() {
  Serial.begin(9600);
  hdc1080.begin(0x40);
  // Serial1.begin(4800, SERIAL_8E2, RX_PIN, TX_PIN); // RX, TX
  // delay(2500);

  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS); // GRB ordering is assumed
  delay(10);
  comm.setup();
  delay(10);
  comm.setup_wifi();
  client.setServer(comm.mqtt_config.mqtt_server, comm.mqtt_config.mqtt_port);
	client.setCallback(callback);
  reconnect();

  Serial.print("Manufacturer ID=0x");
	Serial.println(hdc1080.readManufacturerId(), HEX); // 0x5449 ID of Texas Instruments
	Serial.print("Device ID=0x");
	Serial.println(hdc1080.readDeviceId(), HEX); // 0x1050 ID of the device
  printSerialNumber();
}

void loop() {
  //const uint8_t buff[] = {0x16, 0x00, 0x00};
  //Serial1.write(buff, sizeof(buff));
 
  if(Serial1.available() > 1){
    Serial.printf("Serial Data is available! The Message is: %d\n", Serial1.read()); 
  }else{
    // Serial.print("No Data received...\n");
  }

  if (!client.connected()) {
		// comm.reconnect(client);
    reconnect();
	}

	client.loop();

	long current_time = millis();
	if (current_time - nextTimeHDC1080 > TIME_INTERVAL) {
		nextTimeHDC1080 = current_time;
		
		temperature = hdc1080.readTemperature(); 
		humidity = hdc1080.readHumidity();
		
		// Convert the values to a char array
		char tempString[8];
		dtostrf(temperature, 1, 2, tempString);
		char humString[8];
		dtostrf(humidity, 1, 2, humString);
		Serial.printf("Temp=%f C, Humidity= %f % \n", temperature, humidity);

    // Publish the values
    for (string topic: comm.mqtt_config.mqtt_publish_topics){
      if(topic.find("temperature")!=string::npos){
          if(!client.publish(topic.c_str(), tempString))
		        Serial.println("Error while publishing the temperature value");
      }else if(topic.find("humidity")!=string::npos){
          if(!client.publish(topic.c_str(), humString))
		        Serial.println("Error while publishing the humidity value");
      }else{
        // ToDo
      }
    }
	}
  delay(2500);
}

void printSerialNumber() {
	Serial.print("Device Serial Number=");
	HDC1080_SerialNumber sernum = hdc1080.readSerialNumber();
	char format[40];
	sprintf(format, "%02X-%04X-%04X", sernum.serialFirst, sernum.serialMid, sernum.serialLast);
	Serial.println(format);
}

void callback(char* topic, byte* message, unsigned int length){
  comm.callback(topic, message, length);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    printf("Attempting MQTT connection...\n\t");
    // Attempt to connect
    if (client.connect("Lab@home")) {
      printf("Device connected\n");
      // Subscribe to topics
      for(string mqtt_topic: comm.mqtt_config.mqtt_subscribe_topics){
        printf("Subscribing to MQTT topic: %s\n", mqtt_topic.c_str());
        client.subscribe(mqtt_topic.c_str());
      }
      break;
    } else {
      printf("Failed, rc=%d. Try again in 1 seconds\n", client.state());
      // Wait 1 seconds before retrying
      delay(1000);
    }
  }
}