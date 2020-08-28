#define MY_NODE_ID 21

// Enable repeater functionality for this node
#define MY_REPEATER_FEATURE

#include <DhtSensor.h>
#include <GasSensor.h>
#include <MySensorsCommon.h>

#define CHILD_ID_TEMPERATURE 2
#define CHILD_ID_HUMIDITY 3
#define CHILD_ID_LPG 4
#define CHILD_ID_CO 5
#define CHILD_ID_SMOKE 6
/************************Hardware Related Macros************************************/
#define MQ_SENSOR_ANALOG_PIN (0) //define which analog input channel you are going to use
#define DHT_PIN (4)              //define which digital input pin to use for dht pin

MessageSender _messageSender;
DhtSensor _dhtSensor(DHT_PIN, CHILD_ID_TEMPERATURE, CHILD_ID_HUMIDITY, _messageSender);
GasSensor _gasSensor(MQ_SENSOR_ANALOG_PIN, CHILD_ID_LPG, CHILD_ID_CO, CHILD_ID_SMOKE, _messageSender);
ISensor *_sensors[2] = {&_gasSensor, &_dhtSensor};

void setup() {
    Serial.println("Setting up sensors...");
    for (ISensor *sensor : _sensors) {
        sensor->setup();
    }
}

void presentation() {
    // Send the sketch version information to the gateway and Controller
    sendSketchInfo("Garage Gas Sensor", "3.3");

    for (ISensor *sensor : _sensors) {
        sensor->present();
    }
}

void loop() {
    for (ISensor *sensor : _sensors) {
        sensor->report();
    }
}

void receive(const MyMessage &message) {
    _messageSender.handleAck(message);
}
