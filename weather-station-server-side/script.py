import os
from dotenv import load_dotenv
import paho.mqtt.client as mqtt
from influxdb import InfluxDBClient
from weather_station import station_id, data_processing, measurement_time_convert

load_dotenv()
INFLUXDB_ADDRESS = os.getenv('INFLUXDB_ADDRESS')
INFLUXDB_USER = os.getenv('INFLUXDB_USER')
INFLUXDB_PASSWORD = os.getenv('INFLUXDB_PASSWORD')
MQTT_ADDRESS = os.getenv('MQTT_ADDRESS')
MQTT_USER = os.getenv('MQTT_USER')
MQTT_PASSWORD = os.getenv('MQTT_PASSWORD')
MQTT_TOPIC = os.getenv('MQTT_TOPIC')
INFLUXDB_DATABASE = os.getenv('INFLUXDB_DATABASE')
MQTT_CLIENT_ID = os.getenv('MQTT_CLIENT_ID')

influxdb_client = InfluxDBClient(
    INFLUXDB_ADDRESS, 8086, INFLUXDB_USER, INFLUXDB_PASSWORD, None)


def on_connect(client, userdata, flags, rc):
    """ The callback for when the client receives a CONNACK response from the server."""
    print('Connected with result code ' + str(rc))
    client.subscribe(MQTT_TOPIC)


def on_message(client, userdata, msg):
    """The callback for when a PUBLISH message is received from the server."""
    # Gets the MQTT message topic, unique of each weather station
    mqtt_topic = str(msg.topic)

    # Gets the MQTT data and clear
    # "b'20210726153318,38.64,98771.21,27.89,39.55,N,21.28,140.75,1'" or
    # "b'20210724201304,27.25,99085.42,27.26,45.89,NW,0.00,0.00,snr,snr,1'"
    # to
    # "20210726153318,38.64,98771.21,27.89,39.55,N,21.28,140.75,1" or
    # "20210724201304,27.25,99085.42,27.26,45.89,NW,0.00,0.00,snr,snr,1"
    mqtt_data = str(msg.payload)[2:]
    mqtt_data = mqtt_data[:-1]
    _mqtt_to_influxdb(mqtt_data, mqtt_topic)


def _mqtt_to_influxdb(data, weather_station_topic):
    weather_station_id = station_id(weather_station_topic)
    data_len = data.count(',')+1

    # Log of measurement time, location tag and received data
    print(32*'----')
    print('Measurement time: ' + measurement_time_convert(data[0:14]))
    print('Weather station: ' + weather_station_id)
    print('Message length: ' + str(data_len))
    print('Received data: ' + data)

    if (data_len == 9) or (data_len == 11):
        json_body = data_processing(data, weather_station_id)
        print('Writing to database...')
        print(*json_body, sep='\n')
        a = influxdb_client.write_points(json_body)
        print('Status write data to Influx Database: ' + str(a))


def _init_influxdb_database():
    databases = influxdb_client.get_list_database()
    if len(list(filter(lambda x: x['name'] == INFLUXDB_DATABASE, databases))) == 0:
        influxdb_client.create_database(INFLUXDB_DATABASE)
    influxdb_client.switch_database(INFLUXDB_DATABASE)


def main():
    _init_influxdb_database()
    mqtt_client = mqtt.Client(MQTT_CLIENT_ID)
    mqtt_client.username_pw_set(MQTT_USER, MQTT_PASSWORD)
    mqtt_client.on_connect = on_connect
    mqtt_client.on_message = on_message
    mqtt_client.connect(MQTT_ADDRESS, 12136)
    mqtt_client.loop_forever()


if __name__ == '__main__':
    print('MQTT to InfluxDB script')
    main()
