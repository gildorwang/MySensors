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
 * Version 2.0 - Enable signature, add node ID - gildorwang
 * 
 * DESCRIPTION
 * This sketch provides an example of how to implement a humidity/temperature
 * sensor using a DHT11/DHT-22.
 *  
 * For more information, please visit:
 * http://www.mysensors.org/build/humidity
 * 
 */

#define MY_NODE_ID 100

// Enabled repeater feature for this node
#define MY_REPEATER_FEATURE

#include <SPI.h>
#include <MySensors.h>  
#include <DHT.h>
#include <PWM.h>

//use pin 11 on the Mega instead, otherwise there is a frequency cap at 31 Hz
int fan = 3;                // the pin that the LED is attached to
int32_t frequency = 25000; //frequency (in Hz)

// Set this to the pin you connected the DHT's data pin to
#define DHT_DATA_PIN 5

// Set this offset if the sensor has a permanent small offset to the real temperatures
#define SENSOR_TEMP_OFFSET 0

// Sleep time between sensor updates (in milliseconds)
// Must be >1000ms for DHT22 and >2000ms for DHT11
static const uint64_t UPDATE_INTERVAL = 10000;

#define CHILD_ID_HUM 0
#define CHILD_ID_TEMP 1
#define LED_PIN 6

float lastTemp;
float lastHum;
bool metric = false;
unsigned long nextUpdateMillis;
unsigned long ledOffMillis;

MyMessage msgHum(CHILD_ID_HUM, V_HUM);
MyMessage msgTemp(CHILD_ID_TEMP, V_TEMP);
DHT dht;

void presentation()  
{ 
    // Send the sketch version information to the gateway
    sendSketchInfo("PowerSupplyMonitor", "2.0");
    
    // Register all sensors to gw (they will be created as child devices)
    present(CHILD_ID_HUM, S_HUM);
    present(CHILD_ID_TEMP, S_TEMP);
    
    metric = getControllerConfig().isMetric;
}


void setup()
{
    /********* Setup PWM for Fan control **********/
    //initialize all timers except for 0, to save time keeping functions
    InitTimersSafe(); 

    //sets the frequency for the specified pin
    bool success = SetPinFrequencySafe(fan, frequency);
    
    //if the pin frequency was set successfully, turn pin 13 on
    if(!success) {
        Serial.println("Error: Failed to set PWM");
    }

    /********* DHT Setup **********/
    pinMode(LED_PIN, OUTPUT);
    
    dht.setup(DHT_DATA_PIN); // set data pin of DHT sensor
    if (UPDATE_INTERVAL <= dht.getMinimumSamplingPeriod()) {
        Serial.println("Warning: UPDATE_INTERVAL is smaller than supported by the sensor!");
    }

    nextUpdateMillis = millis() + UPDATE_INTERVAL;
}


void loop()      
{  
    //Write PWM
    pwmWrite(fan, 220);

    unsigned long currentMillis = millis();
    if (currentMillis >= ledOffMillis) {
        digitalWrite(LED_PIN, LOW);
    }
    if (currentMillis >= nextUpdateMillis) {
        nextUpdateMillis = currentMillis + UPDATE_INTERVAL;
        
        // Force reading sensor, so it works also after sleep()
        dht.readSensor(true);
        
        bool isWorking = true;
        
        // Get temperature from DHT library
        float temperature = dht.getTemperature();
        if (isnan(temperature)) {
            isWorking = false;
            Serial.println("Failed reading temperature from DHT!");
        } else {
            lastTemp = temperature;
            if (!metric) {
                temperature = dht.toFahrenheit(temperature);
            }
            temperature += SENSOR_TEMP_OFFSET;
            send(msgTemp.set(temperature, 1));
    
            #ifdef MY_DEBUG
            Serial.print("T: ");
            Serial.println(temperature);
            #endif
        }
    
        // Get humidity from DHT library
        float humidity = dht.getHumidity();
        if (isnan(humidity)) {
            isWorking = false;
            Serial.println("Failed reading humidity from DHT");
        } else {
            lastHum = humidity;
            send(msgHum.set(humidity, 1));
            
            #ifdef MY_DEBUG
            Serial.print("H: ");
            Serial.println(humidity);
            #endif
        }

        if (isWorking) {
            digitalWrite(LED_PIN, HIGH);
            ledOffMillis = currentMillis + 50;
        }
    }
}
