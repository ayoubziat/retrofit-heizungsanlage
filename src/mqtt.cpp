#include <PubSubClient.h>
#include <WiFi.h>

class MQTTClass {
    private:
        WiFiClient espCLient;
        PubSubClient pubSubClient;
    public:
        struct mqtt
        {
            /* data */
            const char* ssid;
            const char* password;
            const char* mqttServer;
            const char* mqttTopic;
        };
};
