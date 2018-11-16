#define MY_NODE_ID 56

// Enable repeater functionality for this node
#define MY_REPEATER_FEATURE

#include <MySensorsCommon.h>
#include <DhtSensor.h>
#include <DimmerSensor.h>

#define         CHILD_ID_DIMMER               0
#define         CHILD_ID_TEMPERATURE          1
#define         CHILD_ID_HUMIDITY             2
/************************Hardware Related Macros************************************/
#define         DIMMER_PIN                   (3)  //define which digital input pin to use for motion sensor
#define         DHT_PIN                      (4)  //define which digital input pin to use for dht pin

const long UpdateInterval = 30000; // Wait time between reads (in milliseconds)
const uint8_t MaxDimmerValue = 60;
unsigned long _nextUpdateMillis = 0;

MessageSender _messageSender;
DhtSensor _dhtSensor(DHT_PIN, CHILD_ID_TEMPERATURE, CHILD_ID_HUMIDITY, _messageSender);
DimmerSensor _dimmerSensor(DIMMER_PIN, CHILD_ID_DIMMER, _messageSender);
ISensor* _sensors[2] = { &_dimmerSensor, &_dhtSensor };

void setup()
{
    Serial.println("Setting up sensors...");
    for (ISensor* sensor : _sensors) {
        sensor->setup();
    }
}

void presentation()
{
    // Send the sketch version information to the gateway and Controller
    sendSketchInfo("Pump Shed Control", "3.1");
    
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
    if (message.type == V_PERCENTAGE && message.sensor == CHILD_ID_DIMMER) {
        uint8_t value = message.getByte();
        Serial.print("Incoming dimmer value: ");
        Serial.println(value);
        if (value > MaxDimmerValue) {
            value = MaxDimmerValue;
            Serial.print("Coerced to ");
            Serial.println(MaxDimmerValue);
        }
        _dimmerSensor.set(value);
    }
}
