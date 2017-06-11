/**
 * This is purely for fan control of the power supply unit.
 * It is not connected to the MySensors network.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 */

#include <PWM.h>
#define FAN_PIN 3
int32_t frequency = 25000;

void setup() {
    Serial.begin(115200);
    pinMode(LED_BUILTIN, OUTPUT);
    
    /********* Setup PWM for Fan control **********/
    //initialize all timers except for 0, to save time keeping functions
    InitTimersSafe();

    //sets the frequency for the specified pin
    bool success = SetPinFrequencySafe(FAN_PIN, frequency);
    
    //if the pin frequency was set successfully, turn pin 13 on
    if(!success) {
        Serial.println("Error: Failed to set PWM");
        digitalWrite(LED_BUILTIN, HIGH);
    }
    else {
        Serial.println("Successfully initialized.");
        digitalWrite(LED_BUILTIN, LOW);
    }
}

void loop() {
    pwmWrite(FAN_PIN, 220);
}