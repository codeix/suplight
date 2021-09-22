#include <Arduino.h>          // Arduino core
#include <FastLED.h>
#include <PinChangeInterrupt.h>
#include <LowPower.h>

#define NUM_LAMP 2
#define NUM_LEDS 13 * NUM_LAMP
#define DATA_PIN 10

#define BUTTON_INTENSITY_PIN A1
#define BUTTON_MODE_PIN A3
#define FADE_MAX_STEPS 3

const uint16_t mask_front_light = 0b0000000000111;
const uint16_t mask_side_light =  0b0000111110000;
const uint16_t mask_back_light =  0b1110000000000;


CRGB leds[NUM_LEDS];

int selectedMode = 0;
int intensity = 0;


void applyColor(uint16_t mask, const struct CRGB& color, uint8_t lamp) {
     for (int i = 0; i < (NUM_LEDS / NUM_LAMP); i++) {
        int b = mask >> i;
        if (b & 1) {
            leds[i + ((NUM_LEDS / NUM_LAMP) * lamp)] = color;
        }
    }
}

void off_mode() {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
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
  static uint16_t sLastMillis = 0;
  unsigned long ms = millis();
  unsigned long deltams = ms - sLastMillis ;
  uint8_t beat = (deltams % 1000) / 100;
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
  applyColor(mask_side_light, CRGB::Red, 0);
  applyColor(mask_side_light, CRGB::Green, 0);
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

void (*funcs[])(void) = {full_mode, front_mode, blink_mode, sinus_mode, boat_mode, rainbow_mode};
uint8_t funcs_size = sizeof(funcs)/sizeof(funcs[0]);

void intensityChange() {
  intensity--;
  if (intensity < 0) {
    intensity = FADE_MAX_STEPS;
  }
}

void modeChange() {
  if (intensity == 0) {
    return;
  }
  selectedMode++;
  if (!(selectedMode < funcs_size)) {
    selectedMode = 0;
  }
}


void setup() {
   	delay(2000);
    FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, NUM_LEDS);


    pinMode(BUTTON_INTENSITY_PIN, INPUT_PULLUP);
    attachPinChangeInterrupt(digitalPinToPCINT(BUTTON_INTENSITY_PIN), intensityChange, RISING);

    pinMode(BUTTON_MODE_PIN, INPUT_PULLUP);
    attachPinChangeInterrupt(digitalPinToPCINT(BUTTON_MODE_PIN), modeChange, RISING);

    boat_mode();
    FastLED.show();
    delay(100);
    off_mode();
    FastLED.show();
}


void loop() {
  const uint8_t step_map[FADE_MAX_STEPS + 1] = {0, 30, 100, 255}; 
  if (intensity == 0) {
    off_mode();
    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
  }
  funcs[selectedMode]();
  LEDS.setBrightness(step_map[intensity]);
  FastLED.show();
}
