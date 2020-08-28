/**
 * This project turns on the water pump intermittently (7 min on, then 23 min off, and so on).
 */

const short _outPin = 3;

bool _currentState = true;
unsigned long _nextToggleMillis = 0;

unsigned long _offMillis = 23 * 60 * 1000L;
unsigned long _onMillis = 7 * 60 * 1000L;

void setup() {
    pinMode(_outPin, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(_outPin, !_currentState);
    digitalWrite(LED_BUILTIN, _currentState);
    _nextToggleMillis = millis() + _onMillis;
}

void loop() {
    unsigned long now = millis();
    if (now > _nextToggleMillis) {
        _currentState = !_currentState;
        // it's a low-trigger relay
        digitalWrite(_outPin, !_currentState);
        digitalWrite(LED_BUILTIN, _currentState);
        _nextToggleMillis += _currentState ? _onMillis : _offMillis;
    }
}
