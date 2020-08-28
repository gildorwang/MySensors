#pragma once
#include <DHT.h>
#include <MySensorsCommon.h>
#include <SPI.h>

class DhtSensor : public ISensor {
  private:
    static constexpr long UpdateInterval = 30000; // Wait time between reports (in milliseconds)
    static constexpr float MinValidTemperature = -40;
    static constexpr float MaxValidTemperature = 176;
    static constexpr float MinValidHumidity = 0;
    static constexpr float MaxValidHumidity = 100;
    uint8_t _pin;
    uint8_t _temperatureSensorId;
    uint8_t _humiditySensorId;
    bool _isMetric;
    DHT _dht;
    MessageSender _messageSender;
    // Set this offset if the sensor has a permanent small offset to the real temperatures
    float _temperatureOffset;
    unsigned long _nextUpdateMillis = 0;

    static bool _isValidTemperature(float temperature) {
        return !isnan(temperature) &&
               temperature >= MinValidTemperature &&
               temperature <= MaxValidTemperature;
    }

    static bool _isValidHumidity(float humidity) {
        return !isnan(humidity) &&
               humidity >= MinValidHumidity &&
               humidity <= MaxValidHumidity;
    }

  public:
    DhtSensor(uint8_t pin, uint8_t temperatureSensorId, uint8_t humiditySensorId, MessageSender messageSender, float temperatureOffset = 0)
        : _pin(pin), _temperatureSensorId(temperatureSensorId), _humiditySensorId(humiditySensorId), _messageSender(messageSender), _temperatureOffset(temperatureOffset) {}

    void setup() {
        this->_dht.setup(this->_pin); // set data pin of DHT sensor
    }

    void present() {
        ::present(this->_temperatureSensorId, S_TEMP, "Temperature");
        ::wait(40);
        ::present(this->_humiditySensorId, S_HUM, "Humidity");
        ::wait(40);
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

        if (!DhtSensor::_isValidTemperature(temperature)) {
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

        if (!DhtSensor::_isValidHumidity(humidity)) {
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
        unsigned long now = millis();
        if (now > this->_nextUpdateMillis) {
            this->_nextUpdateMillis = now + this->UpdateInterval;
            this->forceRead();
            bool success = this->reportTemperature();
            ::wait(40);
            success = success && this->reportHumidity();
            return success;
        }
        return false;
    }

    void forceRead() {
        this->_dht.readSensor(true);
    }

    ~DhtSensor() {}
};