// For rotary encoder with button
#include <ClickEncoder.h>
#include <TimerOne.h>
// For 7-segment LCD display
#include "SevSeg.h"

// State of the system
enum State {
    setting,
    counting
};
State _state;

SevSeg _sevseg; // Instantiate a seven segment controller object

ClickEncoder *_encoder;
int16_t _lastMinutes, _minutes;

unsigned long _minutesLastUpdatedByUserMillis = 0;

void timerIsr() {
    const short BlinkPeriodInMs = 2000;
    const short DelayBeforeBlinkInMs = 1000;

    unsigned long now = millis();
    if (now % BlinkPeriodInMs < (BlinkPeriodInMs / 2) && now - _minutesLastUpdatedByUserMillis > DelayBeforeBlinkInMs) {
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

    _lastMinutes = -1;
}

void setup() {
    Serial.begin(9600);
    setupEncoder();
    setupSevSeg();
    setState(setting);
}

void displayMinutes() {
    if (_minutes < 120) {
        _sevseg.setNumber(_minutes);
    } else {
        _sevseg.setNumber(_minutes / 60 * 100 + (_minutes % 60), 2);
    }
    _sevseg.refreshDisplay(); // Must run repeatedly
}

void loop() {
    switch (_state) {
    case setting:
        processSettingState();
        break;
    case counting:
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
        case ClickEncoder::Pressed:
            break;
        case ClickEncoder::Held:
            _minutes = 0;
            break;
        case ClickEncoder::Released:
            break;
        case ClickEncoder::Clicked:
            break;
        case ClickEncoder::DoubleClicked:
            Serial.println("ClickEncoder::DoubleClicked");
            _encoder->setAccelerationEnabled(!_encoder->getAccelerationEnabled());
            Serial.print("  Acceleration is ");
            Serial.println((_encoder->getAccelerationEnabled()) ? "enabled" : "disabled");
            break;
        }
    }
}

void setState(State newState) {
    Serial.print("State=");
    Serial.println(newState);
    _state = newState;
}