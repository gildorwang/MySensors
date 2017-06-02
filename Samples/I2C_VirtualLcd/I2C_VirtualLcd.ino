// Source: https://bitbucket.org/fmalpartida/new-liquidcrystal/wiki/Home

#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>


#define I2C_ADDR 0x3F // <<----- Add your address here.  Find it from I2C Scanner
#define BACKLIGHT_PIN 3
#define En_pin 2
#define Rw_pin 1
#define Rs_pin 0
#define D4_pin 4
#define D5_pin 5
#define D6_pin 6
#define D7_pin 7

LiquidCrystal_I2C lcd(I2C_ADDR, En_pin, Rw_pin, Rs_pin, D4_pin, D5_pin, D6_pin, D7_pin);
LCD *myLcd = &lcd;

void setup()
{
    // Switch on the backlight
    //pinMode(BACKLIGHT_PIN, OUTPUT);
    //digitalWrite(BACKLIGHT_PIN, HIGH);
    myLcd->begin(16, 2); // initialize the lcd
    myLcd->setBacklightPin(BACKLIGHT_PIN, POSITIVE);
    myLcd->setBacklight(HIGH);

    myLcd->home(); // go home
    myLcd->print("Hello, ARDUINO ");
    myLcd->setCursor(0, 1); // go to the next line
    myLcd->print(" WORLD 2!  ");
}

void loop()
{
}