#include <Arduino.h>

#ifndef Button_C
#define Button_C


class Button {
    private:
        uint8_t pin;
        unsigned long last_raising;
        void (*click)(void);
        void (*longClick)(void);
        void (*isrRaising)();
        void (*isrFalling)();
        void isrRaisingCallback();
        void isrFallingCallback();
    public:
        Button(uint8_t pin, void (*isrRaising)(), void (*isrFalling)());
        bool longClickPending();
        void Button::attach(void(*click)(void), void(*longClick)(void));
        friend void buttonIntensityRaising();
        friend void buttonIntensityFalling();
        friend void buttonModeRaising();
        friend void buttonModeFalling();
};

extern Button* buttonIntensity; 
extern Button* buttonMode;
#endif
