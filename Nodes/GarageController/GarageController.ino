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

#define MY_NODE_ID 7

// Enable repeater functionality for this node
#define MY_REPEATER_FEATURE

#include <SPI.h>
#include <MySensors.h>

#define RELAY_ON 0  // GPIO value to write to turn on attached relay
#define RELAY_OFF 1 // GPIO value to write to turn off attached relay

#define HEARTBEAT_INTERVAL 30000
unsigned long nextHeartbeatMillis = 0;

class ControlGroup {
  private:
    bool lastSensorState = false;

    MyMessage msgSensorState;

    bool IsDoorOpen() {
      return digitalRead(this->SensorPin) == HIGH;
    }
  
    void SetControl(bool state) {
      digitalWrite(this->ControlPin, state ? RELAY_ON : RELAY_OFF);
      // Store state in eeprom
      saveState(this->ControlId, state);

      // Write some debug info
      Serial.print("Set door control with Sensor ID:");
      Serial.print(this->ControlId);
      Serial.print(", New status: ");
      Serial.println(state);
    }
    
  public:
    int SensorId;
    int ControlId;
    int SensorPin;
    int ControlPin;

    ControlGroup(int sensorId, int controlId, int sensorPin, int controlPin): SensorId(sensorId), ControlId(controlId), SensorPin(sensorPin), ControlPin(controlPin), msgSensorState(sensorId, V_TRIPPED) {
    }

    void Init() {
      pinMode(this->SensorPin, INPUT);
      pinMode(this->ControlPin, OUTPUT);

      SetControl(loadState(this->ControlId));
    }

    void Present() {
      present(this->ControlId, S_LIGHT);
      present(this->SensorId, S_DOOR);
    }

    void Receive(const MyMessage &message) {
      if (message.type == V_LIGHT && message.sensor == this->ControlId) {
        // Operate the door relay
        SetControl(message.getBool());
      }
    }

    void CheckSensor() {
      bool state = IsDoorOpen();
      if (state != this->lastSensorState) {
        this->lastSensorState = state;
        SendState();
      }
    }

    void SendState() {
      send(msgSensorState.set(this->lastSensorState));
      Serial.print("Sending state for sensor ID: ");
      Serial.print(this->SensorId);
      Serial.print(", Value: ");
      Serial.print(this->lastSensorState);
      Serial.println();
    }
};

ControlGroup door1(1, 2, 2, 3);
ControlGroup door2(3, 4, 4, 5);

void before() { 
  door1.Init();
  door2.Init();
}

void setup() {
  nextHeartbeatMillis = millis() + HEARTBEAT_INTERVAL;
}

void presentation()  
{   
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("Garage controller", "2.0");
  door1.Present();
  door2.Present();
}


void loop() 
{
  unsigned long now = millis();
  if (now >= nextHeartbeatMillis) {
    nextHeartbeatMillis = now + HEARTBEAT_INTERVAL;
    #ifdef MY_DEBUG
    Serial.print("Sending status as heartbeat.");
    #endif

    door1.SendState();
    door2.SendState();
  }
  door1.CheckSensor();
  door2.CheckSensor();
}

void receive(const MyMessage &message) {
  door1.Receive(message);
  door2.Receive(message);
}



