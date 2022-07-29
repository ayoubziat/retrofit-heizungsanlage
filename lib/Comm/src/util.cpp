#include "util.h"

ClosedCube_HDC1080 hdc1080;
WiFiClient deviceClient;
PubSubClient pubSubClient(deviceClient);

int64_t nextTimeHDC1080{0};
char msg[50];
float temperature = 0;
float humidity = 0;
CRGB leds[NUM_LEDS];

static mqttConfiguration* CONFIG;

Communication::Communication(struct mqttConfiguration config)
{
    comm_mqtt_config = config;
    CONFIG = &comm_mqtt_config;
}

void Communication::setup(){
  this->commSerial->print("## Comm Setup ## \n");
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS); // GRB ordering is assumed
  hdc1080.begin(0x40);
  Communication::setup_wifi();

  pubSubClient.setServer(comm_mqtt_config.mqtt_server, comm_mqtt_config.mqtt_port);
	pubSubClient.setCallback(mqttCallback);
  Communication::reconnect();
  printSerialNumber();
}

void Communication::setup_wifi() {
  // We start by connecting to a WiFi network
  this->commSerial->print("\n### Setup WiFi ###\n");
  this->commSerial->print("\tConnecting to ");
  this->commSerial->print(comm_mqtt_config.ssid);
  WiFi.begin(comm_mqtt_config.ssid, comm_mqtt_config.password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    this->commSerial->print(".");
  }
  this->commSerial->print("\n\tWiFi connected!");
  char buf[16];
  IPAddress ip = WiFi.localIP();
  sprintf(buf, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
  this->commSerial->printf("\n\tIP address: %s\n\n", buf);
}

void Communication::reconnect() {
  this->commSerial->print("## Reconnect ##\n");
  // Loop until we're reconnected
  while (!pubSubClient.connected()) {
    this->commSerial->print("Attempting MQTT connection...\n\t");
    // Attempt to connect
    if (pubSubClient.connect("Lab@home")) {
      this->commSerial->print("Device connected\n");
      // Subscribe to topics
      for(string mqtt_topic: comm_mqtt_config.mqtt_subscribe_topics){
        this->commSerial->printf("Subscribing to MQTT topic: %s\n", mqtt_topic.c_str());
        pubSubClient.subscribe(mqtt_topic.c_str());
      }
      break;
    } else {
      this->commSerial->printf("Failed, rc=%d. Try again in 2 seconds\n", pubSubClient.state());
      // Wait 2 seconds before retrying
      delay(2000);
    }
  }
}

void Communication::mqttCallback(char* topic, byte* message, unsigned int length){
  Serial.print("## MQTT Callback ##\n");
  Serial.printf("Message arrived on topic: %s.\n", topic);
  printf("Message is: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.printf("%c", (char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  if (string(topic) == "de/lab@home/lightControl") {
    Serial.print("Changing LED output to ");
    if(messageTemp == "on"){
      Serial.print("on\n");
      leds[0] = CRGB::DarkOliveGreen;
      FastLED.show();
    }
    else if(messageTemp == "off"){
      Serial2.print("off\n");
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
}

void Communication::loop(){
  if (!pubSubClient.connected()) {
    Communication::reconnect();
	}

	pubSubClient.loop();

	// long current_time = millis();
	// if (current_time - nextTimeHDC1080 > TIME_INTERVAL) {
	// 	nextTimeHDC1080 = current_time;
		
	// 	temperature = hdc1080.readTemperature(); 
	// 	humidity = hdc1080.readHumidity();
		
	// 	// Convert the values to a char array
	// 	char tempString[8];
	// 	dtostrf(temperature, 1, 2, tempString);
	// 	char humString[8];
	// 	dtostrf(humidity, 1, 2, humString);
	// 	printf("Temp=%f C, Humidity= %f % \n", temperature, humidity);

  //   // Publish the values
  //   for (string topic: this->comm_mqtt_config.mqtt_publish_topics){
  //     if(topic.find("temperature") != string::npos){
  //         if(!pubSubClient.publish(topic.c_str(), tempString))
	// 	        printf("Error while publishing the temperature value!\n");
  //     }
  //     else if(topic.find("humidity") != string::npos){
  //         if(!pubSubClient.publish(topic.c_str(), humString))
	// 	        printf("Error while publishing the humidity value!\n");
  //     }
  //     else {
  //       // ToDo
  //     }
  //   }
	// }
  //delay(2500);
}

void Communication::publish(struct dataPoint point){
          // Convert the values to a char array
          char valueString[8];
          // ultoa(value, tempString);
          sprintf(valueString, "%d", point.value);
          // Serial.print(point.name);
          // Serial.printf(" = %d \n",point.value);
          if(point.name == "03_Kesseltemp_(S3)"){
              Serial.printf("### Publish: ###\n");
              Serial.println(point.name);
              Serial.printf(" = %d \n",point.value);
              if(!pubSubClient.publish("de/heizungsanlage/data/kesseltemperatureist", valueString))
                Serial.print("Error while publishing the geraete value!\n");
            }
            else if(point.name == "17_Kesseltemp_Soll"){
              Serial.printf("### Publish: ###\n");
              Serial.println(point.name);
              Serial.printf(" = %d \n",point.value);
                if(!pubSubClient.publish("de/heizungsanlage/data/kesseltemperaturesoll", valueString))
                  Serial.print("Error while publishing the kesseltemperatureist value!\n");
            }
            else if(point.name == "19_Betriebsart"){
              Serial.printf("### Publish: ###\n");
              Serial.println(point.name);
              Serial.printf(" = %d \n",point.value);
                if(!pubSubClient.publish("de/heizungsanlage/data/betriebsart", valueString))
                  Serial.print("Error while publishing the kesseltemperaturesoll value!\n");
            }
            else if(point.name == "20_Sparbetrieb"){
              Serial.printf("### Publish: ###\n");
              Serial.println(point.name);
              Serial.printf(" = %d \n",point.value);
                if(!pubSubClient.publish("de/heizungsanlage/data/sparbetrieb", valueString))
                  Serial.print("Error while publishing the betriebsart value!\n");
            }
            else if(point.name == "Party Temperatur Soll"){
              Serial.printf("### Publish: ###\n");
              Serial.println(point.name);
              Serial.printf(" = %d \n",point.value);
                if(!pubSubClient.publish("de/heizungsanlage/data/partytemperaturesoll", valueString))
                  Serial.print("Error while publishing the sparbetrieb value!\n");
            }
          // Publish the values
          for (string topic: this->comm_mqtt_config.mqtt_publish_topics){
            
          }
}

// Getters
mqttConfiguration Communication::getMQTTConfig() {
  return comm_mqtt_config;
}


// Helper functions
void printSerialNumber() {
  Serial.print("\n## Print Serial Number ##\n");
  Serial.printf("Manufacturer ID=0x%x\n", hdc1080.readManufacturerId()); // 0x5449 ID of Texas Instruments
	Serial.printf("Device ID=0x%x\n", hdc1080.readDeviceId()); // 0x1050 ID of the device

	HDC1080_SerialNumber sernum = hdc1080.readSerialNumber();
	char format[40];
	sprintf(format, "%02X-%04X-%04X", sernum.serialFirst, sernum.serialMid, sernum.serialLast);
	Serial.printf("Device Serial Number=%s\n\n", format);
}