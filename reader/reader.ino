#include "HardwareSerial.h"
uint8_t command_pins[6] = {3, 7, 4, 5, 2, 6};
uint8_t analog_pins[6] = {A2, A5, A6, A3, A4, A7};
float target_values[6] = {0, 0, 0, 0, 0, 0};
int command_values[6] = {0, 0, 0, 0, 0, 0};

void setup() {
    Serial.begin(9600);
    for (int i = 0; i < 6; i++) {
        pinMode(command_pins[i], OUTPUT);
        pinMode(analog_pins[i], INPUT);
    }
}

void loop() {
    if (Serial.available() > 1) {
        int command = Serial.parseInt();
        if (command >= 0 && command < 6) {
            target_values[command] = Serial.parseFloat();
        }
    }
    char buffer[255], line_buf[31], pressure_buf[10], target_buf[10], control_buf[10];
    buffer[0] = '\0';
    for (int i = 0; i < 6; i++) {
        float pressure = (analogRead(analog_pins[i]) * 5.0 / 1023 - 1) * 0.9 * 1000 / 4;
        dtostrf(pressure, 6, 2, pressure_buf);
        dtostrf(target_values[i], 6, 2, target_buf);
        dtostrf(command_values[i] * 100.0 / 255, 6, 2, control_buf);
        sprintf(line_buf, "|%s,%s", pressure_buf, control_buf);
        strcat(buffer, line_buf);
        if (analogRead(analog_pins[i]) == 0) {
            command_values[i] = 0;
            analogWrite(command_pins[i], command_values[i]);
            continue;
        }
        if (abs(pressure - target_values[i]) < 5) {
            continue;
        }
        if (pressure < target_values[i]) {
            command_values[i] += 1;
            if (command_values[i] > 255) {
                command_values[i] = 255;
            }
        } else {
            command_values[i] -= 1;
            if (command_values[i] < 0) {
                command_values[i] = 0;
            }
        }
        analogWrite(command_pins[i], command_values[i]);
    }
    Serial.println(buffer);
    // delay(100);
}