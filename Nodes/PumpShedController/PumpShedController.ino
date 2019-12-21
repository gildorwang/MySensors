#define MY_NODE_ID 56
//#define MY_DEBUG 
//#define MY_DEBUG_VERBOSE_RFM69

// Enable repeater functionality for this node
#define MY_REPEATER_FEATURE

#include <MySensorsCommon.h>
#include <DhtSensor.h>
#include <DimmerSensor.h>
//#include <RadioMotionSensor.h>

#define         CHILD_ID_DIMMER               0
#define         CHILD_ID_TEMPERATURE          1
#define         CHILD_ID_HUMIDITY             2
#define         CHILD_ID_MOTION               3
/************************Hardware Related Macros************************************/
#define         DIMMER_PIN                   (3)
#define         DHT_PIN                      (4)
#define         MOTION_PIN                   (5)

const uint8_t MaxDimmerValue = 60;
// const uint8_t InvalidDimmerValue = 100;
// uint8_t _dimmerValue = 0;

MessageSender _messageSender;
DhtSensor _dhtSensor(DHT_PIN, CHILD_ID_TEMPERATURE, CHILD_ID_HUMIDITY, _messageSender);
DimmerSensor _dimmerSensor(DIMMER_PIN, CHILD_ID_DIMMER, _messageSender);
//RadioMotionSensor _radioMotionSensor(MOTION_PIN, CHILD_ID_MOTION, _messageSender);
ISensor* _sensors[2] = { &_dimmerSensor, &_dhtSensor/* , &_radioMotionSensor */ };

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
    sendSketchInfo("Pump Shed Control", "3.3");
    
    for (ISensor* sensor : _sensors) {
        sensor->present();
    }
}

void loop() {
    for (ISensor* sensor : _sensors) {
        sensor->report();
    }
    // if (_radioMotionSensor.read()) {
    //     uint8_t currentDimmerValue = _dimmerSensor.read();
    //     if (_dimmerValue == InvalidDimmerValue && currentDimmerValue > 0) {
    //         _dimmerValue = currentDimmerValue;
    //         _dimmerSensor.set(MaxDimmerValue);
    //     }
    // } else if (_dimmerValue != InvalidDimmerValue) {
    //     _dimmerSensor.set(_dimmerValue);
    //     _dimmerValue = InvalidDimmerValue;
    // }
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
        // _dimmerValue = InvalidDimmerValue;
    }
}
