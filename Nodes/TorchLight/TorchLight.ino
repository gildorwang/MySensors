#define MY_NODE_ID 31

// Enabled repeater feature for this node
#define MY_REPEATER_FEATURE

#include <MySensorsCommon.h>
#include <DhtSensor.h>
#include <DimmerSensor.h>

#define CHILD_ID_DIMMER               0
#define CHILD_ID_TEMPERATURE          1
#define CHILD_ID_HUMIDITY             2

// Set this to the pin you connected the DHT's data pin to
#define DHT_PIN 3
#define LED_PIN 4

// Set this offset if the sensor has a permanent small offset to the real temperatures
#define SENSOR_TEMP_OFFSET 0

const uint64_t UpdateInterval = 30000;
unsigned long _nextUpdateMillis = 0;

MessageSender _messageSender;
DhtSensor _dhtSensor(DHT_PIN, CHILD_ID_TEMPERATURE, CHILD_ID_HUMIDITY, _messageSender);
DimmerSensor _dimmerSensor(LED_PIN, CHILD_ID_DIMMER, _messageSender);
ISensor* _sensors[2] = { &_dimmerSensor, &_dhtSensor };

void setup() {
    Serial.println("Setting up sensors...");
    for (ISensor* sensor : _sensors) {
        sensor->setup();
    }
}

void presentation() {
    // Send the sketch version information to the gateway
    sendSketchInfo("Torch Light", "3.1");
    
    for (ISensor* sensor : _sensors) {
        sensor->present();
        delay(40);
    }
}

void loop() {
    unsigned long now = millis();
    if (now > _nextUpdateMillis) {
        _nextUpdateMillis = now + UpdateInterval;

        for (ISensor* sensor : _sensors) {
            sensor->report();
            wait(40);
        }
    }
}

void receive(const MyMessage &message) {
    if (_messageSender.handleAck(message)) {
        return;
    }
    if (message.type == V_PERCENTAGE && message.sensor == CHILD_ID_DIMMER) {
        uint8_t value = message.getByte();
        Serial.print("Incoming dimmer value: ");
        Serial.println(value);
        // The LED does not support dimming
        if (value >= 50) {
            value = 100;
        }
        else {
            value = 0;
        }
        _dimmerSensor.set(value);
    }
}

