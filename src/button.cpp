#include <Arduino.h>
#include <PinChangeInterrupt.h>
#include <FastLED.h>

#include "button.h"
#include "configuration.h"



Button::Button(uint8_t _pin, void (*isrChange)()) : pin(_pin) {
    pinMode(pin, INPUT_PULLUP);
    attachPinChangeInterrupt(digitalPinToPCINT(pin), isrChange, CHANGE);
};

void Button::attach(void(*_click)(void), void(*_longClick)(void)) {
    click = _click;
    longClick = _longClick;
}

void Button::isrChange() {
    if(digitalRead(pin)) {
        isrRaising();
    } else {
        isrFalling();
    }
};

void Button::isrRaising() {
    this->click();
};

void Button::isrFalling() {
    
};








void buttonIntensityChange() {
    buttonIntensity->isrChange();
}

void buttonModeChange(){
    buttonMode->isrChange();
}

extern Button* buttonIntensity = new Button(BUTTON_INTENSITY_PIN, buttonIntensityChange); 
extern Button* buttonMode = new Button(BUTTON_MODE_PIN, buttonModeChange);
