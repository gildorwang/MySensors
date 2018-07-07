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
#include <DHT.h>

unsigned long ReportInterval = 120000; // Time between reports (in milliseconds)
unsigned long _lastRegularReportMillis = 0;
bool _lastMotionSensorValue = false;
#define MOTION_SENSOR_PIN 3     // The digital input you attached your motion sensor.  (Only 2 and 3 generates interrupt!)
#define MOTION_SENSOR_ID 1                 // Id of the sensor child

#define DHT_DATA_PIN 4
#define HUM_SENSOR_ID 2
#define TEMP_SENSOR_ID 3
// Set this offset if the sensor has a permanent small offset to the real temperatures
#define SENSOR_TEMP_OFFSET 0
MyMessage _humidityMsg(HUM_SENSOR_ID, V_HUM);
MyMessage _temperatureMsg(TEMP_SENSOR_ID, V_TEMP);
DHT _dht;
bool _isMetric = false;

#define RELAY_PIN 5
#define SIREN_SENSOR_ID 4
#define RELAY_ON LOW
#define RELAY_OFF HIGH

// Initialize motion message
MyMessage _motionSensorMsg(MOTION_SENSOR_ID, V_TRIPPED);

void setup()
{
    // sets the motion sensor digital pin as input
    pinMode(MOTION_SENSOR_PIN, INPUT);
    // sets the relay digital pin as output
    pinMode(RELAY_PIN, OUTPUT);
    // Turns off the siren on startup
    digitalWrite(RELAY_PIN, RELAY_OFF);
    _dht.setup(DHT_DATA_PIN);
}

void presentation()
{
    // Send the sketch version information to the gateway and Controller
    sendSketchInfo("Stairway Motion Sensor", "2.2");

    // Register all sensors to gw (they will be created as child devices)
    present(MOTION_SENSOR_ID, S_MOTION);
    present(HUM_SENSOR_ID, S_HUM);
    present(TEMP_SENSOR_ID, S_TEMP);
    present(SIREN_SENSOR_ID, S_LIGHT);
    
    _isMetric = getControllerConfig().isMetric;
}

void loop()
{
    // Read digital motion value
    bool tripped = digitalRead(MOTION_SENSOR_PIN) == HIGH;

    if (tripped != _lastMotionSensorValue) {
        _lastMotionSensorValue = tripped;
        Serial.print("Motion sensor: ");
        Serial.println(tripped);
        _reportMotionSensor(tripped);
    }

    unsigned long now = millis();
    if (now - _lastRegularReportMillis >= ReportInterval) {
        _lastRegularReportMillis = now;
        _reportMotionSensor(tripped);
        _reportTemperatureAndHumidity();
    }
}

void receive(const MyMessage &message) {
    if (message.type == V_LIGHT) {
        digitalWrite(RELAY_PIN, message.getBool() ? RELAY_ON : RELAY_OFF);
    }
}

void _reportMotionSensor(bool tripped) {
    send(_motionSensorMsg.set(tripped ? "1" : "0")); // Send tripped value to gw
}

void _reportTemperatureAndHumidity() {
    // Force reading sensor, so it works also after sleep()
    _dht.readSensor(true);
    
    // Get temperature from DHT library
    float temperature = _dht.getTemperature();
    if (isnan(temperature)) {
      Serial.println("Failed reading temperature from DHT!");
    } else {
      if (!_isMetric) {
        temperature = _dht.toFahrenheit(temperature);
      }
      temperature += SENSOR_TEMP_OFFSET;
      send(_temperatureMsg.set(temperature, 1));
      Serial.print("T: ");
      Serial.println(temperature);
    }

    // Get humidity from DHT library
    float humidity = _dht.getHumidity();
    if (isnan(humidity)) {
      Serial.println("Failed reading humidity from DHT");
    } else {
      send(_humidityMsg.set(humidity, 1));
      
      Serial.print("H: ");
      Serial.println(humidity);
    }
}