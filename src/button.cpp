#include <Arduino.h>
#include <PinChangeInterrupt.h>

#include "button.h"
#include "configuration.h"



Button::Button(uint8_t _pin, void (*_isrRaising)(), void (*_isrFalling)()) :
pin(_pin), isrRaising(_isrRaising), isrFalling(_isrFalling) {
    pinMode(pin, INPUT_PULLUP);
    attachPinChangeInterrupt(digitalPinToPCINT(pin), isrRaising, RISING);
    attachPinChangeInterrupt(digitalPinToPCINT(pin), isrFalling, FALLING);
};

void Button::attach(void(*_click)(void), void(*_longClick)(void)) {
    click = _click;
    longClick = _longClick;
}

void Button::isrRaisingCallback() {
    this->click();
};

void Button::isrFallingCallback() {
    
};











void buttonIntensityRaising(){
    buttonIntensity->isrRaisingCallback();
}

void buttonIntensityFalling(){
    buttonIntensity->isrFallingCallback();
}

void buttonModeRaising(){
    buttonMode->isrRaisingCallback();
}

void buttonModeFalling(){
    buttonMode->isrFallingCallback();
}

extern Button* buttonIntensity = new Button(BUTTON_INTENSITY_PIN, buttonIntensityRaising, buttonIntensityFalling); 
extern Button* buttonMode = new Button(BUTTON_MODE_PIN, buttonModeRaising, buttonModeFalling); 
