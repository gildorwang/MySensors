#pragma once
#include <MySensorsCommon.h>

class DimmerSensor : public ISensor
{
    private:
        uint8_t _pin;
        uint8_t _sensorId;
        MessageSender _messageSender;
    public:
        DimmerSensor(uint8_t pin, uint8_t sensorId, MessageSender messageSender)
            : _pin(pin), _sensorId(sensorId), _messageSender(messageSender)
        {
            // Initialize the dimmer to 50%
            ::pinMode(this->_pin, OUTPUT);
            ::analogWrite(this->_pin, 127);
        }

        void setup() {
            Serial.print("Initializing DimmerSensor with pin ");
            Serial.println(this->_pin);
            uint8_t prevValue = ::loadState(this->_sensorId);
            Serial.print("Restoring dimmer value to ");
            Serial.println(prevValue);
            ::analogWrite(this->_pin, prevValue);
        }

        void present() {
            ::present(this->_sensorId, S_DIMMER);
        }

        bool report() {
        }

        void set(uint8_t percentage) {
            uint8_t outputValue = ::map(percentage, 0, 100, 0, 255);
            ::saveState(this->_sensorId, outputValue);
            #ifdef MY_DEBUG
            Serial.print("Setting DimmerSensor value to ");
            Serial.println(outputValue);
            #endif
            ::analogWrite(this->_pin, outputValue);
        }
};
