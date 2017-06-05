/**
 * The MySensors Arduino library handles the wireless radio link and protocol
 * between your home built sensors/actuators and HA controller of choice.
 * The sensors forms a self healing radio network with optional repeaters. Each
 * repeater and gateway builds a routing tables in EEPROM which keeps track of the
 * network topology allowing messages to be routed to nodes.
 *
 * Created by Henrik Ekblad <henrik.ekblad@mysensors.org>
 * Copyright (C) 2013-2015 Sensnology AB
 * Full contributor list: https://github.com/mysensors/Arduino/graphs/contributors
 *
 * Documentation: http://www.mysensors.org
 * Support Forum: http://forum.mysensors.org
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 *******************************
 *
 * REVISION HISTORY
 * Version 1.0 - Henrik Ekblad
 * Version 2.1 - Ken
 * 
 * DESCRIPTION
 * Motion Sensor example using HC-SR501 
 * http://www.mysensors.org/build/motion
 *
 */

#define MY_NODE_ID 10

#define MY_REPEATER_FEATURE

#include <SPI.h>
#include <MySensors.h>

unsigned long ReportInterval = 120000; // Time between reports (in milliseconds)
unsigned long _lastRegularReportMillis = 0;
bool _lastMotionSensorValue = false;
#define MOTION_SENSOR_PIN 3     // The digital input you attached your motion sensor.  (Only 2 and 3 generates interrupt!)
#define MOTION_SENSOR_ID 1                 // Id of the sensor child

// Initialize motion message
MyMessage _motionSensorMsg(MOTION_SENSOR_ID, V_TRIPPED);

void setup()
{
    pinMode(MOTION_SENSOR_PIN, INPUT); // sets the motion sensor digital pin as input
}

void presentation()
{
    // Send the sketch version information to the gateway and Controller
    sendSketchInfo("Stairway Motion Sensor", "2.1");

    // Register all sensors to gw (they will be created as child devices)
    present(MOTION_SENSOR_ID, S_MOTION);
}

void loop()
{
    // Read digital motion value
    bool tripped = digitalRead(MOTION_SENSOR_PIN) == HIGH;

    if (tripped != _lastMotionSensorValue) {
        _lastMotionSensorValue = tripped;
        Serial.println(tripped);
        _reportMotionSensor(tripped);
    }

    unsigned long now = millis();
    if (now - _lastRegularReportMillis >= ReportInterval) {
        _lastRegularReportMillis = now;
        _reportMotionSensor(tripped);
    }
}

void _reportMotionSensor(bool tripped) {
    send(_motionSensorMsg.set(tripped ? "1" : "0")); // Send tripped value to gw
}