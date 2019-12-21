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

/************************Hardware Related Macros************************************/
#define         RL_VALUE                     (5)     //define the load resistance on the board, in kilo ohms
#define         RO_CLEAN_AIR_FACTOR          (9.83)  //RO_CLEAR_AIR_FACTOR=(Sensor resistance in clean air)/RO,
//which is derived from the chart in datasheet
/***********************Software Related Macros************************************/
#define         CALIBRATION_SAMPLE_TIMES     (50)    //define how many samples you are going to take in the calibration phase
#define         CALIBRATION_SAMPLE_INTERVAL  (500)   //define the time interal(in milisecond) between each samples in the
//cablibration phase
#define         READ_SAMPLE_INTERVAL         (50)    //define how many samples you are going to take in normal operation
#define         READ_SAMPLE_TIMES            (5)     //define the time interal(in milisecond) between each samples in
//normal operation
/**********************Application Related Macros**********************************/
#define         GAS_LPG                      (0)
#define         GAS_CO                       (1)
#define         GAS_SMOKE                    (2)
/*****************************Globals***********************************************/


class MotionSensor : public ISensor
{
private:
    uint8_t _pin;
    uint8_t _sensorId;
    MessageSender _messageSender;
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
    }

    bool read() {
        // Read digital motion value
        bool tripped = digitalRead(this->_pin) == HIGH;
        Serial.println(tripped ? "Tripped" : "Not tripped");
        return tripped;
    }

    void report() {
        bool tripped = this->read();
        if (millis() < this->_lastReportMillis + MinReportInterval) {
            return;
        }
        this->_lastReportMillis = millis();
        MyMessage msg(this->_sensorId, V_TRIPPED);
        this->_messageSender.send(msg.set(tripped ? "1" : "0"));
    }

    void sleepForInterrupt(uint32_t sleepTime) {
        // Sleep until interrupt comes in on motion sensor. Send update every two minute.
        ::sleep(digitalPinToInterrupt(this->_pin), CHANGE, sleepTime);
    }

    ~MotionSensor() { }
};