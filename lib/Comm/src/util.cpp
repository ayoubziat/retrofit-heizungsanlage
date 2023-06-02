#include "util.h"
#include "configuration.h"
#include "dht_sensor.h"

#include <FastLED.h>
#include <WiFiMulti.h>
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>


WiFiClient deviceClient;
DHTSensor dhtSensor;
PubSubClient pubSubClient(deviceClient);
WiFiMulti wifiMulti;

// InfluxDB client instance with preconfigured InfluxCloud certificate
InfluxDBClient influxDBClient(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);

// Data points
Point dhtSensorPoint("DHTSensor");
Point heizungsanlagePoint("Heinzungsanlage");

int64_t nextTime{0};
CRGB leds[NUM_LEDS];

static boolean PUBLISH_DHT_DATA = true;
static boolean PUBLISH_HEINZUNGSANLAGE_DATA = true;

Communication::Communication(struct mqttConfiguration config){
    comm_mqtt_config = config;
}

void Communication::setup(){
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS); // GRB ordering is assumed
  dhtSensor.setup();
  delay(1000);
  Communication::setup_wifi();

  // Add tags
  dhtSensorPoint.addTag("device", DEVICE);
  heizungsanlagePoint.addTag("device", DEVICE);

  // Accurate time is necessary for certificate validation and writing in batches
  // For the fastest time sync find NTP servers in your area: https://www.pool.ntp.org/zone/
  // Syncing progress and the time will be printed to Serial.
  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");

  // Check server connection
  if (influxDBClient.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(influxDBClient.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(influxDBClient.getLastErrorMessage());
  }

  pubSubClient.setServer(comm_mqtt_config.mqtt_server, comm_mqtt_config.mqtt_port);
	pubSubClient.setCallback(mqttCallback);
  Communication::reconnect();
}

void Communication::setup_wifi() {
  // We start by connecting to a WiFi network
  Serial.println("-----------------------------------------------");
  Serial.print("### Setup WiFi ###\n");
  Serial.print("Connecting to ");
  Serial.println(comm_mqtt_config.ssid);
  // Setup wifi connection
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(comm_mqtt_config.ssid, comm_mqtt_config.password);
  // Connect to WiFi
  while (wifiMulti.run() != WL_CONNECTED){
    Serial.print(".");
    delay(100);
  }
  char buf[16];
  IPAddress ip = WiFi.localIP();
  sprintf(buf, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
  Serial.printf("\nWiFi connected! IP address: %s\n", buf);
  Serial.println("-----------------------------------------------");
}

void Communication::reconnect() {
  Serial.println("-----------------------------------------------");
  Serial.println("## Reconnect ##");
  // Loop until we're reconnected
  while (!pubSubClient.connected()) {
    Serial.print("Attempting MQTT connection...\n\t");
    // Attempt to connect
    if (pubSubClient.connect("Lab@home")) {
      Serial.println("Device connected");
      // Subscribe to topics
      for(string mqtt_topic: comm_mqtt_config.mqtt_subscribe_topics){
        Serial.printf("Subscribing to MQTT topic: %s\n", mqtt_topic.c_str());
        pubSubClient.subscribe(mqtt_topic.c_str());
      }
      break;
    } else {
      Serial.printf("Failed, rc=%d. Try again in 2 seconds\n", pubSubClient.state());
      // Wait 2 seconds before retrying
      delay(2000);
    }
  }
  Serial.println("-----------------------------------------------");
}

void Communication::mqttCallback(char* topic, byte* message, unsigned int length){
  Serial.println("-----------------------------------------------");
  Serial.println("## MQTT Callback ##");
  Serial.printf("Message arrived on topic: %s \n", topic);
  String msg;
  for (int i = 0; i < length; i++) {
    msg += (char)message[i];
  }
  Serial.printf("Message is: %s\n", msg);
  if (string(topic) == "de/lab@home/lightControl") {
    Serial.printf("Changing LED output to %s\n", msg);
    if(msg == "on"){
      leds[0] = CRGB::DarkOliveGreen;
      FastLED.show();
    }
    else if(msg == "off"){
      leds[0] = CRGB::Black;
      FastLED.show();
    }
    else{
      Serial.print("Message couldn't be handled\n");
    }
  } 
  else if (string(topic) == "de/lab@home/dht/publishData"){
    if(msg == "true"){
      Serial.print("Publish DHT sensor data\n");
      PUBLISH_DHT_DATA = true;
    }
    else if(msg == "false"){
      Serial.print("Stop publish DHT sensor data\n");
      PUBLISH_DHT_DATA = false;
    }
    else{
      Serial.print("Message couldn't be handled\n");
    }
  }
  else if (string(topic) == "de/heizungsanlage/data/publishData"){
    if(msg == "true"){
      Serial.print("Publish Heinzungsanlage data\n");
      PUBLISH_HEINZUNGSANLAGE_DATA = true;
    }
    else if(msg == "false"){
      Serial.print("Stop publish Heinzungsanlage data\n");
      PUBLISH_HEINZUNGSANLAGE_DATA = false;
    }
    else{
      Serial.print("Message couldn't be handled\n");
    }
  }
  Serial.println("-----------------------------------------------");
}

void Communication::loop(){
  // Check WiFi connection and reconnect if needed
  if (wifiMulti.run() != WL_CONNECTED) {
    Serial.println("Wifi connection lost");
    while (wifiMulti.run() != WL_CONNECTED){
      Serial.print(".");
      delay(100);
    }
  }
  
  if (!pubSubClient.connected())
    Communication::reconnect();

	pubSubClient.loop();
	long current_time = millis();
	if (current_time - nextTime > TIME_INTERVAL) {
		nextTime = current_time;

    if (PUBLISH_DHT_DATA){
      // Clear fields for reusing the point. Tags will remain untouched
      dhtSensorPoint.clearFields();

      float dhtHumidity = dhtSensor.measureHumidity();
      float dhtTemp = dhtSensor.measureTemperature();
      // Check if read failed.
      if (isnan(dhtHumidity) || isnan(dhtTemp)) {
        Serial.println(F("Failed to read from DHT sensor"));
      }
      else{
        char dht_json_Content[80];
        sprintf(dht_json_Content, "{\"humidity\":\"%f\", \"temperature\":\"%f\"}", dhtHumidity, dhtTemp);
        Serial.print("DHT measurements: ");
        Serial.println(dht_json_Content);
        if(!pubSubClient.publish("de/lab@home/dht/data", dht_json_Content))
          printf("Error while publishing the DHT Sensor values!\n");

        // Store measured value into point
        dhtSensorPoint.addField("temperature", dhtTemp);
        dhtSensorPoint.addField("humidity", dhtHumidity);
        // Write data point
        if (!influxDBClient.writePoint(dhtSensorPoint)){
          Serial.print("InfluxDB writing DHT Sensor values failed: ");
          Serial.println(influxDBClient.getLastErrorMessage());
        }

        // ---------- Simulation code -------------
        heizungsanlagePoint.clearFields();
        // Store measured value into point
        heizungsanlagePoint.addField("kesseltemperatureist", dhtTemp);
        heizungsanlagePoint.addField("kesseltemperaturesoll", dhtTemp);
        heizungsanlagePoint.addField("partytemperaturesoll", dhtTemp);
        heizungsanlagePoint.addField("sparbetrieb", 0);
        heizungsanlagePoint.addField("betriebsart", 0);
        // Write data point
        if (!influxDBClient.writePoint(heizungsanlagePoint)){
          Serial.print("InfluxDB writing Heinzungsanlage values failed: ");
          Serial.println(influxDBClient.getLastErrorMessage());
        }
      }
    }
	}
}

void Communication::publish(struct dataPoint point){
  if (!PUBLISH_HEINZUNGSANLAGE_DATA)
    return ;

  Serial.println("-----------------------------------------------");
  Serial.println("### Publish: ###");
  Serial.printf("%s = %d \n", point.name, point.value);
  // Convert the values to a char array
  char valueString[8];
  sprintf(valueString, "%d", point.value);

  // Clear fields for reusing the point. Tags will remain untouched
  heizungsanlagePoint.clearFields();

  if(point.name == "03_Kesseltemp_(S3)"){
    // Store measured value into point
    heizungsanlagePoint.addField("kesseltemperatureist", point.value);
    if(!pubSubClient.publish("de/heizungsanlage/data/kesseltemperatureist", valueString))
      Serial.print("Error while publishing the kesseltemperatureist value!\n");
  }
  else if(point.name == "17_Kesseltemp_Soll"){
    // Store measured value into point
    heizungsanlagePoint.addField("kesseltemperaturesoll", point.value);
    if(!pubSubClient.publish("de/heizungsanlage/data/kesseltemperaturesoll", valueString))
      Serial.print("Error while publishing the kesseltemperaturesoll value!\n");
  }
  else if(point.name == "19_Betriebsart"){
    // Store measured value into point
    heizungsanlagePoint.addField("betriebsart", point.value);
    if(!pubSubClient.publish("de/heizungsanlage/data/betriebsart", valueString))
      Serial.print("Error while publishing the betriebsart value!\n");
  }
  else if(point.name == "20_Sparbetrieb"){
    // Store measured value into point
    heizungsanlagePoint.addField("sparbetrieb", point.value);
    if(!pubSubClient.publish("de/heizungsanlage/data/sparbetrieb", valueString))
      Serial.print("Error while publishing the sparbetrieb value!\n");
  }
  else if(point.name == "Party Temperatur Soll"){
    // Store measured value into point
    heizungsanlagePoint.addField("partytemperaturesoll", point.value);
    if(!pubSubClient.publish("de/heizungsanlage/data/partytemperaturesoll", valueString))
      Serial.print("Error while publishing the partytemperaturesoll value!\n");
  }

  // Write data point
  if (!influxDBClient.writePoint(heizungsanlagePoint)){
    Serial.print("InfluxDB writing Heinzungsanlage values failed: ");
    Serial.println(influxDBClient.getLastErrorMessage());
  }
  Serial.println("-----------------------------------------------");
}

// Helper functions
// void printSerialNumber() {
//   Serial.println("-----------------------------------------------");
//   Serial.println("## Print Serial Number ##");
//   Serial.printf("Manufacturer ID=0x%x\n", hdc1080.readManufacturerId()); // 0x5449 ID of Texas Instruments
// 	Serial.printf("Device ID=0x%x\n", hdc1080.readDeviceId()); // 0x1050 ID of the device

// 	HDC1080_SerialNumber sernum = hdc1080.readSerialNumber();
// 	char format[40];
// 	sprintf(format, "%02X-%04X-%04X", sernum.serialFirst, sernum.serialMid, sernum.serialLast);
// 	Serial.printf("Device Serial Number=%s\n", format);
//   Serial.println("-----------------------------------------------");
// }