"""
This script receives via MQTT an string like:

"b'20201014222901,22.05,98612.67,21.80,80.23,N,9.72,1.75,1'"

Add this data to the dadosMQTT list (splitted by comma),
where the data is:

dadosMQTT[0] = date_and_time
dadosMQTT[1] = bmp_temperature
dadosMQTT[2] = bmp_pressure
dadosMQTT[3] = sht_temperature
dadosMQTT[4] = sht_humidity
dadosMQTT[5] = wind_direction
dadosMQTT[6] = anemometer(km/h)
dadosMQTT[7] = pluviometer
dadosMQTT[8] = sd_status

Then the data is verified and prepared in a JSON that is sent
to the InfluxDB database.

reference: https://diyi0t.com/visualize-mqtt-data-with-influxdb-and-grafana/
"""
import paho.mqtt.client as mqtt
from influxdb import InfluxDBClient

# Add your InfluxDB parameters
INFLUXDB_ADDRESS = 'ipAddressOrServer.com'
INFLUXDB_USER = 'YourInfluxdbUser'
INFLUXDB_PASSWORD = 'YourInfluxdbPassword'
INFLUXDB_DATABASE = 'ws_test' # If you chanche this name, need to change the name of database and the name on grafana json/dashboard

# Add your MQTT broker parameters
MQTT_ADDRESS = 'yourMQTTserver.com'
MQTT_USER = 'yourMQTTuser'
MQTT_PASSWORD = 'yourMQTTpassword'
MQTT_TOPIC = 'ws/esp' # The same topic used by the ESP8266 to upload the data
MQTT_CLIENT_ID = 'MQTTInfluxDBBridge'

influxdb_client = InfluxDBClient(INFLUXDB_ADDRESS, 8086, INFLUXDB_USER, INFLUXDB_PASSWORD, None)

def on_connect(client, userdata, flags, rc):
    """ The callback for when the client receives a CONNACK response from the server."""
    print('Connected with result code ' + str(rc))
    client.subscribe(MQTT_TOPIC)

def on_message(client, userdata, msg):
    """The callback for when a PUBLISH message is received from the server."""
    print(msg.topic + ' ' + str(msg.payload))
    
    dadosMQTT = str(msg.payload).split(",")
    _mqtt_data_to_influxdb(dadosMQTT)

def _mqtt_data_to_influxdb(dadosMQTT):

    # b'20200614194528 => 20200614194528
    dadosMQTT[0] = dadosMQTT[0][2:]

    # "20200614094518" => "2020-06-14T09:45:18Z"
    esp8266_time = str(dadosMQTT[0])    #20200614194528
    # 20200614 -> 2020-06-14T
    measurement_time = esp8266_time[0:4]+"-"+esp8266_time[4:6]+"-"+esp8266_time[6:8]+"T" 
    # 094518 -> T09:45:18Z
    measurement_time = measurement_time+esp8266_time[8:10]+":"+esp8266_time[10:12]+":"+esp8266_time[12:]+"Z"
    print(measurement_time)
 
    # Wind Direction Output
    #     EN    |    PT    | Output | Degrees
    # North     | Norte    | N      | 0/360
    # Northeast | Nordeste | NE     | 45
    # East      | Leste    | E      | 90
    # Southeast | Sudeste  | SE     | 135
    # South     | Sul      | S      | 180
    # Southwest | Sudoeste | SW     | 225
    # West      | Oeste    | W      | 270
    # Northwest | Noroeste | NW     | 315

    if str(dadosMQTT[5]) == "N":
        wind_direction = 0
    elif str(dadosMQTT[5]) == "NE":
        wind_direction = 45
    elif str(dadosMQTT[5]) == "E":
        wind_direction = 90
    elif str(dadosMQTT[5]) == "SE":
        wind_direction = 135
    elif str(dadosMQTT[5]) == "S":
        wind_direction = 180
    elif str(dadosMQTT[5]) == "SW":
        wind_direction = 225
    elif str(dadosMQTT[5]) == "W":
        wind_direction = 270
    elif str(dadosMQTT[5]) == "NW":
        wind_direction = 315
    # 404 error -> if Attiny send null
    elif str(dadosMQTT[5]) == "null":
        wind_direction = 404
    # 405 error -> if Attiny not send data via serial to ESP8266
    elif str(dadosMQTT[5]) == "snr":
        wind_direction = 405

    # Str, etc, to Float
    anemometer = float(dadosMQTT[6])
    temperature_bmp = float(dadosMQTT[1])
    pressure_bmp = float(dadosMQTT[2])*0.01
    temperature_sht = float(dadosMQTT[3])
    humidity_sht = float(dadosMQTT[4])

    # When ESP8266 not receive pluviometer Attiny data via serial set as null
    if str(dadosMQTT[7]) == "snr":
        pluviometer_raw = "null"
    else:
        pluviometer_raw = float(dadosMQTT[7])

    # 2' => 2 (sd_status)
    dadosMQTT[8] = dadosMQTT[8][0:1]
    wemos_memory_status = int(dadosMQTT[8])

    # JSON to InfluxDB
    json_body = [
        {
            'measurement': "anemometer",
            'tags': {
                'location': "ws01"
            },
            "time": measurement_time,
            'fields': {
                'value': anemometer
            }
        },
        {
            'measurement': "temperature_bmp",
            'tags': {
                'location': "ws01"
            },
            "time": measurement_time,
            'fields': {
                'value': temperature_bmp
            }
        },
        {
            'measurement': "pressure_bmp",
            'tags': {
                'location': "ws01"
            },
            "time": measurement_time,
            'fields': {
                'value': pressure_bmp
            }
        },
        {
            'measurement': "temperature_sht",
            'tags': {
                'location': "ws01"
            },
            "time": measurement_time,
            'fields': {
                'value': temperature_sht
            }
        },
        {
            'measurement': "humidity_sht",
            'tags': {
                'location': "ws01"
            },
            "time": measurement_time,
            'fields': {
                'value': humidity_sht
            }
        },
        {
            'measurement': "pluviometer_raw",
            'tags': {
                'location': "ws01"
            },
            "time": measurement_time,
            'fields': {
                'value': pluviometer_raw
            }
        },
        {
            'measurement': "wind_direction",
            'tags': {
                'location': "ws01"
            },
            "time": measurement_time,
            'fields': {
                'value': wind_direction
            }
        },
        {
            'measurement': "wemos_memory_status",
            'tags': {
                'location': "ws01"
            },
            "time": measurement_time,
            'fields': {
                'value': wemos_memory_status
            }
        }
    ]
    a = influxdb_client.write_points(json_body)
    print("Status: ")
    print(a)


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
    print('MQTT-InfluxDB')
    main()
