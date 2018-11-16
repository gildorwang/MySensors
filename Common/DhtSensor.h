#pragma once
#include <MySensorsCommon.h>
#include <SPI.h>
#include <DHT.h>

class DhtSensor : public ISensor
{
private:
    uint8_t _pin;
    uint8_t _temperatureSensorId;
    uint8_t _humiditySensorId;
    bool _isMetric;
    DHT _dht;
    MessageSender _messageSender;
    // Set this offset if the sensor has a permanent small offset to the real temperatures
    float _temperatureOffset;
public:
    DhtSensor(uint8_t pin, uint8_t temperatureSensorId, uint8_t humiditySensorId, MessageSender messageSender, float temperatureOffset = 0)
        : _pin(pin), _temperatureSensorId(temperatureSensorId), _humiditySensorId(humiditySensorId), _messageSender(messageSender), _temperatureOffset(temperatureOffset)
    { }

    void setup() {
        this->_dht.setup(this->_pin); // set data pin of DHT sensor
    }

    void present() {
        ::present(this->_temperatureSensorId, S_TEMP);
        ::delay(40);
        ::present(this->_humiditySensorId, S_HUM);
        ::delay(40);
        this->_isMetric = false; //getControllerConfig().isMetric;
        Serial.print("Unit: ");
        Serial.println(this->_isMetric ? "metric" : "imperial");
    }

    float readTemperature() {
        // Get temperature from DHT library
        float temperature = this->_dht.getTemperature();
        if (!isnan(temperature)) {
            if (!this->_isMetric) {
                temperature = this->_dht.toFahrenheit(temperature);
            }
            temperature += this->_temperatureOffset;
        }
        return temperature;
    }

    bool reportTemperature() {
        float temperature = this->readTemperature();
        
        if (isnan(temperature)) {
            Serial.println("Failed reading temperature from DHT!");
            return false;
        }

        MyMessage msgTemp(this->_temperatureSensorId, V_TEMP);
        this->_messageSender.send(msgTemp.set(temperature, 1));
    
        Serial.print("T: ");
        Serial.println(temperature);
        return true;
    }

    float readHumidity() {
        // Get humidity from DHT library
        float humidity = this->_dht.getHumidity();
        return humidity;
    }

    bool reportHumidity() {
        float humidity = this->readHumidity();
        
        if (isnan(humidity)) {
            Serial.println("Failed reading humidity from DHT!");
            return false;
        }

        MyMessage msgTemp(this->_humiditySensorId, V_HUM);
        this->_messageSender.send(msgTemp.set(humidity, 1));
    
        Serial.print("H: ");
        Serial.println(humidity);
        return true;
    }

    bool report() {
        this->forceRead();
        bool success = this->reportTemperature();
        ::wait(40);
        success = success && this->reportHumidity();
        return success;
    }

    void forceRead() {
        this->_dht.readSensor(true);
    }

    ~DhtSensor() { }
};