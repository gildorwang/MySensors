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
 * DESCRIPTION
 *
 * Connect the MQ2 sensor as follows :
 *
 *   A H A   >>> 5V
 *   B       >>> A0
 *   H       >>> GND
 *   B       >>> 10K ohm >>> GND
 *
 * Contribution: epierre
 * Based on http://sandboxelectronics.com/?p=165
 * License: Attribution-NonCommercial-ShareAlike 3.0 Unported (CC BY-NC-SA 3.0)
 * Modified by HEK to work in 1.4
 *
 */
#pragma once
#include <MySensorsCommon.h>
#include <SPI.h>

/************************Hardware Related Macros************************************/
#define RL_VALUE (5)               //define the load resistance on the board, in kilo ohms
#define RO_CLEAN_AIR_FACTOR (9.83) //RO_CLEAR_AIR_FACTOR=(Sensor resistance in clean air)/RO,
//which is derived from the chart in datasheet
/***********************Software Related Macros************************************/
#define CALIBRATION_SAMPLE_TIMES (50)     //define how many samples you are going to take in the calibration phase
#define CALIBRATION_SAMPLE_INTERVAL (500) //define the time interal(in milisecond) between each samples in the
//cablibration phase
#define READ_SAMPLE_INTERVAL (50) //define how many samples you are going to take in normal operation
#define READ_SAMPLE_TIMES (5)     //define the time interal(in milisecond) between each samples in
//normal operation
/**********************Application Related Macros**********************************/
#define GAS_LPG (0)
#define GAS_CO (1)
#define GAS_SMOKE (2)
/*****************************Globals***********************************************/

class GasSensor : public ISensor {
  private:
    static constexpr long UpdateInterval = 30000; // Wait time between reports (in milliseconds)
    unsigned long _nextUpdateMillis = 0;
    uint8_t _pin;
    uint8_t _lpgSensorId;
    uint8_t _coSensorId;
    uint8_t _smokeSensorId;
    MessageSender _messageSender;

    //VARIABLES
    float Ro = 10000.0; // this has to be tuned 10K Ohm
    int val = 0;        // variable to store the value coming from the sensor
    float lastMQ = 0.0;
    float LPGCurve[3] = {2.3, 0.21, -0.47}; //two points are taken from the curve.
    //with these two points, a line is formed which is "approximately equivalent"
    //to the original curve.
    //data format:{ x, y, slope}; point1: (lg200, 0.21), point2: (lg10000, -0.59)
    float COCurve[3] = {2.3, 0.72, -0.34}; //two points are taken from the curve.
    //with these two points, a line is formed which is "approximately equivalent"
    //to the original curve.
    //data format:{ x, y, slope}; point1: (lg200, 0.72), point2: (lg10000,  0.15)
    float SmokeCurve[3] = {2.3, 0.53, -0.44}; //two points are taken from the curve.
    //with these two points, a line is formed which is "approximately equivalent"
    //to the original curve.
    //data format:{ x, y, slope}; point1: (lg200, 0.53), point2:(lg10000,-0.22)

    /****************** _mqResistanceCalculation ****************************************
    Input:   raw_adc - raw value read from adc, which represents the voltage
    Output:  the calculated sensor resistance
    Remarks: The sensor and the load resistor forms a voltage divider. Given the voltage
            across the load resistor and its resistance, the resistance of the sensor
            could be derived.
    ************************************************************************************/
    float _mqResistanceCalculation(int raw_adc) {
        return (((float)RL_VALUE * (1023 - raw_adc) / raw_adc));
    }

    /***************************** _mqCalibration ****************************************
    Input:   mq_pin - analog channel
    Output:  Ro of the sensor
    Remarks: This function assumes that the sensor is in clean air. It use
            _mqResistanceCalculation to calculates the sensor resistance in clean air
            and then divides it with RO_CLEAN_AIR_FACTOR. RO_CLEAN_AIR_FACTOR is about
            10, which differs slightly between different sensors.
    ************************************************************************************/
    float _mqCalibration(int mq_pin) {
        int i;
        float val = 0;

        for (i = 0; i < CALIBRATION_SAMPLE_TIMES; i++) { //take multiple samples
            val += this->_mqResistanceCalculation(analogRead(mq_pin));
            delay(CALIBRATION_SAMPLE_INTERVAL);
        }
        val = val / CALIBRATION_SAMPLE_TIMES; //calculate the average value

        val = val / RO_CLEAN_AIR_FACTOR; //divided by RO_CLEAN_AIR_FACTOR yields the Ro
        //according to the chart in the datasheet

        return val;
    }
    /*****************************  _mqRead *********************************************
    Input:   mq_pin - analog channel
    Output:  Rs of the sensor
    Remarks: This function use _mqResistanceCalculation to calculate the sensor resistenc (Rs).
            The Rs changes as the sensor is in the different consentration of the target
            gas. The sample times and the time interval between samples could be configured
            by changing the definition of the macros.
    ************************************************************************************/
    float _mqRead(int mq_pin) {
        int i;
        float rs = 0;

        for (i = 0; i < READ_SAMPLE_TIMES; i++) {
            rs += this->_mqResistanceCalculation(analogRead(mq_pin));
            ::wait(READ_SAMPLE_INTERVAL);
        }

        rs = rs / READ_SAMPLE_TIMES;

        return rs;
    }

    /*****************************  _mqGetGasPercentage **********************************
    Input:   rs_ro_ratio - Rs divided by Ro
            gas_id      - target gas type
    Output:  ppm of the target gas
    Remarks: This function passes different curves to the _mqGetPercentage function which
            calculates the ppm (parts per million) of the target gas.
    ************************************************************************************/
    int _mqGetGasPercentage(float rs_ro_ratio, int gas_id) {
        if (gas_id == GAS_LPG) {
            return this->_mqGetPercentage(rs_ro_ratio, LPGCurve);
        } else if (gas_id == GAS_CO) {
            return this->_mqGetPercentage(rs_ro_ratio, COCurve);
        } else if (gas_id == GAS_SMOKE) {
            return this->_mqGetPercentage(rs_ro_ratio, SmokeCurve);
        }

        return 0;
    }

    /*****************************  _mqGetPercentage **********************************
    Input:   rs_ro_ratio - Rs divided by Ro
            pcurve      - pointer to the curve of the target gas
    Output:  ppm of the target gas
    Remarks: By using the slope and a point of the line. The x(logarithmic value of ppm)
            of the line could be derived if y(rs_ro_ratio) is provided. As it is a
            logarithmic coordinate, power of 10 is used to convert the result to non-logarithmic
            value.
    ************************************************************************************/
    int _mqGetPercentage(float rs_ro_ratio, float *pcurve) {
        return (pow(10, (((log(rs_ro_ratio) - pcurve[1]) / pcurve[2]) + pcurve[0])));
    }

    bool _reportLpg(float rs) {
        if (this->_lpgSensorId != InvalidSensorId) {
            int valMQ = this->_mqGetGasPercentage(rs / this->Ro, GAS_LPG);
            Serial.print("LPG:");
            Serial.print(valMQ);
            Serial.println("ppm");
            MyMessage msg(this->_lpgSensorId, V_LEVEL);
            this->_messageSender.send(msg.set(valMQ));
            return true;
        }
        return false;
    }

    bool _reportCo(float rs) {
        if (this->_coSensorId != InvalidSensorId) {
            int valMQ = this->_mqGetGasPercentage(rs / this->Ro, GAS_CO);
            Serial.print("CO:");
            Serial.print(valMQ);
            Serial.println("ppm");
            MyMessage msg(this->_coSensorId, V_LEVEL);
            this->_messageSender.send(msg.set(valMQ));
            return true;
        }
        return false;
    }

    bool _reportSmoke(float rs) {
        if (this->_smokeSensorId != InvalidSensorId) {
            int valMQ = this->_mqGetGasPercentage(rs / this->Ro, GAS_SMOKE);
            Serial.print("SMOKE:");
            Serial.print(valMQ);
            Serial.println("ppm");
            MyMessage msg(this->_smokeSensorId, V_LEVEL);
            this->_messageSender.send(msg.set(valMQ));
            return true;
        }
        return false;
    }

  public:
    GasSensor(uint8_t pin, uint8_t lpgSensorId, uint8_t coSensorId, uint8_t smokeSensorId, MessageSender messageSender) : _pin(pin),
                                                                                                                          _lpgSensorId(lpgSensorId),
                                                                                                                          _coSensorId(coSensorId),
                                                                                                                          _smokeSensorId(smokeSensorId),
                                                                                                                          _messageSender(messageSender) {
    }

    void setup() {
        this->Ro = this->_mqCalibration(this->_pin); //Calibrating the sensor. Please make sure the sensor is in clean air
    }

    void present() {
        ::present(this->_lpgSensorId, S_AIR_QUALITY, "LPG");
        ::present(this->_coSensorId, S_AIR_QUALITY, "CO");
        ::present(this->_smokeSensorId, S_AIR_QUALITY, "Smoke");
        ::wait(40);
    }

    float read() {
        return this->_mqRead(this->_pin);
    }

    bool report() {
        unsigned long now = millis();
        if (now > this->_nextUpdateMillis) {
            this->_nextUpdateMillis = now + this->UpdateInterval;

            float rs = this->read();
            if (this->_reportLpg(rs)) {
                ::wait(40);
            }
            if (this->_reportCo(rs)) {
                ::wait(40);
            }
            if (this->_reportSmoke(rs)) {
                ::wait(40);
            }
            return true;
        }
        return false;
    }

    ~GasSensor() {}
};