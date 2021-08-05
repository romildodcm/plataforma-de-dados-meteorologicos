def station_id(mqtt_topic: str) -> str:
    """
    Returns the id of the weather station based on mqtt topic.
    The id is for the database location tag.

    Parameters:
        mqtt_topic (str): weather station mqtt topic

    Returns:
        weather station id
    """

    # Topic is a string like "ws/esp0", the number after "esp" is the id
    if mqtt_topic[-1].isdigit():
        # If number == 0 (zero) is a test weather station
        if mqtt_topic[6:] == '0':
            id = 'ws_test'
        else:
            # ws/espX when X > 1, stations in production
            id = 'ws0' + mqtt_topic[6:]
    else:
        id = 'ws_not_identified'

    return id


def measurement_time_convert(time_parameter: str) -> str:
    """
    Converts time to ISO 8601 standard:
    "20200614094518" -> "2020-06-14T09:45:18Z".

    Parameters:
        time_parameter (str): measurement time received via MQTT

    Returns:
        measurement time on ISO 8601 standard
    """

    # Gets "20200614094518"
    # 20200614 to 2020-06-14T
    iso_timestamp = time_parameter[0:4]+'-' + \
        time_parameter[4:6]+'-'+time_parameter[6:8]+'T'
    # 094518 to 09:45:18Z
    iso_timestamp = iso_timestamp + \
        time_parameter[8:10]+':'+time_parameter[10:12] + \
        ':'+time_parameter[12:]+'Z'
    # Returns '2020-06-14T09:45:18Z'
    return iso_timestamp


def checks_measurement(measurement: str, expected_type: str):
    """
    Checks if the measurement received by the ESP8266 is a valid or a serial
    communication failure occurred between ESP8266 and others microcontrollers.
    If failure occurred the value received is equals "snr" (serial not received)

    Parameters:
        measurement received via MQTT (str)

    Returns:
        measurement (null, int, float or str)
    """
    # if 'snr' or other error
    if not(measurement.replace('.', '', 1).isdigit()):
        measurement_verified = 'null'
    elif expected_type == 'int':
        measurement_verified = int(measurement)
    elif expected_type == 'float':
        measurement_verified = float(measurement)

    return measurement_verified


def wind_convert(wind: str) -> int:
    """
    Convert direction to degrees, "S" -> 180.

    Parameters:
        wind (str): wind direction measured 

    Returns:
        wind direction measure in degrees (int) or "null" for error
    """

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

    if str(wind) == 'N':
        wind_int_degrees = 0
    elif str(wind) == 'NE':
        wind_int_degrees = 45
    elif str(wind) == 'E':
        wind_int_degrees = 90
    elif str(wind) == 'SE':
        wind_int_degrees = 135
    elif str(wind) == 'S':
        wind_int_degrees = 180
    elif str(wind) == 'SW':
        wind_int_degrees = 225
    elif str(wind) == 'W':
        wind_int_degrees = 270
    elif str(wind) == 'NW':
        wind_int_degrees = 315
    else:
        # 'null' read error by ATtiny85
        # 'snr' send serial error from ATtiny85 to ESP8266, etc
        wind_int_degrees = 'null'

    return wind_int_degrees


def data_processing(data_csv: str, weather_station: str) -> list:
    """
    Receives a csv string like:
    "20210726153318,38.64,98771.21,27.89,39.55,N,21.28,140.75,1" or
    "20210724201304,27.25,99085.42,27.26,45.89,NW,0.00,0.00,snr,snr,1",
    process the data and returns a JSON for database.

    Parameters:
        data_csv (str): data received via MQTT
        weather_station (str): MQTT topic/weather station id

    Returns:
        dict/json to write on database
    """
    data_names_9 = [
        [
            'measurement_time',
            'temperature_bmp',
            'pressure_bmp',
            'temperature_sht',
            'humidity_sht',
            'wind_direction',
            'anemometer',
            'pluviometer_raw',
            'sd_memory_status'
        ],
        [
            'str',
            'float',
            'float',
            'float',
            'float',
            'int',
            'float',
            'float',
            'int'
        ]
    ]

    data_names_11 = [
        [
            'measurement_time',
            'temperature_bmp',
            'pressure_bmp',
            'temperature_sht',
            'humidity_sht',
            'wind_direction',
            'anemometer',
            'pluviometer_raw',
            'serial_sensor1',
            'serial_sensor2',
            'sd_memory_status'
        ],
        [
            'str',
            'float',
            'float',
            'float',
            'float',
            'int',
            'float',
            'float',
            'int',
            'int',
            'int'
        ]
    ]

    data_list_of_dict = []
    data = data_csv.split(',')
    data_length = len(data)
    measurement_time = measurement_time_convert(data[0])
    measurement = checks_measurement(data[2], data_names_9[1][2])
    if measurement != 'null':
        # Converts to hPa (100 x 1 pascal)
        data[2] = '{:.4f}'.format(float(data[2])*0.01)
    data[5] = str(wind_convert(data[5]))

    # If no solar data length = 9, if with solar data length = 11
    if data_length == 9:
        for i in range(1, 9):
            measurement = checks_measurement(data[i], data_names_9[1][i])

            if measurement != 'null':
                measurement_stamp = {
                    'measurement': data_names_9[0][i],
                    'tags': {
                        'location': weather_station
                    },
                    'time': measurement_time,
                    'fields': {
                        'value': measurement
                    }
                }

                data_list_of_dict.append(measurement_stamp.copy())
            else:
                print(
                    f'Invalid {data_names_9[0][i]}: {data[i]} => {measurement}')

    elif data_length == 11:
        for i in range(1, 11):
            measurement = checks_measurement(data[i], data_names_11[1][i])

            if measurement != 'null':
                measurement_stamp = {
                    'measurement': data_names_11[0][i],
                    'tags': {
                        'location': weather_station
                    },
                    'time': measurement_time,
                    'fields': {
                        'value': measurement
                    }
                }

                data_list_of_dict.append(measurement_stamp.copy())
            else:
                print(
                    f'Invalid {data_names_11[0][i]}: {data[i]} => {measurement}')

    return data_list_of_dict