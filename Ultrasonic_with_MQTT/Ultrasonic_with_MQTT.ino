#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Ultrasonic.h>

Ultrasonic ultrasonic(5, 4); //trig,echo d1,d2

#define wifi_ssid "Xiaomi_2F86"
#define wifi_password "95418086478750819950"

#define mqtt_server "192.168.143.200"
#define mqtt_clientid "ULTRASONIC"

#define distance_topic "Home/Distance"                          // Distance between sensor and surface (cm)
#define water_level_topic "Home/Garden/Cistern/WaterLevel" // Water level (cm)
#define fill_level_topic "Home/Garden/Cistern/FillLevel"   // Fill level of the cistern (%)
#define content_topic "Home/Garden/Cistern/Content"        // Content of the cistern (Liters)

int distance_empty = 142; // Distance between sensor and cistern bottom
float water_max = 132;    // Max. water level (distance from bottom to top edge of siphon)
int total_content = 5000; // Maximum total content of the cistern in liters

WiFiClient espClient;
PubSubClient client(espClient);

void setup()
{
    Serial.begin(115200);
    setup_wifi();
    client.setServer(mqtt_server, 1883);
}

void setup_wifi()
{
    delay(10);
    // We start by connecting to a WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(wifi_ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifi_ssid, wifi_password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void reconnect()
{
    // Loop until we're reconnected
    while (!client.connected())
    {
        Serial.print("Attempting MQTT connection...");
        // Attempt to connect
        // If you do not want to use a username and password, change next line to
        if (client.connect(mqtt_clientid))
        // if (client.connect(mqtt_clientid, mqtt_user, mqtt_password)) {
        {
            Serial.println("connected");
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

void loop()
{
    if (!client.connected())
    {
        reconnect();
    }
    client.loop();
    int Distance = ultrasonic.distanceRead();
    Serial.println(Distance);
    int WaterLevel = distance_empty - Distance;
    int fill_level = (WaterLevel / water_max) * 100;
    int content = (total_content / 100) * fill_level;
    /*
      Serial.println("Distance cm: ");
      Serial.println(Distance);
      Serial.println("Water level cm: ");
      Serial.println(WaterLevel);
      Serial.println("Fill level in percentage: ");
      Serial.println(fill_level);
      Serial.println("Content in Liters: ");
      Serial.println(content);
    */
    if(Distance <= 21){
    client.publish(distance_topic, String(Distance).c_str(), true);
    client.publish(water_level_topic, String(WaterLevel).c_str(), true);
    client.publish(fill_level_topic, String(fill_level).c_str(), true);
    client.publish(content_topic, String(content).c_str(), true);
    delay(100); // Check every 60.000 ms = 60 s = 1 min
    // delay(10000); // Check every 10.000 ms = 10 s
    }
}
