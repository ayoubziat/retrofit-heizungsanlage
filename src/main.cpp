#include <Arduino.h>
#include <Wire.h>
#include <FastLED.h>
#include <PubSubClient.h>
#include <ClosedCube_HDC1080.h>
#include <WiFi.h>
#include "main.h"

using namespace std;

ClosedCube_HDC1080 hdc1080;
WiFiClient deviceClient;
PubSubClient pubSubClient(deviceClient);

int64_t nextTimeHDC1080{0};
char msg[50];
float temperature = 0;
float humidity = 0;
CRGB leds[NUM_LEDS];

struct dataPoint{
  uint32_t value;
  uint16_t address;
  uint8_t length;
  int16_t factor;
  String name;
};

enum StateMachine { INIT = 0, IDLE, READ, WRITE, LISTEN, WAIT };
StateMachine currentState = IDLE;

class Optolink {
  public:
    bool print = true;
    unsigned long lastMillis{0};
    //uint8_t rxBuffer[4];
    uint8_t rxMsgLen{0};
    uint8_t datapoint{0};
    HardwareSerial* stream{nullptr};
    struct dataPoint datapoints[63] = {
      {0, 0x00F8, 0x02, -1, "Gerätekennung"},
      // IST
      {0, 0x0800, 0x02, 10, "01_Aussentemp_(S1)"},
      {0, 0x5525, 0x02, 10, "02_Aussentemp_Tiefpass"},
      {0, 0x0802, 0x02, 10, "03_Kesseltemp_(S3)"},
      {0, 0x0804, 0x02, 10, "04_Speichertemp_(S5)"},
      {0, 0x0806, 0x02, 10, "05_Speichertemp_2_(S5B)"},
      {0, 0x0808, 0x02, 10, "06_Abgastemp_(S15)"},
      {0, 0x080A, 0x02, 10, "07_Rücklauftemp_RLTS(17A)"},
      {0, 0x080C, 0x02, 10, "08_Vorlauf_IST_temp_(17B)"},
      {0, 0x2900, 0x02, 10, "09_Vorlauf_IST_temp_M1"},
      {0, 0x2900, 0x02, 10, "10_Vorlauftemp_M1(minimal)"},
      {0, 0x0896, 0x02, 10, "11_Raumtemp_A1M1_Tiefpass"},
      {0, 0x0810, 0x02, 10, "12_Kesseltemp_Tiefpass"},
      {0, 0x0812, 0x02, 10, "13_Speichertemp_Tiefpass"},
      {0, 0x2510, 0x01, -1, "14_Frostgefahr"},
      // SOLL
      {0, 0x6300, 0x02, 10, "15_Warmwassertemp_Soll"},
      {0, 0x2511, 0x02, 10, "16_Vorlauftemp_Soll_M1"},
      {0, 0x5502, 0x02, 10, "17_Kesseltemp_Soll"},
      {0, 0x2306, 0x01, 1, "18_Raumtemp_Soll"},
      // Betriebsart
      {0, 0x2301, 0x01, -1, "19_Betriebsart" },
      {0, 0x2302, 0x01, -1, "20_Sparbetrieb" },
      {0, 0x2303, 0x01, -1, "21_Partybetrieb"},
      // Heizkennlinie
      {0, 0x2304, 0x01, 1, "22_Heizkennlinie Niveau"},
      {0, 0x2305, 0x01, 10, "23_Heizkennlinie Neigung"},
      // Status
      {0, 0x083A, 0x01, -1, "24_Aussentemp_Sensor(Status)"},
      {0, 0x083B, 0x01, -1, "25_Kesseltemp_Sensor(Status)"},
      {0, 0x551E, 0x01, -1, "26_Brennerstatus 0..2"},
      {0, 0x0842, 0x01, -1, "27_Brenner_Status(Stufe1)"},
      {0, 0x55E3, 0x01,  2, "28_Leistung_IST(0..100%)"},
      {0, 0x088A, 0x04, -1, "29_Brennerstarts"},
      // Pumpen
      {0, 0x0845, 0x01, -1, "30_Speicherladepumpe 0..1"},
      {0, 0x0846, 0x01, -1, "31_Zirkulationspumpe 0..1"},
      {0, 0x2906, 0x01, -1, "32_Heizkreispumpe_A1M1"},
      {0, 0x254C, 0x01,  2, "33_Mischerpos_M1(0..100%)"},
      // Störung
      {0, 0x0847, 0x01, -1, "34_Sammelstörung(0..1)"},
      {0, 0x0883, 0x01, -1, "35_Brennerstörung(0..1)"},
      // Timer
      /* {0, 0x3028, 0x08, -1, "36_Zirkulationspumpe 0..1"},
      {0, 0x2128, 0x08, -1, "37_Heizkreispumpe_A1M1"},
      {0, 0x2228, 0x08,  2, "38_Mischerpos_M1(0..100%)"}, */
      // Weitere
      {0, 0x5527, 0x02, 10, "Aussentemp_gedämpft"},
      {0, 0x3900, 0x02, 10, "Vorlauftemp_M2"},
      {0, 0x3902, 0x02, 10, "Rücklauftemp_M2"},
      {0, 0xA309, 0x02, 10, "Kesseltemp"},
      {0, 0x0814, 0x02, 10, "Speichertemp_2(Tiefpass)"},
      {0, 0x0816, 0x02, 10, "Abgastemp_Tiefpass"},
      {0, 0x0818, 0x02, 10, "Rücklauftemp_Tiefpass(17A)"},
      {0, 0x081A, 0x02, 10, "Rück/Vorlauftemp_Tiefpass(17B)"},
      {0, 0x089F, 0x02, 10, "max_Abgastemperatur"},
      {0, 0x2544, 0x02, 10, "Vorlauftemp_Soll_A1M1"},
      {0, 0x555A, 0x02, 10, "Kesseltemp_Soll"},
      {0, 0x0849, 0x01, -1, "Zustand 2. Stufe"},
      {0, 0x7574, 0x04, 1000, "Oelverbrauch"},
      {0, 0x08A7, 0x04, -1, "Betriebsstunden Stufe 1"},
      {0, 0x08AB, 0x04, -1, "Betriebsstunden Stufe 2"},
      {0, 0x5555, 0x01,  2, "Drosselklappenposition (0..100%)"},
      {0, 0x2500, 0x02, -1, "aktuelle Betriebsart A1M1"},
      //{0, 0x3500, 0x16, -1, "aktuelle Betriebsart M2"},
      {0, 0xA38F, 0x02, -1, "Anlagen Ist-Leistung"},
      {0, 0xA305, 0x02, -1, "Kesselleistung"},
      //{0, 0x08A1, 0x02,  0 | 2, "maximal erreichte Abgastemp. zurücksetzen "},
      {0, 0x2307, 0x01, -1, "Red. Raumtemperatur Soll"},
      {0, 0x2308, 0x01, -1, "Party Temperatur Soll"},
      {0, 0x2501, 0x01, -1, "Betriebsart" },
      {0, 0x2502, 0x04, 10, "Timer Aufheizphase"},
      {0, 0x2508, 0x01, -1, "Warmwasser Freigabe"},
      {0, 0x250A, 0x01, -1, "Heizkreispumpe"},
      {0, 0x250B, 0x01, -1, "Raumaufschaltung"},
      {0, 0x250C, 0x02, 10, "Raum Solltemperatur"},
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
  };

class Communication {
    public:
        struct mqttConfiguration comm_mqtt_config = {
          "ssid",
          "pw",
          "broker.hivemq.com",
          1883,
          {
            "de/lab@home/data/temperature", 
            "de/lab@home/data/humidity",
            "de/heizungsanlage/data/temperature"
          },
          {
            "de/lab@home/lightControl",
            "de/heizungsanlage/data",
          }
        };
        Communication(){

        };
        void setup(){
          Serial.print("## Comm Setup ## \n");
          FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS); // GRB ordering is assumed
          hdc1080.begin(0x40);
          setup_wifi();

          pubSubClient.setServer(comm_mqtt_config.mqtt_server, comm_mqtt_config.mqtt_port);
          pubSubClient.setCallback(mqttCallback);
          reconnect();
          printSerialNumber();
        };
        void setup_wifi(){
          // We start by connecting to a WiFi network
          Serial.print("\n### Setup WiFi ###\n");
          Serial.print("\tConnecting to ");
          Serial.print(comm_mqtt_config.ssid);
          WiFi.begin(comm_mqtt_config.ssid, comm_mqtt_config.password);
          while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.print(".");
          }
          Serial.print("\n\tWiFi connected!");
          char buf[16];
          IPAddress ip = WiFi.localIP();
          sprintf(buf, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
          Serial.print("\n\tIP address: ");
          Serial.print(buf);
          Serial.println();
        };
        void reconnect(){
          Serial.print("## Reconnect ##\n");
          // Loop until we're reconnected
          while (!pubSubClient.connected()) {
            Serial.print("Attempting MQTT connection...\n\t");
            // Attempt to connect
            if (pubSubClient.connect("Lab@home")) {
              Serial.print("Device connected\n");
              //Serial.printSubscribe to topics
              for(string mqtt_topic: comm_mqtt_config.mqtt_subscribe_topics){
                Serial.print("Subscribing to MQTT topic: ");
                Serial.print(mqtt_topic.c_str());
                Serial.println();
                pubSubClient.subscribe(mqtt_topic.c_str());
              }
              Serial.print("----- End -----\n");
              break;
            } else {
              Serial.print("Failed, rc=");
              Serial.print(pubSubClient.state());
              Serial.print(". Try again in 5 seconds\n");
              Serial.println();
              // Wait 2 seconds before retrying
              delay(2000);
            }
          }
        };
        static void mqttCallback(char* topic, byte* message, unsigned int length){
          Serial.print("## MQTT Callback ##\n");
          Serial.print("Message arrived on topic: ");
          Serial.print(topic);
          Serial.print(".\nMessage is: ");
          String messageTemp;
          
          for (int i = 0; i < length; i++) {
            Serial.print((char)message[i]);
            messageTemp += (char)message[i];
          }
          Serial.print("\n");

          if (string(topic) == "de/lab@home/lightControl") {
            Serial.print("Changing LED output to ");
            if(messageTemp == "on"){
              Serial.print("on\n");
              leds[0] = CRGB::DarkOliveGreen;
              FastLED.show();
            }
            else if(messageTemp == "off"){
              Serial.print("off\n");
              leds[0] = CRGB::Black;
              FastLED.show();
            }
            else{
              Serial.print("Message couldn't be handled\n");
            }
          } 
          else if (string(topic) == "de/heizungsanlage/data"){
            if(messageTemp == "true"){
              Serial.print("Subscribe to topic \"de/heizungsanlage/data/+\"\n");
              pubSubClient.subscribe("de/heizungsanlage/data/+");
            }
            else if(messageTemp == "false"){
              Serial.print("Unsubscribe from topic \"de/heizungsanlage/data/+\"\n");
              pubSubClient.unsubscribe("de/heizungsanlage/data/+");
            }
            else{
              Serial.print("Message couldn't be handled\n");
            }
          } 
          else{
            // ToDo
          }
        };
        void loop(){
          if (!pubSubClient.connected()) {
            Communication::reconnect();
          }

          pubSubClient.loop();
        };
};

Optolink optolink;  
Communication comm;

void setup() {
  Serial.begin(57600);
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
      optolink.rxMsgLen = optolink.datapoints[optolink.datapoint].length;
      uint16_t adr = optolink.datapoints[optolink.datapoint].address;
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
        
        if(optolink.datapoints[optolink.datapoint].factor == -1){
          optolink.datapoints[optolink.datapoint].value = value;
        } else{
          optolink.datapoints[optolink.datapoint].value = value / optolink.datapoints[optolink.datapoint].factor;
        }
        
          // Nächsten Datenpunkt setzen
        if(optolink.datapoint < sizeof(optolink.datapoints) / sizeof(optolink.datapoints[0])){
          ++optolink.datapoint;
        } else {
          // Datenpunkte wurden einmal komplett durchlaufen
          optolink.datapoint = 0;
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


void printSerialNumber()
{
  Serial.printf("\n## Print Serial Number ##\n");
  Serial.printf("Manufacturer ID=0x%x\n", hdc1080.readManufacturerId()); // 0x5449 ID of Texas Instruments
	Serial.printf("Device ID=0x%x\n", hdc1080.readDeviceId()); // 0x1050 ID of the device
  
	HDC1080_SerialNumber sernum = hdc1080.readSerialNumber();
	Serial.printf("Device Serial Number=%02X-%04X-%04X\n", sernum.serialFirst, sernum.serialMid, sernum.serialLast);
}