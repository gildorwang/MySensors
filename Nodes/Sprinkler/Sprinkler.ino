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

#define MY_NODE_ID 5

// Enable repeater functionality for this node
#define MY_REPEATER_FEATURE

#include <SPI.h>
#include <MySensors.h>

#define RELAY_1  3  // Arduino Digital I/O pin number for first relay (second on pin+1 etc)
#define NUMBER_OF_RELAYS 4 // Total number of attached relays
#define RELAY_ON 1  // GPIO value to write to turn on attached relay
#define RELAY_OFF 0 // GPIO value to write to turn off attached relay

#define SENSOR_ID_LCD 0

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,16,2); // set the LCD address to 0x27 for a 16 chars and 2 line display

#define NODE_VERSION "2.2"

const int lcdOnDurationMillis = 3000;
unsigned long lcdOffMillis = 0;

void before() {
    lcd.init(); // initialize the lcd
    msg("Initializing...", "Node ver: " NODE_VERSION); // Print a message to the LCD.
    
    for (int sensor=1, pin=RELAY_1; sensor<=NUMBER_OF_RELAYS;sensor++, pin++) {
        // Then set relay pins in output mode
        pinMode(pin, OUTPUT);
        // Set relay to last known state (using eeprom storage)
        digitalWrite(pin, loadState(sensor)?RELAY_ON:RELAY_OFF);
    }

    msg("Ready", "Node ver: " NODE_VERSION);
}

void presentation()
{
    // Send the sketch version information to the gateway and Controller
    sendSketchInfo("Sprinkler", NODE_VERSION);

    for (int sensor=1, pin=RELAY_1; sensor<=NUMBER_OF_RELAYS;sensor++, pin++) {
        // Register all sensors to gw (they will be created as child devices)
        present(sensor, S_BINARY);
    }

    present(SENSOR_ID_LCD, S_INFO);
}


void loop()
{
    if (lcdOffMillis != 0 && millis() > lcdOffMillis) {
        lcd.noBacklight();
        Serial.println("Turn off backlight");
        lcdOffMillis = 0;
    }
}

void receive(const MyMessage &message) {
    // We only expect one type of message from controller. But we better check anyway.
    if (message.type==V_STATUS) {
        // Change relay state
        digitalWrite(message.sensor - 1 + RELAY_1, message.getBool()?RELAY_ON:RELAY_OFF);
        // Store state in eeprom
        saveState(message.sensor, message.getBool());
        // Write some debug info
        Serial.print("Incoming change for sensor:");
        Serial.print(message.sensor);
        Serial.print(", New status: ");
        Serial.println(message.getBool());

        char str[100];
        snprintf(str, sizeof(str), "%d", message.sensor);
        strcat(str, ": ");
        strcat(str, message.getBool() ? "ON" : "OFF");
        msg("New status", str);
    }
    else if (message.type == V_TEXT) {
        const char* text = message.getString();
        Serial.print("Incoming text: ");
        Serial.println(text);
        msg("Server:", text);
    }
}

void msg(const char line1[], const char line2[]) {
    lcd.clear();
    lcd.backlight();
    lcd.print(line1);
    lcd.setCursor(0,1); //newline
    lcd.print(line2);
    lcdOffMillis = millis() + lcdOnDurationMillis;
}

