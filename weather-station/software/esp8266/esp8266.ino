/*******************************************************************************
 * STATUS: all ok.
 * 
 * CSV Description
 * 
 * 20201014222901,22.05,98612.67,21.80,80.23,N,9.72,1.75,1023,1023,1
 * date_and_time,bmp_temperature,bmp_pressure,sht_temperature,sht_humidity,wind_direction,anemometer(km/h),pluviometer,serial_sensor1,serial_sensor2,sd_status
 * 
 * STATUS SD Description
 * sd_status = 1, 2 ou 3
 * 1 = Ok
 * 2 = SD begin error
 * 3 = SD data write error
 *  
 * Need to download this library https://github.com/NorthernWidget/DS3231
****************/
// LIBRARIES
#include <ESP8266WiFi.h>     // ESP8266 Wi-Fi connection
#include <NTPClient.h>       // NTP time
#include <WiFiUdp.h>         // UDP communication for NTP time
#include <Wire.h>            // I2C connection/interface (BMP280, SHT20, RTC)
#include <Adafruit_Sensor.h> // BMP280
#include <Adafruit_BMP280.h> // BMP280
#include "DFRobot_SHT20.h"   // SHT20
#include <PubSubClient.h>    // MQTT
#include <SPI.h>             // SPI connection/interface (SD)
#include <SD.h>              // SD card SPI
#include <EEPROM.h>          // EEPROM library
#include <Time.h>            // Time library
#include <TimeLib.h>         // Libary for function days_interval(..)
#include <DS3231.h>          // RTC

// Wi-Fi parameters
const char *ssid = "REPLACE_WITH_YOUR_WIFI_SSID";
const char *password = "REPLACE_WITH_YOUR_WIFI_PASSWORD";

// MQTT broker parameters
const char *mqttServer = "yourMQTTserver.com"; // Server
const char *mqttUser = "yourMQTTuser";            // User
const char *mqttPassword = "yourMQTTpassword";    // Password
const int mqttPort = 12136;                   // Port
// const char *mqttTopicSub = "ws/esp0";          // Topic Weather Station 0 (teste)
const char *mqttTopicSub = "ws/esp1";          // Topic Weather Station 1, change the number 1 to the station number 1, 2, 3, etc

// MQTT
WiFiClient espClient;
PubSubClient client(espClient);

// NTP
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

Adafruit_BMP280 bmp; // BMP280 I2C
DFRobot_SHT20 sht20; // SHT20 I2C
DS3231 Clock;        // RTC DS3231 I2C

// Constants
const int chipSelect = 15; // 15 = ESP8266 GPIO15 = Hardware Pin D8

// Variables
int sd_status = 0, wifi_connect_attempts = 0, mqtt_connect_attempts = 0, mqtt_client_state;

bool wifi_connection_status = true; // if false, not execute functions that maybe need connection

String csv_output,
    attiny_serial_data,
    esp8266_time,
    serial_sensor_data,
    file_name; // Only '8.3' file (8character.3character), like 1234578.csv or abcdefgh.txt
float bmp_temperature,
    bmp_pressure,
    sht_temperature,
    sht_humidity;
unsigned long begin_time = 0, // #DEBUG
    sleep_time = 0;

String serial_data_request(String sensor_id = "")
{
    int request_attempt = 0, comma_index, asterisk_index, sensor_id_length;
    unsigned long wait_for_request = 0;
    String received;
    String received_measurements;

    sensor_id_length = sensor_id.length();

    // Clear serial buffer
    if (Serial.available())
    {
        // Serial.println("Had in serial before requesting sensor_id " + sensor_id + " data:");
        while (Serial.available())
        {
            received.concat(Serial.read());
        }
        received = "";
    }

    // #DEBUG
    // Serial.print("Begin sensor_id data request: ");
    // Serial.println(millis());

    while (!Serial.available() && request_attempt < 5)
    {
        request_attempt++;

        // #DEBUG
        // Serial.print("Time on while for requesting sensor_id " + sensor_id + "data: ");
        // Serial.println(millis());
        // Serial.print("Serial request attempt: " + String(request_attempt));
        // Serial.println(request_attempt);

        delay(100);
        // Sensor id
        Serial.print(String(sensor_id) + '\n');
        // Serial.println();

        wait_for_request = millis();
        // while (((millis() - wait_for_request) < 2000) && !Serial.available())
        while ((abs(millis() - wait_for_request) < 2000) && !Serial.available())
        {
            // #DEBUG
            // Serial.print("Serial delay while, serial status: ");
            // Serial.println(Serial.available());

            delay(100);
        }
    }

    if (Serial.available())
    {
        // "id1,1000,850*" -> "sensor_id id, measurement"
        received = Serial.readStringUntil('\n');
        

        if (sensor_id.equals(received.substring(0, sensor_id_length)))
        {
            comma_index = received.indexOf(",");
            asterisk_index = received.indexOf("*");

            received_measurements = received.substring(comma_index + 1, asterisk_index);

            // DEBUG
            Serial.println("Received: " + received);
            Serial.println("Measurement: " + received_measurements);

            return received_measurements;
        }
    }
    else
    {
        // If not received, "snr" = Serial Not Received
        // received.concat(sensor_id);
        received_measurements.concat("snr,snr");

        // #DEBUG
        Serial.println("Serial unavailable snr code, sends: " + received_measurements);

        return received_measurements;
    }
}

/************************************************************
 * this function returns a String with the format:
 * YYYYMMDDhhmmss
 * YYYY = year
 * MM = month
 * DD = day
 * hh = hour
 * mm = minute
 * ss = second
 * *********************************************************/
String get_time_NTP()
{
    timeClient.begin(); // NTPClient Begin
    delay(100);

    String t;
    String formatted_time;
    unsigned long epochTime;
    int year, month, day;

    timeClient.update();
    epochTime = timeClient.getEpochTime();

    // Get time in format 17:12:36
    t = timeClient.getFormattedTime();

    // Get formated time struct
    struct tm *ptm = gmtime((time_t *)&epochTime);

    // Year
    year = ptm->tm_year + 1900;
    formatted_time.concat(String(year)); // ex: 2020

    // Month
    month = ptm->tm_mon + 1;
    if (month < 10)
    {
        formatted_time.concat("0");
        formatted_time.concat(String(month)); // 06
    }
    else
    {
        formatted_time.concat(String(month)); // 12
    }

    // Day
    day = ptm->tm_mday;
    if (day < 10)
    {
        formatted_time.concat("0");
        formatted_time.concat(String(day)); // 06
    }
    else
    {
        formatted_time.concat(String(day)); // 20
    }

    // t = 17:12:36
    formatted_time.concat(String(t.substring(0, 2))); // Hour 10
    formatted_time.concat(String(t.substring(3, 5))); // Minutes 10
    formatted_time.concat(String(t.substring(6)));    // Seconds

    return formatted_time;
}

/************************************************************
* Get the time of an DS3231 RTC
* Return as an String with the YYYYMMDDhhmmss format
************************************************************/
String get_time_RTC()
{
    // RTC parameters
    bool century = false;
    bool h12Flag;
    bool pmFlag;
    // variables for month, day, hour, minute, seconds
    int yy, mm, dd, h, m, s;
    // Time string to return
    String t = "";

    // Get and formats the RTC time to YYYYMMDDhhmmss
    // Concatenates the year to the string t
    t.concat("20"); // Concatenates YY
    yy = int(Clock.getYear());
    if (yy < 10)
    {
        t.concat("0");
        t.concat(String(yy));
    }
    else
    {
        t.concat(String(yy));
    }
    // Concatenates the Month to the string t
    mm = int(Clock.getMonth(century));
    if (mm < 10)
    {
        t.concat("0");
        t.concat(String(mm));
    }
    else
    {
        t.concat(String(mm));
    }
    // Concatenates the Day to the string t
    dd = int(Clock.getDate());
    if (dd < 10)
    {
        t.concat("0");
        t.concat(String(dd));
    }
    else
    {
        t.concat(String(dd));
    }
    // Concatenates the Hour to the string t
    h = int(Clock.getHour(h12Flag, pmFlag));
    if (h < 10)
    {
        t.concat("0");
        t.concat(String(h));
    }
    else
    {
        t.concat(String(h));
    }
    // Concatenates the Minute to the string t
    m = int(Clock.getMinute());
    if (m < 10)
    {
        t.concat("0");
        t.concat(String(m));
    }
    else
    {
        t.concat(String(m));
    }
    // Concatenates the Second to the string t
    s = int(Clock.getSecond());
    if (s < 10)
    {
        t.concat("0");
        t.concat(String(s));
    }
    else
    {
        t.concat(String(s));
    }

    return t;
}

/************************************************************
* Read the time stored on EEPROM
* Input/Output time format: YYYYMMDDhhmmss
* If time_string.length == 0 -> read EEPROM and return the
*                               stored time as an string
* else -> write the string parameter on EEPROM, read to
*         verify and return the stored time as an string
************************************************************/
String eeprom_time(String time_string = "")
{
    // EEPROM.begin(256);  // Begin the EEPROM with 256 bytes size
    EEPROM.begin(512);  // Begin the EEPROM with 512 bytes size
    int byte_size = 14; // time string length bytes

    if (time_string.length() == 0)
    {
        // Read and reaturn the time on EEPROM
        for (int i = 0; i < byte_size; i++)
        {
            // Read the EEPROM byte by byte and concat on String
            time_string.concat(char(EEPROM.read(i)));
        }

        EEPROM.end(); // Close the EEPROM
        return time_string;
    }
    else
    {
        // Write the time on EEPROM
        for (int i = 0; i < byte_size; i++)
        {
            // write the String array char by char
            EEPROM.write(i, int(char(time_string.charAt(i))));
        }
        EEPROM.commit(); // save the data on EEPROM
        // Read and reaturn the time on EEPROM
        for (int i = 0; i < byte_size; i++)
        {
            // Read the EEPROM byte by byte and concat on String
            time_string.setCharAt(i, char(EEPROM.read(i)));
        }

        EEPROM.end(); // Close the EEPROM
        return time_string;
    }
}

/************************************************************
* Get the interval in days between two times.
* Input parameters: two Strings with "YYYYMMDDhhmmss" format.
*     -> The first parameter is the first\begin time;
*     -> The second is the most recent\end time.
* Output: days between two times or 32767 if negative days,
*         (int format).
*
* WARNINGS
* -> When in the first time is month with December of the last
* year and the second time is on present year (last year+1),
* had an issue that increases 1 month of days in the result.
* -> Limited to 32766 days (more than 89 years)
************************************************************/
int days_interval(String t1, String t2)
{
    int out;
    tmElements_t time_1_tes, time_2_tes;
    time_t time_1, time_2, interval;

    // Converts string to time elements structures
    // Time 1
    time_1_tes.Year = t1.substring(0, 4).toInt() - 1970;
    time_1_tes.Month = t1.substring(4, 6).toInt() - 1;
    time_1_tes.Day = t1.substring(6, 8).toInt();
    time_1_tes.Hour = t1.substring(8, 10).toInt();
    time_1_tes.Minute = t1.substring(10, 12).toInt();
    time_1_tes.Second = t1.substring(12).toInt();
    // Time 2
    time_2_tes.Year = t2.substring(0, 4).toInt() - 1970;
    time_2_tes.Month = t2.substring(4, 6).toInt() - 1;
    time_2_tes.Day = t2.substring(6, 8).toInt();
    time_2_tes.Hour = t2.substring(8, 10).toInt();
    time_2_tes.Minute = t2.substring(10, 12).toInt();
    time_2_tes.Second = t2.substring(12).toInt();

    // time to Unix time
    time_1 = makeTime(time_1_tes);
    time_2 = makeTime(time_2_tes);

    // Interval in Unix time
    interval = time_2 - time_1;

    // Check if positive number of days,
    // return error number (32766) if negative
    if (interval >= 0)
    {
        return int(interval / 86400);
    }
    else
    {
        return 32767;
    }
}

/***********************************************************
* Verify and update the RTC time if necessary
* returns 0 if not updated and 1 for updated
* 
*  Parameter: int interval of days before a new update,
*             default 20 days.
**********************************************************/
bool rtc_time_update(int interval = 20)
{
    bool update_status;
    String time_on_eeprom = "", rtc_time = "", ntp_time = "";
    int interval_in_days = 0;

    // Get the time stored on EEPROM, of the last update
    time_on_eeprom.concat(eeprom_time());
    // Get the RTC time
    rtc_time.concat(get_time_RTC());
    // Get the interval days since the last update
    interval_in_days = days_interval(time_on_eeprom, rtc_time);

    /**********************************************************
     * This if verify:
     * -> Check if had time to read on EEPROM;
     * -> if have 14 char on time_on_eeprom and rtc_time strings
     *    and are true for like a "YYYYMMDDhhmmss" format;
     * -> if time_on_eeprom begins with "20" (suport times
     *    between 2000-2099).
    **********************************************************/
    if ((time_on_eeprom.length() == 14) && (time_on_eeprom.substring(0, 2).toInt() == 20) && (rtc_time.length() == 14))
    {
        // compare the stored time with the RTC
        // -> if have more than interval days since the last update -> get_time_NTP() and update the RTC
        if (interval_in_days >= interval)
        {
            // Get the NTP time "YYYYMMDDhhmmss"
            ntp_time.concat(get_time_NTP());

            Clock.setClockMode(false); // set to 24h
            // Update/set the RTC
            Clock.setYear(byte(ntp_time.substring(2, 4).toInt()));     // Set the Year (get only last YY of YYYY, works fine for 2020+ years)
            Clock.setMonth(byte(ntp_time.substring(4, 6).toInt()));    // Set the month
            Clock.setDate(byte(ntp_time.substring(6, 8).toInt()));     // Set the day
            Clock.setHour(byte(ntp_time.substring(8, 10).toInt()));    // Set the hour
            Clock.setMinute(byte(ntp_time.substring(10, 12).toInt())); // Set the minute
            Clock.setSecond(byte(ntp_time.substring(12).toInt()));     // Set the second

            eeprom_time(get_time_RTC());

            // #DEBUG
            // Serial.println("RTC updated");

            update_status = true;
        }
        else
        {
            // #DEBUG
            // Serial.println("RTC not updated");

            update_status = false;
        }
    }
    else
    {
        // Get the NTP time "YYYYMMDDhhmmss"
        ntp_time.concat(get_time_NTP());

        Clock.setClockMode(false); // set to 24h
        // Update/set the RTC
        Clock.setYear(byte(ntp_time.substring(2, 4).toInt()));     // Set the Year (get only last YY of YYYY, works fine for 2020+ years)
        Clock.setMonth(byte(ntp_time.substring(4, 6).toInt()));    // Set the month
        Clock.setDate(byte(ntp_time.substring(6, 8).toInt()));     // Set the day
        Clock.setHour(byte(ntp_time.substring(8, 10).toInt()));    // Set the hour
        Clock.setMinute(byte(ntp_time.substring(10, 12).toInt())); // Set the minute
        Clock.setSecond(byte(ntp_time.substring(12).toInt()));     // Set the second

        eeprom_time(get_time_RTC());

        // #DEBUG
        // Serial.println("RTC updated");

        update_status = true;
    }

    // #DEBUG
    // Serial.print("Interval to update time: ");
    // Serial.println(interval);
    // Serial.print("interval days since the last update: ");
    // Serial.println(interval_in_days);

    return update_status;
}

String get_attiny_data()
{
    int serialRequest = 0;
    char character;      // character for serial buffer read
    String message = ""; // message received from ATtiny85 via serial
    unsigned long begin_delay = 0;

    // #DEBUG
    Serial.print("Begin data request: ");
    Serial.println(millis());

    // inicio código adicionado para resolver problema

    if (Serial.available())
    {
        // Serial.println("Had in serial before requesting sensor " + sensor + " data:");
        while (Serial.available())
        {
            message.concat(Serial.read());
        }
        message = "";
    }

    // fim código adicionado para resolver problema

    while (!Serial.available() && serialRequest < 3)
    {
        // #DEBUG
        Serial.print("Time on data request while: ");
        Serial.println(millis());
        Serial.print("SerialRequest: ");
        Serial.println(serialRequest);

        serialRequest++;

        // Signal to request
        digitalWrite(0, HIGH);
        delay(100); // Debouncing time
        digitalWrite(0, LOW);
        // delay for ATtiny85 data processing
        begin_delay = millis();
        // while (((millis() - begin_delay) < 7000) && !Serial.available())
        while ((abs(millis() - begin_delay) < 7000) && !Serial.available())
        {
            // #DEBUG
            Serial.print("Serial delay while, serial status: ");
            Serial.println(Serial.available());

            delay(100);
        }
        // Serial.print(millis());
        // Serial.print(",");
        // Serial.print("solicitou");
        // Serial.print(",");
    }

    if (Serial.available())
    {
        while (Serial.available())
        {
            character = Serial.read();
            if (character == '*')
            {
                // #DEBUG
                Serial.println("Received: " + message);

                return message;
            }
            else
            {
                message.concat(character);
            }
        }
    }
    else
    {
        // If not received, concatenates "snr" = Serial Not Received
        // #DEBUG
        // Serial.println("snr,snr,snr,snr,snr");
        return "snr,snr,snr";
    }
}

void reconnect_MQTT()
{
    int attempt_counter = 0;
    // Enquanto não conectar tenta novamente até 3 tentativas
    while (!client.connected() && attempt_counter < 3)
    {
        Serial.println("Connecting to the MQTT broker...");
        if (client.connect("ESP8266Client", mqttUser, mqttPassword))
        {
            Serial.println("Connected");
        }
        else
        {
            Serial.print("Error. State: ");
            Serial.println(client.state());
            delay(5000);
        }
        attempt_counter++;
    }
}

void setup()
{
    begin_time = millis(); // Gets begin time
    Serial.begin(9600);    // Begin serial communications

    // #DEBUG
    Serial.print("Setup begin time: ");
    Serial.println(millis());

    pinMode(0, OUTPUT); // Set D3 to request ATtiny85 serial data
    delay(20);          // Delay for stabilization

    // // Request ATtiny85 data [ comentado em 20210728 1253]
    // digitalWrite(0, HIGH);
    // delay(100); // Debouncing time
    digitalWrite(0, LOW);

    Wire.begin(); // RTC

    // BMP280 setup
    if (!bmp.begin(0x76))
    {
        Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
        delay(10);
        while (1)
            ;
    }
    bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                    Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                    Adafruit_BMP280::SAMPLING_X16,    /* bmp_Pressure oversampling */
                    Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                    Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

    // Init SHT20 Sensor
    sht20.initSHT20();
    delay(100);
    sht20.checkSHT20();

    // SD begin
    if (!SD.begin(chipSelect))
    {
        Serial.println("Falha ao iniciar SD)");
        sd_status = 2; // 2 = falha ao iniciar
    }
    else
    {
        Serial.println("SD inicializado");
    }

    // Wi-Fi setup and connection
    WiFi.begin(ssid, password);
    while ((WiFi.status() != WL_CONNECTED) && (wifi_connect_attempts <= 100))
    {
        delay(500);
        Serial.print(".");
        wifi_connect_attempts++;
    }
    Serial.println();
    delay(500);

    if (WiFi.status() == WL_CONNECTED)
    {
        wifi_connection_status = true;
        // MQTT
        // mqtt_connect_attempts on the while is for avoid a infinity loop
        // mqtt_client_state is for verifying if need a reset on the end of the "void loop()"
        client.setServer(mqttServer, mqttPort);
        while ((!client.connected()) && (mqtt_connect_attempts <= 5))
        {
            mqtt_connect_attempts++;
            Serial.println("Connecting to the MQTT broker...");
            if (client.connect("ESP8266Client", mqttUser, mqttPassword))
            {
                mqtt_client_state = client.state();
                client.subscribe(mqttTopicSub);
                Serial.println("Connected. State: ");
                Serial.print(mqtt_client_state);
            }
            else
            {
                mqtt_client_state = client.state();
                Serial.print("Error. State: ");
                Serial.print(mqtt_client_state);
            }
        }
        

        Serial.print("RTC update status (1 for updated and 0 for not): ");
        // The parameter is the interval between updates
        Serial.println(rtc_time_update(7));
        Serial.println("Interval between updates: 7 days");
        delay(300);
    }
    else
    {
        wifi_connection_status = false;
        Serial.println("Wi-Fi not connected, RTC not updated and MQTT not connected");
    }

    // #DEBUG
    Serial.print("Setup end time - Loop Begin time: ");
    Serial.println(millis());
}

void loop()
{
    // String 'reset'
    csv_output = "";
    attiny_serial_data = "";
    esp8266_time = "";
    file_name = "";
    serial_sensor_data = "";

    // Request data from ATtiny85 and return a csv string
    delay(200);
    attiny_serial_data = get_attiny_data(); // "wind_direction,anemometer(km/h),pluviometer"

    // Gets serial sensors data csv (sensor1,sensor2)
    delay(600);
    serial_sensor_data = serial_data_request("id1");

    esp8266_time.concat(get_time_RTC()); // Get RTC time
    // Read BMP280 sensors
    bmp_temperature = bmp.readTemperature();
    bmp_pressure = bmp.readPressure();
    // Read SHT20
    sht_temperature = sht20.readTemperature();
    sht_humidity = sht20.readHumidity();

    // #DEBUG
    Serial.println("RTC time: " + esp8266_time);
    Serial.println("bmp_temperature = " + String(bmp_temperature));
    Serial.println("bmp_pressure = " + String(bmp_pressure));
    Serial.println("sht_temperature = " + String(sht_temperature));
    Serial.println("sht_humidity = " + String(sht_humidity));

    // Concatenates the data on csv_output
    csv_output.concat(esp8266_time); // Date and time of reading
    csv_output.concat(",");          // CSV comma
    csv_output.concat(String(bmp_temperature));
    csv_output.concat(",");
    csv_output.concat(String(bmp_pressure));
    csv_output.concat(",");
    csv_output.concat(String(sht_temperature));
    csv_output.concat(",");
    csv_output.concat(String(sht_humidity));
    csv_output.concat(",");
    csv_output.concat(attiny_serial_data);
    csv_output.concat(",");
    csv_output.concat(serial_sensor_data);

    // Setup the file name
    // esp8266_time = '20200911220150', substring returns only '20200911'
    file_name.concat(String(esp8266_time.substring(0, 8)));
    file_name.concat(".csv");

    //Write the CSV data on SD card
    File dataFile = SD.open(file_name, FILE_WRITE);
    if (dataFile)
    {
        dataFile.println(csv_output);
        dataFile.close();
        sd_status = 1;
    }
    else
    {
        sd_status = 3;
    }

    csv_output.concat(",");
    csv_output.concat(String(sd_status));

    if (wifi_connection_status == true)
    {
        // Check the MQTT
        if (!client.connected())
        {
            reconnect_MQTT();
            client.subscribe(mqttTopicSub);
        }
        client.loop();
        client.publish(mqttTopicSub, String(csv_output).c_str(), true);

        mqtt_client_state = client.state();
        Serial.print("MQTT client state: ");
        Serial.println(mqtt_client_state);

        if (mqtt_client_state == 0)
        {
            Serial.println("MQTT_CONNECTED - the client is connected");
        }

        if ((mqtt_connect_attempts >= 4))
        {
            if (mqtt_client_state == -2)
            {
                Serial.println("MQTT_CONNECT_FAILED - the network connection failed.");
                delay(1000);
                client.disconnect();
                delay(500);
                WiFi.disconnect();
                Serial.println("Client and Wi-Fi disconnected and begin ESP reset...");
                delay(1000);
                ESP.reset();
            }
            else if (mqtt_client_state == -4)
            {
                Serial.println("MQTT_CONNECTION_TIMEOUT - the server didn't respond within the keepalive time.");
            }
            else if (mqtt_client_state == -3)
            {
                Serial.println("MQTT_CONNECTION_LOST - the network connection was broken.");
            }
            else if (mqtt_client_state == -1)
            {
                Serial.println("MQTT_DISCONNECTED - the client is disconnected cleanly.");
            }
            else if (mqtt_client_state == 1)
            {
                Serial.println("MQTT_CONNECT_BAD_PROTOCOL - the server doesn't support the requested version of MQTT.");
            }
            else if (mqtt_client_state == 2)
            {
                Serial.println("MQTT_CONNECT_BAD_CLIENT_ID - the server rejected the client identifier.");
            }
            else if (mqtt_client_state == 3)
            {
                Serial.println("MQTT_CONNECT_UNAVAILABLE - the server was unable to accept the connection.");
            }
            else if (mqtt_client_state == 4)
            {
                Serial.println("MQTT_CONNECT_BAD_CREDENTIALS - the username/password were rejected.");
            }
            else if (mqtt_client_state == 5)
            {
                Serial.println("MQTT_CONNECT_UNAUTHORIZED - the client was not authorized to connect.");
            }
        }
        else
        {
            delay(1000);
            client.disconnect();
            WiFi.disconnect();
            delay(1000);
        }
        // #DEBUG
        Serial.println(csv_output);
        Serial.print("Loop end time: ");
        Serial.println(millis());
    }
    else
    {
        delay(1000);
        WiFi.disconnect();

        // #DEBUG
        Serial.println(csv_output);
        Serial.print("Loop end time: ");
        Serial.println(millis());
        
        delay(1000);
    }

    // Calculates the interval of sleepmode
    sleep_time = millis() - begin_time;
    if (sleep_time < 0)
    {
        sleep_time = 59000;
    }
    else if (sleep_time > 40000)
    {
        sleep_time = 20000;
    }
    else
    {
        // sleep_time = 59000 - sleep_time;
        sleep_time = abs(59000 - sleep_time);
    }
    Serial.print("Sleep time: ");
    Serial.println(sleep_time);

    // Turn on the sleep mode only after setup the time of sleep and delay for complete work
    ESP.deepSleep(sleep_time * 1000);
    //ESP.deepSleep(55e6);
    //ESP.deepSleep(47e6);
}