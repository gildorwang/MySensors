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
 * Version 2.0 - Ken - Signature
 * Version 2.1 - Ken - Heartbeat
 * Version 2.2 - Ken - Button & LED
 * 
 * DESCRIPTION
 * Example sketch showing how to control physical relays. 
 * This example will remember relay state after power failure.
 * http://www.mysensors.org/build/relay
 */ 

#define MY_NODE_ID 32

// Enable repeater functionality for this node
#define MY_REPEATER_FEATURE

#include <MySensorsCustomConfig.h>
#include <SPI.h>
#include <MySensors.h>
#include <Bounce2.h>

#define RELAY_PIN_1  3  // Arduino Digital I/O pin number for first relay (second on pin+1 etc)
#define SENSOR_ID_1 1
#define NUMBER_OF_RELAYS 1 // Total number of attached relays
#define RELAY_ON LOW  // GPIO value to write to turn on attached relay
#define RELAY_OFF HIGH // GPIO value to write to turn off attached relay
#define BUTTON_ON LOW
#define BUTTON_OFF HIGH
#define BUTTON_PIN 4
#define LED_PIN 5
#define LED_PIN_ON HIGH
#define LED_PIN_OFF LOW

#define HEARTBEAT_INTERVAL 30000
unsigned long nextHeartbeatMillis = 0;

MyMessage msgState1(SENSOR_ID_1, V_LIGHT);

int buttonOldVal = -1;
Bounce button = Bounce();

void before() { 
    Serial.println("Initializing...");
    pinMode(RELAY_PIN_1, OUTPUT);
    pinMode(LED_PIN, OUTPUT);
    // Set relay to last known state (using eeprom storage) 
    setRelay(loadState(SENSOR_ID_1));
    
    button.attach(BUTTON_PIN, INPUT_PULLUP);
    button.interval(5);

    nextHeartbeatMillis = millis() + HEARTBEAT_INTERVAL;
}

void presentation()  
{   
    // Send the sketch version information to the gateway and Controller
    sendSketchInfo("Water heater", "3.0");
    // Register sensor to gw (they will be created as child devices)
    present(SENSOR_ID_1, S_LIGHT);
}

void loop() 
{
    if (isButtonPushed()) {
        setState(!loadState(SENSOR_ID_1));
    }
    unsigned long now = millis();
    if (now >= nextHeartbeatMillis) {
        nextHeartbeatMillis = now + HEARTBEAT_INTERVAL;
        #ifdef MY_DEBUG
        Serial.print("Sending status as heartbeat.");
        #endif
        sendState();
    }
}

void receive(const MyMessage &message) {
    if (message.type == V_LIGHT && message.sensor == SENSOR_ID_1) {
        setState(message.getBool());
    }
}

void setState(bool state) {
    Serial.print("Setting sensor state to: ");
    Serial.println(state);
    // Change relay state
    setRelay(state);
    // Store state in eeprom
    saveState(SENSOR_ID_1, state);
    sendState();
}

void setRelay(bool state) {
    digitalWrite(RELAY_PIN_1, state ? RELAY_ON : RELAY_OFF);
    digitalWrite(LED_PIN, state ? LED_PIN_ON : LED_PIN_OFF);
}

void sendState() {
    bool state = loadState(SENSOR_ID_1);
    Serial.print("Sending state: ");
    Serial.println(state);
    send(msgState1.set(state));
}

int isButtonPushed() {
    button.update();
    return button.fell();
}
