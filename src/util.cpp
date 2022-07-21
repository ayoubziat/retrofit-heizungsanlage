#include <PubSubClient.h>
#include <WiFi.h>
#include <util.h>

// LED Pin
const int ledPin = 4;

// MQTT Ccnfiguration
mqttConfiguration mqttConfig = {
  "ssid",
  "pw",
  "broker.hivemq.com",
  1883,
  {"de/heizungsanlage/data/temperature", "de/heizungsanlage/data/humidity"},
  {"de/heizungsanlage/lightControl"}
};

Communication::Communication()
{
    printf("## Init Communication Class ## \n");
    mqtt_config = mqttConfig;
}

void Communication::setup(){
  pinMode (ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);
  delay(2000);
  digitalWrite(ledPin, LOW);
}

void Communication::setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  printf("\n\n### Setup WiFi ###\n");
  printf("\tConnecting to ");
  printf(mqtt_config.ssid);
  WiFi.begin(mqtt_config.ssid, mqtt_config.password);
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

void Communication::callback(char* topic, byte* message, unsigned int length) {
  printf("## Callback ##\n");
  printf("Message arrived on topic: %s.\n", topic);
  printf("Message is: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    printf("%c", (char)message[i]);
    messageTemp += (char)message[i];
  }
  printf("\n");

  for(string mqtt_topic: mqttConfig.mqtt_subscribe_topics){
    if (string(topic) == mqtt_topic) {
      Serial.print("Changing LED output to ");
      if(messageTemp == "on"){
        printf("on\n");
        digitalWrite(ledPin, HIGH);
      }
      else if(messageTemp == "off"){
        printf("off\n");
        digitalWrite(ledPin, LOW);
      }else{
        printf("Message couldn't be evalued\n");
      }
  }
  }
}

void Communication::reconnect(PubSubClient client) {
  // Loop until we're reconnected
  while (!client.connected()) {
    printf("Attempting MQTT connection...\n\t");
    // Attempt to connect
    if (client.connect("")) {
      printf("Device connected\n");
      // Subscribe
      client.subscribe("");
      break;
    } else {
      printf("Failed, rc=%d. Try again in 5 seconds\n", client.state());
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// Getters
mqttConfiguration Communication::getMQTTConfig() {
  return mqtt_config;
}
