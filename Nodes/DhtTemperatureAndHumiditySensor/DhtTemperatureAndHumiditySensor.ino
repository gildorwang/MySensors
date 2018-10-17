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
 * Version 1.0: Henrik EKblad
 * Version 1.1 - 2016-07-20: Converted to MySensors v2.0 and added various improvements - Torben Woltjen (mozzbozz)
 * 
 * DESCRIPTION
 * This sketch provides an example of how to implement a humidity/temperature
 * sensor using a DHT11/DHT-22.
 *  
 * For more information, please visit:
 * http://www.mysensors.org/build/humidity
 * 
 */

#define MY_NODE_ID 11

// Enabled repeater feature for this node
#define MY_REPEATER_FEATURE

#include <MySensorsCommon.h>
#include <DhtSensor.h>

// Set this to the pin you connected the DHT's data pin to
#define DHT_DATA_PIN 3

#define CHILD_ID_HUM 0
#define CHILD_ID_TEMP 1
#define LED_PIN 4

unsigned long _nextUpdateMillis = 0;
MessageSender _messageSender;
DhtSensor _dhtSensor(DHT_DATA_PIN, CHILD_ID_TEMP, CHILD_ID_HUM, _messageSender);

const long UpdateInterval = 30000; // Wait time between reads (in milliseconds)
void presentation()
{
    // Send the sketch version information to the gateway
    sendSketchInfo("TemperatureAndHumidity_1", "3.1");
    _dhtSensor.present();
}

void setup() {
    _dhtSensor.setup();
}

void loop() {  
    unsigned long currentMillis = millis();
    if (currentMillis >= _nextUpdateMillis) {
        _nextUpdateMillis = currentMillis + UpdateInterval;
        
        if (_dhtSensor.report()) {
            digitalWrite(LED_PIN, HIGH);
        } else {
            digitalWrite(LED_PIN, LOW);
        }
    }
}
