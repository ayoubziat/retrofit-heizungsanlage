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
    printf("## Init Communication Class ## \n");
    comm_mqtt_config = config;
    CONFIG = &comm_mqtt_config;
}

void Communication::setup(){
  printf("## Comm Setup ## \n");
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
  printf("\n### Setup WiFi ###\n");
  printf("\tConnecting to ");
  printf(comm_mqtt_config.ssid);
  WiFi.begin(comm_mqtt_config.ssid, comm_mqtt_config.password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    printf(".");
  }
  printf("\n\tWiFi connected!");
  char buf[16];
  IPAddress ip = WiFi.localIP();
  sprintf(buf, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
  printf("\n\tIP address: %s\n\n", buf);
}

void Communication::reconnect() {
  printf("## Reconnect ##\n");
  // Loop until we're reconnected
  while (!pubSubClient.connected()) {
    printf("Attempting MQTT connection...\n\t");
    // Attempt to connect
    if (pubSubClient.connect("Lab@home")) {
      printf("Device connected\n");
      // Subscribe to topics
      for(string mqtt_topic: comm_mqtt_config.mqtt_subscribe_topics){
        printf("Subscribing to MQTT topic: %s\n", mqtt_topic.c_str());
        pubSubClient.subscribe(mqtt_topic.c_str());
      }
      printf("----- End -----\n");
      break;
    } else {
      printf("Failed, rc=%d. Try again in 5 seconds\n", pubSubClient.state());
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void Communication::mqttCallback(char* topic, byte* message, unsigned int length){
  printf("## MQTT Callback ##\n");
  printf("Message arrived on topic: %s.\n", topic);
  printf("Message is: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    printf("%c", (char)message[i]);
    messageTemp += (char)message[i];
  }
  printf("\n");

  if (string(topic) == "de/lab@home/lightControl") {
    printf("Changing LED output to ");
    if(messageTemp == "on"){
      printf("on\n");
      leds[0] = CRGB::DarkOliveGreen;
      FastLED.show();
    }
    else if(messageTemp == "off"){
      printf("off\n");
      leds[0] = CRGB::Black;
      FastLED.show();
    }
    else{
      printf("Message couldn't be handled\n");
    }
  } 
  else if (string(topic) == "de/heizungsanlage/data"){
    if(messageTemp == "true"){
      printf("Subscribe to topic \"de/heizungsanlage/data/+\"\n");
      pubSubClient.subscribe("de/heizungsanlage/data/+");
    }
    else if(messageTemp == "false"){
      printf("Unsubscribe from topic \"de/heizungsanlage/data/+\"\n");
      pubSubClient.unsubscribe("de/heizungsanlage/data/+");
    }
    else{
      printf("Message couldn't be handled\n");
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

// Getters
mqttConfiguration Communication::getMQTTConfig() {
  return comm_mqtt_config;
}


// Helper functions
void printSerialNumber() {
  printf("\n## Print Serial Number ##\n");
  printf("Manufacturer ID=0x%x\n", hdc1080.readManufacturerId()); // 0x5449 ID of Texas Instruments
	printf("Device ID=0x%x\n", hdc1080.readDeviceId()); // 0x1050 ID of the device

	HDC1080_SerialNumber sernum = hdc1080.readSerialNumber();
	char format[40];
	sprintf(format, "%02X-%04X-%04X", sernum.serialFirst, sernum.serialMid, sernum.serialLast);
	printf("Device Serial Number=%s\n\n", format);
}