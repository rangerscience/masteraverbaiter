#define USE_OCTOWS2811

// 3rd Party Libs
#include<OctoWS2811.h>
#include <FastLED.h>
#include <EEPROM.h>
#include <Bounce2.h>

// Notes
// Added /Applications/Arduino.app/Contents/Java/hardware/teensy/avr/libraries/EEPROM/EEPROM.h:147 to use 'e' variable to suppress warning
// Commented out /Users/nickbarone/Documents/Arduino/libraries/FastLED/FastLED.h:17 to suppress message
// Added line 34 to /Applications/Arduino.app/Contents/Java/hardware/teensy/avr/libraries/Bounce2/Bounce2.cpp to supress warning

// Basic strip settings

#define NUM_LEDS_PER_STRIP  30
#define NUM_STRIPS          1
#define OFFSET              0 //The current wiring has us starting in the middle
#define CAP                 30
#define NUM_LEDS NUM_LEDS_PER_STRIP * NUM_STRIPS
#define FRAMES_PER_SECOND   120

// Button settings

#define BRIGHTNESS_UP_PIN   0
#define BRIGHTNESS_DOWN_PIN 23
#define MODE_UP_PIN         19
#define MODE_DOWN_PIN       17
#define BOUNCE_INTERVAL     5 //ms

// Menu Settings
#define NUM_MODES           6

// Global State

CRGB      _LEDS[NUM_LEDS]   ;
int       _BRIGHTNESS =     32;
int       _MODE =           4;
uint16_t  _START_TIME       ;

// Buttons!
Bounce _MODE_UP         = Bounce();
Bounce _MODE_DOWN       = Bounce();

// Our Libs
// (here because right now they reference globals and defines)
#include "menu.h"
#include "masteraverbaiter.h"
#include "prebuilt.h"

void setup() {
  delay(3000); // sanity delay
  FastLED.addLeds<OCTOWS2811>(_LEDS, NUM_LEDS_PER_STRIP);
  FastLED.setBrightness( _BRIGHTNESS );

  pinMode(0, INPUT_PULLUP);
  pinMode(23, INPUT_PULLUP);
  pinMode(19, INPUT_PULLUP);
  pinMode(17, INPUT_PULLUP);

  _MODE_UP.attach(MODE_UP_PIN);
  _MODE_UP.interval(BOUNCE_INTERVAL);

  _MODE_DOWN.attach(MODE_DOWN_PIN);
  _MODE_DOWN.interval(BOUNCE_INTERVAL);

  int __BRIGHTNESS = 0;
  EEPROM.get(0, __BRIGHTNESS);
  if(__BRIGHTNESS != 0) {
    _BRIGHTNESS = __BRIGHTNESS;
  }

  int __MODE = 0;
  EEPROM.get(2, __MODE);
  if(__MODE != 0) {
    _MODE = __MODE;
  }

  _START_TIME = millis();
}

void loop() {
  
  if(dimmer(BRIGHTNESS_UP_PIN, BRIGHTNESS_DOWN_PIN, _BRIGHTNESS)) {
    EEPROM.put(0, _BRIGHTNESS);
    FastLED.setBrightness( _BRIGHTNESS );
  }

  if(modeSelect(_MODE_DOWN, _MODE_UP, _MODE, NUM_MODES)) {
    EEPROM.put(2, _MODE);
  }
  
  if(_MODE == 0) {
    MasterRaverBaiter();
  }
  
  if(_MODE == 1) {
    Rainbow();
  }
  if(_MODE == 2) {
    for( int i = 0; i < NUM_LEDS; i++) {
      _LEDS[i] = CRGB::Red;
    }
  }
  if(_MODE == 3) {
    for( int i = 0; i < NUM_LEDS; i++) {
      _LEDS[i] = CRGB::Blue;
    }
  }
  if(_MODE == 4) {
    for( int i = 0; i < NUM_LEDS; i++) {
      _LEDS[i] = CRGB::Green;
    }
  }
  if(_MODE == 5) {
    Fire2012();
  }

  FastLED.show(); // display this frame

  //Dithering
  FastLED.delay(1000 / FRAMES_PER_SECOND);
  FastLED.setBrightness(0);
  FastLED.delay(1000 / (FRAMES_PER_SECOND * 6));
  FastLED.setBrightness(_BRIGHTNESS);
  
}
