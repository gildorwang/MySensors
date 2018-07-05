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
State state;

SevSeg sevseg; // Instantiate a seven segment controller object

ClickEncoder *encoder;
int16_t last, minutes;

void timerIsr() { encoder->service(); }

void setupSevSeg() {
    byte numDigits = 4;
    byte digitPins[] = {2, 3, 4, 5};
    byte segmentPins[] = {6, 7, 8, 9, 10, 11, 12, 13};
    bool resistorsOnSegments = false;     // 'false' means resistors are on digit pins
    byte hardwareConfig = COMMON_CATHODE; // See README.md for options
    bool updateWithDelays = false;        // Default. Recommended
    bool leadingZeros = false;            // Use 'true' if you'd like to keep the leading zeros

    sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments, updateWithDelays, leadingZeros);
    sevseg.setBrightness(10);
}

void setupEncoder() {
    encoder = new ClickEncoder(A1, A0, A2);

    Timer1.initialize(1000);
    Timer1.attachInterrupt(timerIsr);

    last = -1;
}

void setup() {
    Serial.begin(9600);
    setupEncoder();
    setupSevSeg();
    setState(setting);
}

void displayMinutes(int16_t minutes) {
    if (minutes < 120) {
        sevseg.setNumber(minutes);
    } else {
        sevseg.setNumber(minutes / 60 * 100 + (minutes % 60), 2);
    }
    sevseg.refreshDisplay(); // Must run repeatedly
}

void loop() {
    switch (state) {
    case setting:
        processSettingState();
        break;
    case counting:
        break;
    }
}

void processSettingState() {
    minutes += encoder->getValue();
    if (minutes < 0) {
        minutes = 0;
    } else if (minutes > 60 * 100 - 1) {
        minutes = 60 * 100 - 1;
    }
    displayMinutes(minutes);

    if (minutes != last) {
        last = minutes;
        Serial.print("Encoder Value: ");
        Serial.println(minutes);
    }

    ClickEncoder::Button b = encoder->getButton();
    if (b != ClickEncoder::Open) {
        switch (b) {
        case ClickEncoder::Pressed:
            break;
        case ClickEncoder::Held:
            minutes = 0;
            break;
        case ClickEncoder::Released:
            break;
        case ClickEncoder::Clicked:
            break;
        case ClickEncoder::DoubleClicked:
            Serial.println("ClickEncoder::DoubleClicked");
            encoder->setAccelerationEnabled(!encoder->getAccelerationEnabled());
            Serial.print("  Acceleration is ");
            Serial.println((encoder->getAccelerationEnabled()) ? "enabled" : "disabled");
            break;
        }
    }
}

void setState(State newState) {
    Serial.print("State=");
    Serial.println(newState);
    state = newState;
}