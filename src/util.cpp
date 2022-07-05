#include <PubSubClient.h>
#include <WiFi.h>
#include <util.h>

// LED Pin
const int ledPin = 4;
mqttConfiguration mqttConfig = {
  "Ayoubs iPhone",
  "aziat2294",
  "broker.hivemq.com",
  "de/heizungsanlage/data"
};

Communication::Communication()
{
    printf("## Init Communication Class ## \n");
    mqtt_config = mqttConfig;
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
  printf("Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    printf("%c", (char)message[i]);
    messageTemp += (char)message[i];
  }
  printf("\n");

  if (String(topic) == "de/klaus-liebler/lightControl") {
    Serial.print("Changing output to ");
    if(messageTemp == "on"){
      printf("on\n");
      digitalWrite(ledPin, HIGH);
    }
    else if(messageTemp == "off"){
      printf("off\n");
      digitalWrite(ledPin, LOW);
    }
  }
}

void Communication::reconnect(PubSubClient client) {
  // Loop until we're reconnected
  while (!client.connected()) {
    printf("Attempting MQTT connection...\n\t");
    // Attempt to connect
    if (client.connect("Klaus-Liebler-Client")) {
      printf("Device connected\n");
      // Subscribe
      client.subscribe("de/klaus-liebler/roomdata");
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
