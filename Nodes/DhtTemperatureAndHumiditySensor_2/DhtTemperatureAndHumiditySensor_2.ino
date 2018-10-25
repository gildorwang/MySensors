#define MY_NODE_ID 12

// Enabled repeater feature for this node
#define MY_REPEATER_FEATURE

#include <MySensorsCommon.h>
#include <DhtSensor.h>

// Set this to the pin you connected the DHT's data pin to
#define DHT_DATA_PIN 3

#define CHILD_ID_HUM 0
#define CHILD_ID_TEMP 1
#define LED_PIN 4

unsigned long _nextUpdateMillis = 0;
MessageSender _messageSender;
DhtSensor _dhtSensor(DHT_DATA_PIN, CHILD_ID_TEMP, CHILD_ID_HUM, _messageSender, -3);

const long UpdateInterval = 30000; // Wait time between reads (in milliseconds)
void presentation()
{
    // Send the sketch version information to the gateway
    sendSketchInfo("TemperatureAndHumidity_2", "3.1");
    _dhtSensor.present();
}

void setup() {
    _dhtSensor.setup();
}

void loop() {  
    unsigned long currentMillis = millis();
    if (currentMillis >= _nextUpdateMillis) {
        _nextUpdateMillis = currentMillis + UpdateInterval;
        
        if (_dhtSensor.report()) {
            digitalWrite(LED_PIN, HIGH);
        } else {
            digitalWrite(LED_PIN, LOW);
        }
    }
}
