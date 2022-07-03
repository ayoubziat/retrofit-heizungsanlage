#include <Arduino.h>
#include "util.h"
#include <Wire.h>
#include "ClosedCube_HDC1080.h"

// Lab@Home Pins
//#define TX_PIN 5
//#define RX_PIN 6

// esp32 NodeMCU
#define TX_PIN 10
#define RX_PIN 9

//HardwareSerial* mySerial = &Serial1;
mqttConfiguration mqtt_config = {
  "ssid",
  "pw",
  "broker.hivemq.com",
  "de/heizungsanlage/data"
};
Communication comm(mqtt_config);
ClosedCube_HDC1080 hdc1080;
WiFiClient espClient;
PubSubClient client(espClient);

//const uint8_t buff[] = {0x16, 0x00, 0x00};
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
void setup() {
  Serial.begin(9600);
  hdc1080.begin(0x40);
  // Serial1.begin(4800, SERIAL_8E2, RX_PIN, TX_PIN); // RX, TX
  // delay(2500);

  comm.setup_wifi();
  client.setServer(mqtt_config.mqtt_server, 1883);
	client.setCallback(callback);

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
    Serial.println();
    Serial.print("Serial Data is available! "); Serial.print("The Message is "); Serial.print(Serial1.read());
  }else{
    Serial.print("No Data received...");
  }

  delay(2500);
  
}