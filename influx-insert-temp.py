import json
import paho.mqtt.client as mqtt
from influxdb_client import InfluxDBClient, Point, WritePrecision
from influxdb_client.client.write_api import SYNCHRONOUS
from datetime import datetime

# Configurazione MQTT
MQTT_BROKER = "localhost"
MQTT_PORT = 1883
MQTT_TOPIC_HUMIDITY = "sensor/humidity"
MQTT_TOPIC_TEMPERATURE = "sensor/temperature_celsius"

# Configurazione InfluxDB
INFLUXDB_URL = "http://localhost:8086"
INFLUXDB_TOKEN = "Z3TMySTq9rLgYA8RMNrWs_0cme3Rgo44ouTUNm7o2tf8zdpyQTboEnIkItQQGTQbSKzy8wZSmLjVaQoKvZCIJQ=="
INFLUXDB_ORG = "lolo"
INFLUXDB_BUCKET = "temp"

# Creazione del client InfluxDB
client = InfluxDBClient(url=INFLUXDB_URL, token=INFLUXDB_TOKEN, org=INFLUXDB_ORG)
write_api = client.write_api(write_options=SYNCHRONOUS)

def on_connect(client, userdata, flags, rc):
    print(f"Connesso a MQTT Broker! Codice di risultato: {rc}")
    client.subscribe([(MQTT_TOPIC_HUMIDITY, 0), (MQTT_TOPIC_TEMPERATURE, 0)])

def on_message(client, userdata, msg):
    print(f"Messaggio ricevuto su {msg.topic}: {msg.payload.decode()}")
    data = msg.payload.decode()
    measurement = "unknown"

    if msg.topic == MQTT_TOPIC_HUMIDITY:
        measurement = "humidity"
    elif msg.topic == MQTT_TOPIC_TEMPERATURE:
        measurement = "temperature"

    try:
        # Tenta di parsare i dati come JSON
        data_dict = json.loads(data)
        if isinstance(data_dict, dict):
            print(f"Parsed data: {data_dict}")
            point = Point(measurement)
            for key, value in data_dict.items():
                if isinstance(value, str):
                    print(f"Adding tag: {key}={value}")
                    point = point.tag(key, value)
                else:
                    print(f"Adding field: {key}={value}")
                    point = point.field(key, value)
            point = point.time(datetime.utcnow(), WritePrecision.NS)
        else:
            # Se i dati parsati non sono un dizionario, trattali come un singolo campo
            raise ValueError("Parsed JSON is not a dictionary")
    except (json.JSONDecodeError, ValueError):
        # Se i dati non sono JSON o il JSON non è un dizionario, trattali come un singolo campo
        print(f"Data is not JSON or not a dictionary, treating as single field: {data}")
        try:
            value = float(data)
        except ValueError:
            value = data  # Se non è convertibile a float, trattalo come stringa
        point = Point(measurement).field("value", value).time(datetime.utcnow(), WritePrecision.NS)
    
    print(f"Writing point: {point}")
    write_api.write(bucket=INFLUXDB_BUCKET, org=INFLUXDB_ORG, record=point)

def main():
    mqtt_client = mqtt.Client()
    mqtt_client.on_connect = on_connect
    mqtt_client.on_message = on_message

    mqtt_client.connect(MQTT_BROKER, MQTT_PORT, 60)
    mqtt_client.loop_forever()

if __name__ == "__main__":
    main()
