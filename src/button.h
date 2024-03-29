#include <Arduino.h>

#ifndef Button_C
#define Button_C


class Button {
    private:
        uint8_t pin;
        unsigned long last_raising;
        void (*click)(void);
        void (*longClick)(void);
        void isrRaising();
        void isrFalling();
        void isrChange();
        bool _isPressed = false;
        unsigned long pressedTime = 0;
    public:
        Button(uint8_t pin, void (*isrChange)());
        bool longClickPending();
        bool isPressed();
        void Button::attach(void(*click)(void), void(*longClick)(void));
        friend void buttonIntensityChange();
        friend void buttonModeChange();
};

extern Button* buttonIntensity; 
extern Button* buttonMode;
#endif
