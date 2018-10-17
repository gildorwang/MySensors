#define MY_NODE_ID 21

// Enable repeater functionality for this node
#define MY_REPEATER_FEATURE

#include <MySensorsCommon.h>
#include <DhtSensor.h>
#include <GasSensor.h>
#include <MotionSensor.h>

#define         CHILD_ID_MQ                   0
#define         CHILD_ID_MOTION               1
#define         CHILD_ID_TEMPERATURE          2
#define         CHILD_ID_HUMIDITY             3
/************************Hardware Related Macros************************************/
#define         MQ_SENSOR_ANALOG_PIN         (0)  //define which analog input channel you are going to use
#define         MOTION_SENSOR_PIN            (3)  //define which digital input pin to use for motion sensor
#define         DHT_PIN                      (4)  //define which digital input pin to use for dht pin

const long UpdateInterval = 30000; // Wait time between reads (in milliseconds)
unsigned long _nextUpdateMillis = 0;
volatile bool _interruptted = false;

MessageSender _messageSender;
DhtSensor _dhtSensor(DHT_PIN, CHILD_ID_TEMPERATURE, CHILD_ID_HUMIDITY, _messageSender);
GasSensor _gasSensor(MQ_SENSOR_ANALOG_PIN, CHILD_ID_MQ, _messageSender);
MotionSensor _motionSensor(MOTION_SENSOR_PIN, CHILD_ID_MOTION, _messageSender);
ISensor* _sensors[3] = { &_gasSensor, &_dhtSensor, &_motionSensor };

void isr() {
    _interruptted = true;
}

void setup()
{
    Serial.println("Setting up sensors...");
    for (ISensor* sensor : _sensors) {
        sensor->setup();
    }

    Serial.println("Setting up interrupt...");
    attachInterrupt(digitalPinToInterrupt(MOTION_SENSOR_PIN), isr, CHANGE);
}

void presentation()
{
    // Send the sketch version information to the gateway and Controller
    sendSketchInfo("Garage Gas Sensor", "3.1");
    
    for (ISensor* sensor : _sensors) {
        sensor->present();
        delay(40);
    }
}

void loop()
{
    unsigned long now = millis();
    if (_interruptted) {
        _interruptted = false;
        _motionSensor.report();
    }
    if (now > _nextUpdateMillis) {
        _nextUpdateMillis = now + UpdateInterval;
        
        for (ISensor* sensor : _sensors) {
            sensor->report();
            wait(40);
        }
    }
}

void receive(const MyMessage &message) {
  _messageSender.handleAck(message);
}
