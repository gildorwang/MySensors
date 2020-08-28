#pragma once
#include <MySensorsCommon.h>
#include <SPI.h>

class RadioMotionSensor : public ISensor
{
private:
    uint8_t _pin;
    uint8_t _sensorId;
    MessageSender _messageSender;
    bool _lastValue = false;
    unsigned long _lastReportMillis;
    const unsigned long MinReportInterval = 5000;
public:
    RadioMotionSensor(uint8_t pin, uint8_t sensorId, MessageSender messageSender): _pin(pin), _sensorId(sensorId), _messageSender(messageSender) {
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
        if (::millis() < this->_lastReportMillis + this->MinReportInterval && tripped == this->_lastValue) {
            return false;
        }
        this->_lastValue = tripped;
        this->_lastReportMillis = ::millis();
        MyMessage msg(this->_sensorId, V_TRIPPED);
        this->_messageSender.send(msg.set(tripped ? "1" : "0"));
        ::wait(40);
        return true;
    }

    void sleepForInterrupt(uint32_t sleepTime) {
        // Sleep until interrupt comes in on motion sensor. Send update every two minute.
        ::sleep(digitalPinToInterrupt(this->_pin), CHANGE, sleepTime);
    }

    ~RadioMotionSensor() { }
};
