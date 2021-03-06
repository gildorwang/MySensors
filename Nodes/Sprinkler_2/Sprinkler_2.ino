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
 */

#define MY_NODE_ID 51
//#define MY_DEBUG 
#define MY_DEBUG_VERBOSE_RFM69

// Enable repeater functionality for this node
#define MY_REPEATER_FEATURE

#include <MySensorsCustomConfig.h>
#include <SPI.h>
#include <MySensors.h>
#include <Bounce2.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

#define RELAY_1_PIN  3  // Arduino Digital I/O pin number for first relay (second on pin+1 etc)
#define RELAY_1_SENSOR_ID 1 // Sensor ID for the first relay
#define NUMBER_OF_RELAYS 3 // Total number of attached relays
#define RELAY_ON LOW  // GPIO value to write to turn on attached relay
#define RELAY_OFF HIGH // GPIO value to write to turn off attached relay
#define GREEN_BUTTON_PIN 8
#define RED_BUTTON_PIN 9
#define MAX_SEND_ATTEMPTS 5

#define SENSOR_ID_LCD 0

#define HEARTBEAT_INTERVAL 30000

// LCD wiring:
// - VCC: 5V
// - GND: GND
// - SDA: A4
// - SCL: A5
LiquidCrystal_I2C lcd(0x3F, 16, 2); // set the LCD address to 0x3F for a 16 chars and 2 line display

#define NODE_VERSION "3.5"

// LCD backlight on duration
const unsigned long LcdOnDurationMillis = 5000;
// Scheduled LCD backlight off time
unsigned long _lcdOffMillis = 0;
// Scheduled heartbeat time
unsigned long _heartbeatMillis = 0;
// The maximum watering time limit, in case the controller is down or lost connectivity
const unsigned long MaxWaterDuration = 30L * 60L * 1000L;
// Scheduled off time for each station
unsigned long _waterOffMillis[NUMBER_OF_RELAYS] = {0};
// Time to wait before starting manual watering when selecting stations
const unsigned long StartWateringDelay = 5 * 1000;
// Millis to start manual watering
unsigned long _startWateringMillis = 0;
// Flag to indicate if an ACK message has been received for the last message sent
bool _acked = false;

// State of the system
enum State {
    ready,
    watering,
    selecting,
    shutdown
};

State state;

// The selected station ID in the selecting state
int selectedStationId = 0;

// Reader for the green and the red buttons
Bounce greenButton = Bounce();
Bounce redButton = Bounce();

void before() {
    // initialize the lcd
    lcd.init();
    msg("Initializing...", "Node ver: " NODE_VERSION); // Print a message to the LCD.
    
    for (int index = 1; index <= NUMBER_OF_RELAYS; index++) {
        // Then set relay pins in output mode
        pinMode(indexToRelayPin(index), OUTPUT);
        // Set relay to OFF
        digitalWrite(indexToRelayPin(index), RELAY_OFF);
        saveState(indexToSensorId(index), false);
    }

    greenButton.attach(GREEN_BUTTON_PIN, INPUT_PULLUP);
    greenButton.interval(5);
    redButton.attach(RED_BUTTON_PIN, INPUT_PULLUP);
    redButton.interval(5);

    setState(ready);
}

void presentation()
{
    // Send the sketch version information to the gateway and Controller
    sendSketchInfo("Sprinkler 2", NODE_VERSION);

    present(SENSOR_ID_LCD, S_INFO);
    wait(50);

    for (int index = 1; index <= NUMBER_OF_RELAYS; index++) {
        // Register all sensors to gw (they will be created as child devices)
        present(indexToSensorId(index), S_BINARY);
        wait(50);
    }

    msg("Ready", "Node ver: " NODE_VERSION);
}

void loop()
{
    if (_lcdOffMillis != 0 && millis() > _lcdOffMillis && state != watering) {
        lcd.noBacklight();
        Serial.println("Turn off backlight");
        _lcdOffMillis = 0;
    }
    if (millis() > _heartbeatMillis) {
        sendHeartbeat();
        _heartbeatMillis += HEARTBEAT_INTERVAL;
    }
    switch (state) {
        case ready:
            if (isGreenButtonPushed()) {
                setState(selecting);
                selectedStationId = 1;
                msg("Station selected: ", selectedStationId);
                delayForStartWatering();
                return;
            }
            if (isRedButtonPushed()) {
                setState(shutdown);
                msg("System shutdown", "Green to turn on");
                return;
            }
            break;
        case selecting:
            if (isGreenButtonPushed()) {
                if (++selectedStationId > NUMBER_OF_RELAYS) {
                    selectedStationId = 1;
                }
                msg("Station selected: ", selectedStationId);
                delayForStartWatering();
                return;
            }
            if (isRedButtonPushed()) {
                cancelManualWatering();
                msg("Manual watering", "cancelled");
                setState(ready);
                return;
            }
            if (_startWateringMillis != 0 && millis() >= _startWateringMillis) {
                setStation(selectedStationId, HIGH, true);
                setState(watering);
                return;
            }
            break;
        case watering:
            if (isGreenButtonPushed()) {
                lcdlight();
                return;
            }
            if (isRedButtonPushed()) {
                Serial.println("Manually stopped");
                stopAllWatering();
                setState(ready);
                return;
            }
            for (int index = 1; index <= NUMBER_OF_RELAYS; ++index) {
                unsigned long offMillis = _waterOffMillis[index - 1];
                if (offMillis != 0 && millis() >= offMillis) {
                    Serial.print("Max watering time limit reached for station: ");
                    Serial.println(index);
                    _waterOffMillis[index - 1] = 0;
                    setStation(index, LOW, true);
                    if (!isAnyStationOn()) {
                        msg("All stations OFF", "Stand by");
                        setState(ready);
                        return;
                    }
                }
            }
            break;
        case shutdown:
            if (isGreenButtonPushed()) {
                setState(ready);
                msg("Ready", "");
            }
            return;
    }
}

int isGreenButtonPushed() {
    greenButton.update();
    int greenVal = greenButton.fell();
    if (greenVal) {
        Serial.print("Green button:");
        Serial.println(greenVal);
    }
    return greenVal;
}

int isRedButtonPushed() {
    redButton.update();
    int redVal = redButton.fell();
    if (redVal) {
        Serial.print("Red button:");
        Serial.println(redVal);
    }
    return redVal;
}

void receive(const MyMessage &message) {
    if (message.isAck()) {
        _acked = true;
        Serial.println("ACK received.");
        return;
    }
    if (message.type == V_STATUS) {
        int sensor = message.sensor;
        int value = message.getBool();
        int stationId = sensorIdToIndex(sensor);
        
        // Write some debug info
        Serial.print("Incoming change for sensor:");
        Serial.print(sensor);
        Serial.print(", New status: ");
        Serial.println(value);

        switch (state) {
            case ready:
                if (value) {
                    setStation(stationId, value, false);
                    Serial.println("Start watering.");
                    setState(watering);
                }
                break;
            case selecting:
                if (value) {
                    setStation(stationId, value, false);
                    cancelManualWatering();
                    setState(watering);
                }
                break;
            case watering:
                if (!value) {
                    setStation(stationId, value, false);
                    if (!isAnyStationOn()) {
                        msg("All stations OFF", "Stand by");
                        setState(ready);
                    }
                }
                break;
            case shutdown:
                Serial.println("System in shutdown; message ignored.");
                break;
        }
    }
    else if (message.type == V_TEXT) {
        const char* text = message.getString();
        msg("Server:", text);
    }
}

void setStation(int index, int value, bool sendState) {
    char str[100];
    snprintf(str, sizeof(str), "Station %d:", index);
    msg(str, value ? "ON" : "OFF");

    // Set relay
    digitalWrite(indexToRelayPin(index), value ? RELAY_ON : RELAY_OFF);

    // Store state in eeprom
    saveState(indexToSensorId(index), value);

    if (value) {
        // If turning station ON, schedule the max off time
        _waterOffMillis[index - 1] = millis() + MaxWaterDuration;
    }
    else {
        // Otherwise clear the timer
        _waterOffMillis[index - 1] = 0;
    }

    if (!sendState) {
        return;
    }

    // Report state to controller
    MyMessage message = MyMessage(indexToSensorId(index), V_STATUS);
    message.set(value ? 1 : 0);
    _acked = false;
    for (byte i = 0; i < MAX_SEND_ATTEMPTS; i++) {
        send(message, true);
        wait(40);
        if (_acked) {
            Serial.println("GW ACK");
            break;
        }
        Serial.println("GW NACK");
        wait(40 * (1 << i));
    }
}

void stopAllWatering() {
    for (int index = 1; index <= NUMBER_OF_RELAYS; ++index) {
        setStation(index, LOW, true);
        wait(50);
    }
    msg("Stopped all", "stations");
}

bool isAnyStationOn() {
    for (int sensor = RELAY_1_SENSOR_ID; sensor <= NUMBER_OF_RELAYS; ++sensor) {
        if (loadState(sensor)) {
            return true;
        }
    }
    return false;
}

void delayForStartWatering() {
    _startWateringMillis = millis() + StartWateringDelay;
}

void cancelManualWatering() {
    _startWateringMillis = 0;
}

int indexToSensorId(int index) {
    return index - 1 + RELAY_1_SENSOR_ID;
}

int sensorIdToIndex(int sensorId) {
    return sensorId - RELAY_1_SENSOR_ID + 1;
}

int indexToRelayPin(int index) {
    return index - 1 + RELAY_1_PIN;
}

void setState(State newState) {
    Serial.print("State=");
    switch(newState) {
        case ready:
            Serial.println("ready");
            break;
        case selecting:
            Serial.println("selecting");
            break;
        case watering:
            Serial.println("watering");
            break;
    }
    state = newState;
}

void lcdlight() {
    lcd.backlight();
    _lcdOffMillis = millis() + LcdOnDurationMillis;
}

void msg(const char line1[], const char line2[]) {
    Serial.println(line1);
    Serial.println(line2);
    lcd.clear();
    lcdlight();
    lcd.print(line1);
    lcd.setCursor(0,1); //newline
    lcd.print(line2);
}

void msg(const char line1[], int line2) {
    Serial.println(line1);
    Serial.println(line2);
    lcd.clear();
    lcdlight();
    lcd.print(line1);
    lcd.setCursor(0,1); //newline
    lcd.print(line2);
}
