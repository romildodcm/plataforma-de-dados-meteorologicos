#include <SoftwareSerial.h>

const int analog_read_interval = 10000;
bool serial_status = false;
int sample1, sample2;
unsigned long last_analog_read = 0;

SoftwareSerial serialCom(2, 0); // RX,TX

bool serial_send(int measurement1, int measurement2, String serial_id = "")
{
    String command;
    String sensor_output;
    bool status_bool = false;

    if (serialCom.available())
    {
        command = serialCom.readStringUntil('\n');

        if (command.equals(serial_id))
        {
            sensor_output.concat(serial_id);
            sensor_output.concat(",");
            sensor_output.concat(String(measurement1));
            sensor_output.concat(",");
            sensor_output.concat(String(measurement2));
            sensor_output.concat("*");

            // Sends data
            serialCom.println(sensor_output);
            status_bool = true;
        }
    }

    return status_bool;
}

void setup()
{
    serialCom.begin(9600);
}

void loop()
{
    // Executa o que está aqui dentro do if a cada 10 segundos, o resto do tempo o attiny fica verificando chamada na serial.
    if (abs(millis() - last_analog_read) > analog_read_interval)
    {
        last_analog_read = millis();
        // random int number
        sample1 = random(1023);
        sample2 = random(1023);
    }

    serial_status = serial_send(sample1, sample2, "id1");
}