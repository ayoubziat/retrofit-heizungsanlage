#include <PubSubClient.h>
#include <WiFi.h>
#include <util.h>

// LED Pin
const int ledPin = 4;

Communication::Communication(mqttConfiguration config)
: mqtt_config(config)
{
    printf("## Init Communication Class ## \n");
}

void Communication::setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  printf("### Setup WiFi ###\n");
  printf("\n\tConnecting to ");
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
  printf("\n\tIP address: %s\n", buf);
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

  if (String(topic) == mqtt_config.mqtt_topic) {
    Serial.print("Changing output to ");
    if(messageTemp == "on"){
      Serial.println("on");
      digitalWrite(ledPin, HIGH);
    }
    else if(messageTemp == "off"){
      Serial.println("off");
      digitalWrite(ledPin, LOW);
    }
  }
}
