#define MY_NODE_ID 22

// #include <DhtSensor.h>
#include <MotionSensor.h>
#include <MySensorsCommon.h>

#define CHILD_ID_MOTION 1
#define CHILD_ID_TEMPERATURE 2
#define CHILD_ID_HUMIDITY 3

#define MOTION_PIN (3)
#define DHT_PIN (4)

#define SLEEP_INTERVAL 10000

MessageSender _messageSender;
//DhtSensor _dhtSensor(DHT_PIN, CHILD_ID_TEMPERATURE, CHILD_ID_HUMIDITY, _messageSender);
MotionSensor _motionSensor(MOTION_PIN, CHILD_ID_MOTION, _messageSender);
ISensor *_sensors[1] = {&_motionSensor};

void setup() {
    Serial.println("Setting up sensors...");
    for (ISensor *sensor : _sensors) {
        sensor->setup();
    }
}

void presentation() {
    // Send the sketch version information to the gateway and Controller
    sendSketchInfo("Garage Motion Sensor", "3.0");

    for (ISensor *sensor : _sensors) {
        sensor->present();
    }
}

void loop() {
    for (ISensor *sensor : _sensors) {
        sensor->report();
    }
    _motionSensor.sleepForInterrupt(SLEEP_INTERVAL);
}

void receive(const MyMessage &message) {
    _messageSender.handleAck(message);
}
