#define MY_NODE_ID 31

// Enabled repeater feature for this node
#define MY_REPEATER_FEATURE

#include <MySensorsCustomConfig.h>
#include <SPI.h>
#include <MySensors.h>  
#include <DHT.h>

// Set this to the pin you connected the DHT's data pin to
#define DHT_DATA_PIN 3

// Set this offset if the sensor has a permanent small offset to the real temperatures
#define SENSOR_TEMP_OFFSET 0

// Sleep time between sensor updates (in milliseconds)
// Must be >1000ms for DHT22 and >2000ms for DHT11
static const uint64_t UPDATE_INTERVAL = 5000;

#define CHILD_ID_LED 0
#define CHILD_ID_HUM 1
#define CHILD_ID_TEMP 2

#define LED_PIN 4
#define RELAY_ON 1  // GPIO value to write to turn on attached relay
#define RELAY_OFF 0 // GPIO value to write to turn off attached relay

float lastTemp;
float lastHum;
bool metric = false;
unsigned long nextUpdateMillis;
unsigned long ledOffMillis;

MyMessage msgHum(CHILD_ID_HUM, V_HUM);
MyMessage msgTemp(CHILD_ID_TEMP, V_TEMP);
DHT dht;

MyMessage msgState1(CHILD_ID_LED, V_LIGHT);

void before() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, loadState(CHILD_ID_LED) ? RELAY_ON : RELAY_OFF);
}

void presentation()  
{ 
  // Send the sketch version information to the gateway
  sendSketchInfo("Torch Light", "3.0");
  
  // Register all sensors to gw (they will be created as child devices)
  present(CHILD_ID_LED, S_LIGHT);
  present(CHILD_ID_HUM, S_HUM);
  present(CHILD_ID_TEMP, S_TEMP);
  
  metric = getControllerConfig().isMetric;
}


void setup()
{ 
  dht.setup(DHT_DATA_PIN); // set data pin of DHT sensor
  if (UPDATE_INTERVAL <= dht.getMinimumSamplingPeriod()) {
    Serial.println("Warning: UPDATE_INTERVAL is smaller than supported by the sensor!");
  }

  nextUpdateMillis = millis() + UPDATE_INTERVAL;
}


void loop()      
{  
  unsigned long currentMillis = millis();
  if (currentMillis >= nextUpdateMillis) {
    nextUpdateMillis = currentMillis + UPDATE_INTERVAL;
    reportRelayState();
    updateHumidityAndTemp();
  }
}

void receive(const MyMessage &message) {
  if (message.type == V_LIGHT && message.sensor == CHILD_ID_LED) {
    digitalWrite(LED_PIN, message.getBool() ? RELAY_ON : RELAY_OFF);
    saveState(message.sensor, message.getBool());
    // Write some debug info
    Serial.print("Incoming change for sensor:");
    Serial.print(message.sensor);
    Serial.print(", New status: ");
    Serial.println(message.getBool());
  }
}

void reportRelayState() {
  send(msgState1.set(loadState(CHILD_ID_LED)));
}

void updateHumidityAndTemp() {
    // Force reading sensor, so it works also after sleep()
    dht.readSensor(true);
    // Get temperature from DHT library
    float temperature = dht.getTemperature();
    if (isnan(temperature)) {
      Serial.println("Failed reading temperature from DHT!");
    } else {
      lastTemp = temperature;
      if (!metric) {
        temperature = dht.toFahrenheit(temperature);
      }
      temperature += SENSOR_TEMP_OFFSET;
      send(msgTemp.set(temperature, 1));
  
      #ifdef MY_DEBUG
      Serial.print("T: ");
      Serial.println(temperature);
      #endif
    }
  
    // Get humidity from DHT library
    float humidity = dht.getHumidity();
    if (isnan(humidity)) {
      Serial.println("Failed reading humidity from DHT");
    } else {
      lastHum = humidity;
      send(msgHum.set(humidity, 1));
      
      #ifdef MY_DEBUG
      Serial.print("H: ");
      Serial.println(humidity);
      #endif
    }
}

