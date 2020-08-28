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
 * Motion Sensor example using HC-SR501 
 * http://www.mysensors.org/build/motion
 *
 */
#pragma once
#include <MySensorsCommon.h>
#include <SPI.h>

#define pinToInterrupt(p) ((p) == 2 ? 0 : (p) == 3 ? 1 : NOT_AN_INTERRUPT)

class MotionSensor : public ISensor
{
private:
    uint8_t _pin;
    uint8_t _sensorId;
    MessageSender _messageSender;
    bool _lastValue = false;
    unsigned long _lastReportMillis;
    const unsigned long MinReportInterval = 5000;
public:
    MotionSensor(uint8_t pin, uint8_t sensorId, MessageSender messageSender): _pin(pin), _sensorId(sensorId), _messageSender(messageSender) {
    }

    void setup() {
        pinMode(this->_pin, INPUT);      // sets the motion sensor digital pin as input
    }

    void present() {
        ::present(this->_sensorId, S_MOTION);
        ::wait(40);
    }

    bool read() {
        // Read digital motion value
        bool tripped = digitalRead(this->_pin) == HIGH;
        return tripped;
    }

    bool report() {
        bool tripped = this->read();
        if (tripped != this->_lastValue) {
            Serial.println(tripped ? "Tripped" : "Not tripped");
        }
        if (millis() < this->_lastReportMillis + MinReportInterval && tripped == this->_lastValue) {
            return false;
        }
        this->_lastValue = tripped;
        this->_lastReportMillis = millis();
        MyMessage msg(this->_sensorId, V_TRIPPED);
        this->_messageSender.send(msg.set(tripped ? "1" : "0"));
        ::wait(40);
        return true;
    }

    // Sleep until interrupt comes in on motion sensor or timeout
    void sleepForInterrupt(uint32_t sleepTime) {
        auto result = ::smartSleep(pinToInterrupt(this->_pin), CHANGE, sleepTime);
        Serial.print("Wake by ");
        Serial.println(result);
    }

    ~MotionSensor() { }
};