#include <Arduino.h>          // Arduino core
#include <FastLED.h>
#include <PinChangeInterrupt.h>
#include <LowPower.h>

#include "configuration.h"
#include "button.h"



const uint16_t mask_front_light = 0b0000000000111;
const uint16_t mask_side_light =  0b0000111110000;
const uint16_t mask_back_light =  0b1110000000000;


CRGB leds[NUM_LEDS];

int selectedMode = 0;
int intensity = 0;

bool lock = false;


void applyColor(uint16_t mask, const struct CRGB& color, uint8_t lamp) {
     for (int i = 0; i < (NUM_LEDS / NUM_LAMP); i++) {
        int b = mask >> i;
        if (b & 1) {
            leds[i + ((NUM_LEDS / NUM_LAMP) * lamp)] = color;
        }
    }
}

void full_mode() {
  fill_solid(leds, NUM_LEDS, CRGB::White);
}

void front_mode() {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  applyColor(mask_front_light, CRGB::White, 0);
  applyColor(mask_front_light, CRGB::White, 1);
}

void blink_mode() {
  uint8_t beat = (millis() % 1000) / 100;
  if (beat == 1 || beat == 3) {
    fill_solid(leds, NUM_LEDS, CRGB::White);
  } else {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
  }
}

void sinus_mode() {
  const uint8_t f = 65536 / (4 * 1000);
  uint16_t pos = sin16( ((millis() * f) % 65536) ) + 32768;
  uint8_t color = 255 * (pos / 65536.0f);
  fill_solid(leds, NUM_LEDS, CRGB( color, color, color));
}

void boat_mode() {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  applyColor(mask_front_light, CRGB::White, 0);
  applyColor(mask_front_light, CRGB::White, 1);
  applyColor(mask_side_light, CRGB::Green, 0);
  applyColor(mask_side_light, CRGB::Red, 1);
  applyColor(mask_back_light, CRGB::White, 0);
  applyColor(mask_back_light, CRGB::White, 1);
}

void rainbow_mode() {
  static uint16_t sPseudotime = 0;
  static uint16_t sLastMillis = 0;
  static uint16_t sHue16 = 0;
 
  uint8_t sat8 = beatsin88( 87, 220, 250);
  uint8_t brightdepth = beatsin88( 341, 96, 224);
  uint16_t brightnessthetainc16 = beatsin88( 203, (25 * 256), (40 * 256));
  uint8_t msmultiplier = beatsin88(147, 23, 60);

  uint16_t hue16 = sHue16;//gHue * 256;
  uint16_t hueinc16 = beatsin88(113, 1, 3000);
  
  uint16_t ms = millis();
  uint16_t deltams = ms - sLastMillis ;
  sLastMillis  = ms;
  sPseudotime += deltams * msmultiplier;
  sHue16 += deltams * beatsin88( 400, 5,9);
  uint16_t brightnesstheta16 = sPseudotime;
  
  for( uint16_t i = 0 ; i < NUM_LEDS; i++) {
    hue16 += hueinc16;
    uint8_t hue8 = hue16 / 256;

    brightnesstheta16  += brightnessthetainc16;
    uint16_t b16 = sin16( brightnesstheta16  ) + 32768;

    uint16_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
    uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
    bri8 += (255 - brightdepth);
    
    CRGB newcolor = CHSV( hue8, sat8, bri8);
    
    uint16_t pixelnumber = i;
    pixelnumber = (NUM_LEDS-1) - pixelnumber;
    
    nblend( leds[pixelnumber], newcolor, 64);
  }
}

void display_off() {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
}

void display_starting() {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  applyColor(mask_side_light, CRGB::Yellow, 0);
  applyColor(mask_side_light, CRGB::Yellow, 1);
}

void display_lock() {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  bool beat = (millis() / 50) % 2;
  if(beat) {
    return;
  }
  CRGB color;
  if (lock) {
    color = CRGB::Red;
  }  else {
    color = CRGB::Green;
  }
  applyColor(mask_side_light, color, 0);
  applyColor(mask_side_light, color, 1);
}

void (*funcs[])(void) = {full_mode, front_mode, blink_mode, sinus_mode, boat_mode, rainbow_mode};
uint8_t funcs_size = sizeof(funcs)/sizeof(funcs[0]);

void intensityClick() {
  if (lock) {
    return;
  }
  intensity--;
  if (intensity < 0) {
    intensity = FADE_MAX_STEPS;
  }
}

void modeClick() {
  if (lock) {
    return;
  }
  if (intensity == 0) {
    return;
  }
  selectedMode++;
  if (!(selectedMode < funcs_size)) {
    selectedMode = 0;
  }
}

void longClick() {
  if (buttonIntensity->longClickPending() && buttonMode->longClickPending()) {
    lock = !lock;
  }
}

void setup() {
    buttonIntensity->attach(intensityClick, longClick);
    buttonMode->attach(modeClick, longClick);
   	delay(2000);
    FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, NUM_LEDS);

    display_starting();
    FastLED.show();
    delay(100);
    display_off();
    FastLED.show();
}


void loop() {
  static const uint8_t step_map[FADE_MAX_STEPS + 1] = {0, 30, 100, 255}; 
  
  if (buttonIntensity->longClickPending() && buttonMode->longClickPending()) {
    LEDS.setBrightness(255);
    display_lock();
    FastLED.show();
  } else {
    if (intensity == 0 && !buttonIntensity->isPressed() && !buttonMode->isPressed()) {
      display_off();
      FastLED.show();
      LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
    }

    funcs[selectedMode]();

    LEDS.setBrightness(step_map[intensity]);
    FastLED.show();
  }
}
