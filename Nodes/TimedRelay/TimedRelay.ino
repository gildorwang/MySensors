// For rotary encoder with button
#include <ClickEncoder.h>
#include <TimerOne.h>
// For 7-segment LCD display
#include <SevSeg.h>
#include <TimerFreeTone.h>

#define SPEAKER_PIN 18
#define RELAY_PIN 17
#define RELAY_ON LOW
#define RELAY_OFF HIGH

// State of the system
enum State {
    SETTING,
    COUNTING
};
State _state;

SevSeg _sevseg; // Instantiate a seven segment controller object

ClickEncoder *_encoder;
int16_t _lastMinutes, _minutes;

bool _relayState = true;

unsigned long _minutesLastUpdatedByUserMillis = 0;
unsigned long _lastMinuteMills = 0;

void setup() {
    Serial.begin(9600);
    setupEncoder();
    setupSevSeg();
    pinMode(RELAY_PIN, OUTPUT);
    setRelay();
    setState(SETTING);
}

void loop() {
    switch (_state) {
    case SETTING:
        processSettingState();
        break;
    case COUNTING:
        processCountingState();
        break;
    }
}

void timerIsr() {
    const short BlinkPeriodInMs = 2000;
    const short DelayBeforeBlinkInMs = 1000;

    unsigned long now = millis();
    if (_state == SETTING && now % BlinkPeriodInMs < (BlinkPeriodInMs / 2) && now - _minutesLastUpdatedByUserMillis > DelayBeforeBlinkInMs) {
        _sevseg.blank();
    } else {
        displayMinutes();
    }
    // Run the encoder service. It needs to be run in the timer
    _encoder->service();
}

void setupSevSeg() {
    byte numDigits = 4;
    byte digitPins[] = {2, 3, 4, 5};
    byte segmentPins[] = {6, 7, 8, 9, 10, 11, 12, 13};
    bool resistorsOnSegments = false;     // 'false' means resistors are on digit pins
    byte hardwareConfig = COMMON_CATHODE; // See README.md for options
    bool updateWithDelays = false;        // Default. Recommended
    bool leadingZeros = false;            // Use 'true' if you'd like to keep the leading zeros

    _sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments, updateWithDelays, leadingZeros);
    _sevseg.setBrightness(10);
}

void setupEncoder() {
    _encoder = new ClickEncoder(A1, A0, A2);
    Timer1.initialize(1000);
    Timer1.attachInterrupt(timerIsr);
    beep(100);
    delay(100);
    beep(100);

    _lastMinutes = -1;
}

void displayMinutes() {
    if (_minutes < 120) {
        _sevseg.setNumber(_minutes);
    } else {
        _sevseg.setNumber(_minutes / 60 * 100 + (_minutes % 60), 2);
    }
    _sevseg.refreshDisplay(); // Must run repeatedly
}

void processCountingState() {
    const unsigned long MinuteInMs = 60 * 1000L;
    unsigned long now = millis();
    if (now - _lastMinuteMills >= MinuteInMs) {
        _minutes--;
        _lastMinuteMills = now;
    }
    if (_minutes <= 0) {
        // Time's out
        _minutes = 0;
        flipRelay();
        beep(100);
        delay(100);
        beep(100);
        setState(SETTING);
        return;
    }

    ClickEncoder::Button b = _encoder->getButton();
    switch (b) {
    case ClickEncoder::Clicked:
        setState(SETTING);
        break;
    case ClickEncoder::DoubleClicked:
        flipRelay();
        break;
    }
}

void processSettingState() {
    _minutes += _encoder->getValue();
    if (_minutes < 0) {
        _minutes = 0;
    } else if (_minutes > 60 * 100 - 1) {
        _minutes = 60 * 100 - 1;
    }
    if (_minutes != _lastMinutes) {
        _lastMinutes = _minutes;
        _minutesLastUpdatedByUserMillis = millis();
    }

    ClickEncoder::Button b = _encoder->getButton();
    if (b != ClickEncoder::Open) {
        switch (b) {
        case ClickEncoder::Held:
            _minutes = 0;
            break;
        case ClickEncoder::Clicked:
            setState(COUNTING);
            break;
        case ClickEncoder::DoubleClicked:
            flipRelay();
            break;
        }
    }
}

void setState(State newState) {
    Serial.print("State=");
    Serial.println(newState);
    _state = newState;
    switch (newState) {
    case SETTING:
        break;
    case COUNTING:
        _lastMinuteMills = millis();
        break;
    }
}

void flipRelay() {
    _relayState = !_relayState;
    Serial.print("Turning relay ");
    Serial.println(_relayState ? "ON" : "OFF");
    setRelay();
}

void setRelay() {
    digitalWrite(RELAY_PIN, _relayState ? RELAY_ON : RELAY_OFF);
}

void beep(unsigned long length) {
    TimerFreeTone(SPEAKER_PIN, 4500, length);
}
