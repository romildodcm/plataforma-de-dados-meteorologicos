/*****************************************************************
  * STATUS: all ok.
  * 
  * FIRMWARE DESCRIPTION
  * 
  * Version: 2.6 (20210202)
  * Developer: Romildo C Marques
  * E-mail: romildodcm@gmail.com
  * 
  * OUTPUT DATA DESCRIPTION
  * 
  * The output data it's a CSV string:
  * "windDirection,wind_speed_kmh,mmOfRain*"
  * Example: "N,9.72,1.75*" = north wind, wind speed 9.72 km/h and 1.75 mm of rain. * is the end character.
  * 
  * // Wind Direction Output
  * //     EN    |    PT    | Output | Degrees
  * // North     | Norte    | N      | 0/360
  * // Northeast | Nordeste | NE     | 45
  * // East      | Leste    | E      | 90
  * // Southeast | Sudeste  | SE     | 135
  * // South     | Sul      | S      | 180
  * // Southwest | Sudoeste | SW     | 225
  * // West      | Oeste    | W      | 270
  * // Northwest | Noroeste | NW     | 315
  * 
  * ATENTION FOR THE HARDWARE LIMITATIONS
  * 
  * Pluviometer will be reset after 8000 mm of rain. The variable
  * pluviometer_reed_switch_count will be reset at 6400 commutations.
  * Becouse is  a unsigned int type and store a 2 byte value, is limited
  * to 65535. When the value is 6400 (8000 mm of rain), the variable
  * will be reset to 0.
  * 
  * I/O Ports Configuration
  * 
  * Reset ->                (Pin 1)
  * Wind Vane -> A3         (Pin 2)
  * Wemos signal in -> A1   (Pin 3)
  * GND ->                  (Pin 4)
  * Serial TX -> D0         (Pin 5)
  * Rain gauge -> PCINT1    (Pin 6)
  * Anemometer -> EI0       (Pin 7)
  * VCC ->                  (Pin 8)
  * 
*****************************************************************/

// SendOnlySoftwareSerial library and seting
#include <SendOnlySoftwareSerial.h>

// Library for pin change interrupt
#include <avr/interrupt.h>

// SendOnlySoftwareSerial seting
SendOnlySoftwareSerial mySerial(0); // Tx pin

// Constants
const float pi = 3.14159265; // Number PI
const int radius = 147;      // Anemometer radius in mm
const int period = 5000;     // anemometer periode of measurement

// Variables
int wemos_com_in = 0, // Wemos's communication input voltage
    last_wemos_com_in = 0; // Wemos's oldest communication input voltage
volatile unsigned int pluviometer_reed_switch_count = 0, // Commutation counter of rain gauge
    anemometer_switch_count = 0; // Commutation counter of anemometer
float anemometer_measurement = 0, pluviometer_measurement = 0;
unsigned long anemometer_last_time = 0;
String out; // String for serial data output

// Interrupt service routines
void anemometer_interrupt_count()
{
    anemometer_switch_count++;
}

ISR(PCINT0_vect)
{
    pluviometer_reed_switch_count++; // Increment volatile variable
}

/******************************************************************
 * This function returns the wind speed in Km/h, in float type
 * ***************************************************************/
float anemometer()
{
    float wind_speed_ms = 0, wind_speed_kmh = 0;
    int interrupts_counter = 0;
    unsigned int RPM = 0; // Revolutions per minute
    unsigned long interrupt_time = 0, last_interrupt_time = 0, start_time, time_while = 0;

    attachInterrupt(0, anemometer_interrupt_count, RISING);

    anemometer_switch_count = 0;
    interrupts_counter = 0;

    // The time when begin the period of read and count interrupts
    start_time = millis();
    time_while = start_time + 1;

    // Counts interruptions
    while (abs(time_while - start_time) <= period)
    {
        if (anemometer_switch_count > 0)
        {
            interrupt_time = millis();
            if (abs(last_interrupt_time - interrupt_time) > 9)
            {
                last_interrupt_time = interrupt_time;
                anemometer_switch_count--;
                interrupts_counter++;
            }
            else
            {
                anemometer_switch_count--;
            }
        }

        // delay to prevent problem with event like "time_while == last_interrupt_time" caused becouse of the high clock of ESP8266 with millis() limitation (only milliseconds is a low precision for this while loop)
        delay(1);

        // Read the time to the while conditions
        time_while = millis();
    }

    detachInterrupt(0);

    // Compute the RPM
    RPM = ((interrupts_counter)*60) / (period / 1000);

    // Compute wind speed in m/s
    wind_speed_ms = ((4 * pi * radius * RPM) / 60) / 1000;

    // Compute wind speed in km/h
    wind_speed_kmh = wind_speed_ms * 3.6;

    return wind_speed_kmh;
}

/******************************************************************
 * This function returns the wind direction of a wind vane sensor
 * 
  * // Wind Direction Output
  * //     EN    |    PT    | Output | Degrees
  * // North     | Norte    | N      | 0/360
  * // Northeast | Nordeste | NE     | 45
  * // East      | Leste    | E      | 90
  * // Southeast | Sudeste  | SE     | 135
  * // South     | Sul      | S      | 180
  * // Southwest | Sudoeste | SW     | 225
  * // West      | Oeste    | W      | 270
  * // Northwest | Noroeste | NW     | 315
 * ***************************************************************/
String wind_vane()
{
    const int n_samples = 35;             // n_samples reads for average
    unsigned int wind_vane_signal_sum = 0;         // Wind vane input signal sum
    float wind_vane_signal_average = 0;   // Wind vane input signal average
    String wind_vane_output;

    // 490ms of delay for n_samples samples
    for (int i = 0; i < n_samples; i++)
    {
        wind_vane_signal_sum = wind_vane_signal_sum + analogRead(A3);
        delay(14);
    }

    // Average
    wind_vane_signal_average = wind_vane_signal_sum / n_samples;

    wind_vane_signal_sum = 0;

    // Wind direction
    if (wind_vane_signal_average >= 971)
    {
        wind_vane_output.concat("NW");
    }
    else if (wind_vane_signal_average >= 926)
    {
        wind_vane_output.concat("W");
    }
    else if (wind_vane_signal_average >= 874)
    {
        wind_vane_output.concat("SW");
    }
    else if (wind_vane_signal_average >= 810)
    {
        wind_vane_output.concat("S");
    }
    else if (wind_vane_signal_average >= 732)
    {
        wind_vane_output.concat("SE");
    }
    else if (wind_vane_signal_average >= 636)
    {
        wind_vane_output.concat("E");
    }
    else if (wind_vane_signal_average >= 514)
    {
        wind_vane_output.concat("NE");
    }
    else if (wind_vane_signal_average >= 300)
    {
        wind_vane_output.concat("N");
    }
    else
    {
        wind_vane_output.concat("null");
    }

    return wind_vane_output;
}

void setup()
{
    // Rain gauge pin change interrupt setup
    GIMSK = 0b00100000; // turns on pin change interrupts
    PCMSK = 0b00000010; // turn on interrupts on pin PCINT1 = Pin 6

    mySerial.begin(9600);
    pinMode(A3, INPUT); // Wind vane input
    pinMode(A2, INPUT); // Wemos signal in
    pinMode(2, INPUT_PULLUP); // Anemometer attach interrupt input pin 0 = digital pin 2 = HW pin 7

    // Get the wind speed
    anemometer_measurement = anemometer();
    anemometer_last_time = millis();
}

void loop()
{
    
    // Get the wind speed every 10 seconds
    if (abs(millis() - anemometer_last_time) > 10000)
    {
        // Get the wind speed
        anemometer_measurement = anemometer();
        anemometer_last_time = millis();
    }

    // reset the pluviometer_reed_switch_count at 64000 (equals 8000 mm of rain)
    if (pluviometer_reed_switch_count >= 64000)
    {
        pluviometer_reed_switch_count = pluviometer_reed_switch_count - 64000;
    }

    // Read the analog pin 2 to detect if ESP8266 microcontroller requested data
    wemos_com_in = analogRead(A2);

    // When the call is LOW to HIGH
    // Check the value, if > or equals 2.2v and the oldest voltage value is < or equals 1.1V
    if ((wemos_com_in >= 450) && (last_wemos_com_in <= 220))
    {
        out = ""; // Clean the string

        // Concatenates the data on csv_output
        out.concat(String(wind_vane())); // Concatenates wind direction
        out.concat(","); // CSV comma
        out.concat(String(anemometer_measurement)); // Concatenates wind speed in km/h
        out.concat(",");
        pluviometer_measurement = (pluviometer_reed_switch_count * 0.5) * 0.25; // Rain measure
        out.concat(String(pluviometer_measurement)); // Concatenates rains measurement
        out.concat('*'); // Concatenates the end word of the string
        mySerial.print(out); // Sends the string via serial
        delay(100); // debouncing delay
    }
    // Stores the value of analog pin 2
    last_wemos_com_in = wemos_com_in;
}

/*
REFERENCES

Arduino Reference - attachInterrupt()
https://www.arduino.cc/reference/pt/language/functions/external-interrupts/attachinterrupt/

PLUVIÔMETRO ARDUINO COMO SENSOR DE CHUVA NA ESTAÇÃO METEOROLÓGICA
https://www.usinainfo.com.br/blog/pluviometro-arduino-como-sensor-de-chuva-na-estacao-meteorologica/

INDICADOR DE DIREÇÃO DO VENTO COM ARDUINO MELHORANDO SUA ESTAÇÃO METEOROLÓGICA
https://www.usinainfo.com.br/blog/indicador-de-direcao-do-vento-com-arduino-melhorando-sua-estacao-meteorologica/
*/