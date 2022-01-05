#include <Arduino.h>
#include <TM1637TinyDisplay.h>
#include "TimerOne.h"
#include "LowPower.h"
#include "animation.h"


// Module connection pins (Digital Pins)
#define CLK 9
#define DIO 8
#define VM A0

#define R1 47000
#define R2 4700

#define THERSHOLD_PIN 13
#define REFERENCE_VOLTAGE 3.35

#define THERSHOLD_TIMEOUT 60 * 60 * 2
#define MIN_SWITCH_DELAY 60 * 10

#define LED_ON_PIN 6
#define LED_OFF_PIN 7

#define POT_LOW_PIN A1
#define POT_HIGH_PIN A2
#define LOW_RANGE 10
#define HIGH_RANGE 16

#define MEDIAN_SIZE_VOL 100
#define MEDIAN_SIZE_POT 10

#define DISPLAY_TIME_CHANGE 2*1000

volatile unsigned long  countdown = 0;
volatile unsigned long  minSwitchTime = 0;


TM1637TinyDisplay display(CLK, DIO);


int sort_desc(const void *cmp1, const void *cmp2)
{
  int a = *((int *)cmp1);
  int b = *((int *)cmp2);
  return b - a;
}

int median(int value, int* index, int queue[], uint8_t size)
{
  if (*index < 0) {
    for(int i = 0; i < size; i++){
      queue[i] = value;
    }
    (*index)++;
  }


  queue[*index] = value;

  if (++(*index) >= size) {
    *index = 0;
  }

  int lt[size];
  for(int i = 0; i < size; i++){
    lt[i] = queue[i];
  }

  int lt_length = sizeof(lt) / sizeof(lt[0]);
  qsort(lt, lt_length, sizeof(lt[0]), sort_desc);
  return lt[size / 2];
}

float convertToVoltageAndRound(int sensor) {
  float steps = (HIGH_RANGE - LOW_RANGE) / 1023.0;
  float voltage = sensor * steps + LOW_RANGE;
  return ((float)((int)(voltage * 10))) / 10;
}

float readPotMeterLow() {
  static int index = -1;
  static int queue[MEDIAN_SIZE_POT];
  int sensor = median(analogRead(POT_LOW_PIN), &index, queue, MEDIAN_SIZE_POT);
  return convertToVoltageAndRound(sensor);
}

float readPotMeterHigh() {
  static int index = -1;
  static int queue[MEDIAN_SIZE_POT];
  int sensor = median(analogRead(POT_HIGH_PIN), &index, queue, MEDIAN_SIZE_POT);
  return convertToVoltageAndRound(sensor);
}

void resetCountdown()
{
  noInterrupts();
  countdown = THERSHOLD_TIMEOUT;
  interrupts();
}

void relayOn()
{
  digitalWrite(THERSHOLD_PIN, HIGH);
  digitalWrite(LED_ON_PIN, HIGH);
  digitalWrite(LED_OFF_PIN, LOW);
  resetCountdown();
}

void relayOff()
{
  digitalWrite(THERSHOLD_PIN, LOW);
  digitalWrite(LED_ON_PIN, LOW);
  digitalWrite(LED_OFF_PIN, HIGH);
}

void decrement(volatile unsigned long *value) {
  if (*value > 0) {
    (*value)--;
  }
}

void secondTicker()
{
  decrement(&countdown);
  decrement(&minSwitchTime);

  if (countdown <= 0) {
    relayOff();
  }
}

void setup()
{
  display.clear();
  display.setBrightness(BRIGHT_1);

  Timer1.initialize();
  Timer1.attachInterrupt(secondTicker); // count every second

  pinMode(THERSHOLD_PIN, OUTPUT);
  pinMode(LED_ON_PIN, OUTPUT);
  pinMode(LED_OFF_PIN, OUTPUT);

  relayOff();

  display.showAnimation(ANIMATION, FRAMES(ANIMATION), TIME_MS(50));


  float potMeterLow;
  float potMeterHigh;
  for(int i = 0; i < MEDIAN_SIZE_POT; i++){
    potMeterLow = readPotMeterLow();
    potMeterHigh = readPotMeterHigh();
  }
  
  uint8_t arrL[] = { 0x38 };
  display.setSegments(arrL, 1);
  display.showNumber(potMeterLow/10, 3, 3, 1);
  delay(DISPLAY_TIME_CHANGE);
 
  uint8_t arrH[] = { 0x76 };
  display.setSegments(arrH, 1);
  display.showNumber(potMeterHigh/10, 3, 3, 1);
  delay(DISPLAY_TIME_CHANGE);
 
}

void loop()
{
  static int index = -1;
  static int queue[MEDIAN_SIZE_VOL];
 
  int sensor = median(analogRead(VM), &index, queue, MEDIAN_SIZE_VOL);
  float vOut = sensor * (REFERENCE_VOLTAGE / 1023.0);
  float vIn = vOut * ((R1 + R2) / R2); 


  static unsigned long displayLowMilis;
  static unsigned long displayHighMilis;

  float potMeterLow = readPotMeterLow();
  float potMeterHigh = readPotMeterHigh();

  static float lastPotMeterLow;
  static float lastPotMeterHigh;


  if (potMeterLow != lastPotMeterLow) {
    displayLowMilis = millis();
  }

  if (potMeterHigh != lastPotMeterHigh) {
    displayHighMilis = millis();
  }

  lastPotMeterLow = potMeterLow;
  lastPotMeterHigh = potMeterHigh;

  potMeterLow = min(potMeterLow, potMeterHigh);

  if (displayLowMilis + DISPLAY_TIME_CHANGE > millis() || displayHighMilis + DISPLAY_TIME_CHANGE > millis()) {
    if (displayHighMilis > displayLowMilis) {
      uint8_t arr[] = { 0x76 };
      display.setSegments(arr, 1);
      display.showNumber(potMeterHigh/10, 3, 3, 1);
    } else {
      uint8_t arr[] = { 0x38 };
      display.setSegments(arr, 1);
      display.showNumber(potMeterLow/10, 3, 3, 1);
    }
  } else {
    display.showNumber(vIn);
  }

  if (vIn >= potMeterHigh && minSwitchTime <= 0) {
    relayOn();
    minSwitchTime = MIN_SWITCH_DELAY;
  }
 
  if (vIn <= potMeterLow && minSwitchTime <= 0) {
    relayOff();
    minSwitchTime = MIN_SWITCH_DELAY;
  }

  LowPower.idle(SLEEP_60MS, ADC_OFF, TIMER2_OFF, TIMER1_ON, TIMER0_ON, 
                SPI_OFF, USART0_OFF, TWI_OFF);

}
