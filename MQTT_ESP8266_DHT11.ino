#include <ESP8266WiFi.h>
#include <Wire.h>
#include <PubSubClient.h>
#include "DHT.h"

#define DHTPIN 12   // Digital pin connected to DHT sensor
#define DHTTYPE DHT11 // Type of DHT sensor being used

#define wifi_ssid "Xiaomi_2F86"
#define wifi_password "95418086478750819950"
#define mqtt_server "192.168.178.24"
#define mqtt_clientid "TEMP_HUMIDITY"
#define humidity_topic "sensor/humidity"
#define temperature_celsius_topic "sensor/temperature_celsius"

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  connectWiFi();
  connectMQTT();
}

void connectWiFi() {
  Serial.println("Connecting to WiFi...");
  WiFi.begin(wifi_ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.println("IP address: " + WiFi.localIP().toString());
}

void connectMQTT() {
  Serial.println("Connecting to MQTT...");
  client.setServer(mqtt_server, 1883);
  while (!client.connected()) {
    if (client.connect(mqtt_clientid)) {
      Serial.println("Connected to MQTT broker");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" Retrying in 5 seconds...");
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    connectMQTT();
  }
  client.loop();
  publishSensorData();
  delay(2000);
}

void publishSensorData() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");

  client.publish(humidity_topic, String(humidity).c_str(), true);
  client.publish(temperature_celsius_topic, String(temperature).c_str(), true);
}
