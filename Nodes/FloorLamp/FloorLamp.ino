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
 * 
 * DESCRIPTION
 * Example sketch showing how to control physical relays. 
 * This example will remember relay state after power failure.
 * http://www.mysensors.org/build/relay
 */ 

// Enable repeater functionality for this node
#define MY_REPEATER_FEATURE

#include <SPI.h>
#include <MySensors.h>

#define RELAY_1  3  // Arduino Digital I/O pin number for first relay (second on pin+1 etc)
#define SENSOR_ID_1 1
#define NUMBER_OF_RELAYS 1 // Total number of attached relays
#define RELAY_ON 0  // GPIO value to write to turn on attached relay
#define RELAY_OFF 1 // GPIO value to write to turn off attached relay

#define HEARTBEAT_INTERVAL 10000
unsigned long nextHeartbeatMillis = 0;

MyMessage msgState1(SENSOR_ID_1, V_LIGHT);

void before() { 
  for (int sensor = SENSOR_ID_1, pin = RELAY_1; sensor<=NUMBER_OF_RELAYS;sensor++, pin++) {
    // Then set relay pins in output mode
    pinMode(pin, OUTPUT);   
    // Set relay to last known state (using eeprom storage) 
    digitalWrite(pin, loadState(sensor)?RELAY_ON:RELAY_OFF);
  }
}

void setup() {
  nextHeartbeatMillis = millis() + HEARTBEAT_INTERVAL;
}

void presentation()  
{   
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("Floor lamp", "2.1");

  for (int sensor = SENSOR_ID_1, pin=RELAY_1; sensor<=NUMBER_OF_RELAYS;sensor++, pin++) {
    // Register all sensors to gw (they will be created as child devices)
    present(sensor, S_LIGHT);
  }
}


void loop() 
{
  unsigned long now = millis();
  if (now >= nextHeartbeatMillis) {
    nextHeartbeatMillis = now + HEARTBEAT_INTERVAL;
    #ifdef MY_DEBUG
    Serial.print("Sending status as heartbeat.");
    #endif
    for (int sensor = SENSOR_ID_1, pin=RELAY_1; sensor<=NUMBER_OF_RELAYS;sensor++, pin++) {
      send(msgState1.set(loadState(sensor)));
    }
  }
}

void receive(const MyMessage &message) {
  // We only expect one type of message from controller. But we better check anyway.
  if (message.type==V_LIGHT) {
     // Change relay state
     digitalWrite(message.sensor-1+RELAY_1, message.getBool()?RELAY_ON:RELAY_OFF);
     // Store state in eeprom
     saveState(message.sensor, message.getBool());
     // Write some debug info
     Serial.print("Incoming change for sensor:");
     Serial.print(message.sensor);
     Serial.print(", New status: ");
     Serial.println(message.getBool());
   } 
}
