#define USE_OCTOWS2811

// 3rd Party Libs
#include<OctoWS2811.h>
#include <FastLED.h>
#include <EEPROM.h>
#include <Bounce2.h>

// Our Libs
#include "menu.h"

// Notes
// Added /Applications/Arduino.app/Contents/Java/hardware/teensy/avr/libraries/EEPROM/EEPROM.h:147 to use 'e' variable to suppress warning
// Commented out /Users/nickbarone/Documents/Arduino/libraries/FastLED/FastLED.h:17 to suppress message
// Added line 34 to /Applications/Arduino.app/Contents/Java/hardware/teensy/avr/libraries/Bounce2/Bounce2.cpp to supress warning

// Basic strip settings

#define NUM_LEDS_PER_STRIP  300
#define NUM_STRIPS          8
#define OFFSET              0 //The current wiring has us starting in the middle
#define CAP                 360
#define NUM_LEDS NUM_LEDS_PER_STRIP * NUM_STRIPS

// Button settings

#define BRIGHTNESS_UP_PIN   0
#define BRIGHTNESS_DOWN_PIN 23
#define MODE_UP_PIN         19
#define MODE_DOWN_PIN       17
#define BOUNCE_INTERVAL     5 //ms

// Menu Settings
#define NUM_MODES           6

// Global State

CRGB      _LEDS[NUM_LEDS];
int       _BRIGHTNESS =     32;
int       _MODE =           0;
uint16_t  _START_TIME;

// Buttons!
Bounce _MODE_UP         = Bounce();
Bounce _MODE_DOWN       = Bounce();

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

// MASTERAVERBAITER
class Context {
  public:
  CRGB c;
  uint16_t i;
  uint32_t t;
};

class Effect {
  public:
    virtual void Run(Context *c){
    }
};

class Composition : public Effect {
  public:
  Effect ** effects;
  Composition(Effect ** e) {
    effects = e;
  }

  void Run(Context *c) {
    Context _c;
    _c.c = c->c;
    _c.i = c->i;
    _c.t = c->t;

    for(uint8_t e = 0; effects[e] != NULL; e++) {
      effects[e]->Run(&_c);
    }

    c->c = _c.c;
  }
};

class ColorWipe : public Effect {
  public:
  CRGB color;
  ColorWipe(CRGB c) {
    color = c;
  }
  
  void Run(Context *c){
    c->c = color;
  }
};

class Running : public Effect {
  public:
  float pixels = 1;
  Running(float p) {
    pixels = p;
  }

  void Run(Context *c) {
    //Note: Adding makes it run "backwards". Subtraction "forwards"
    float fps = 5; //Microseconds per frame (so this is 60fps?) (and it seems that I get hundrendths of a second, not milliseconds)
    float _t = c->t;
    int16_t _i = c->i;
    
    _i = _i - pixels * _t / fps; // Param1 is lights-per-frame at 60FPS // Also, note that subtracting a positive makes it appear to move forwards, because math.
  
    //Loop
    if (_i < 0) {
      _i = CAP + _i % CAP; //Remember, _i is negative here
    } else if (_i > CAP) {
      _i = _i % CAP;
    }
  
    c->i = _i;
  }
};

class Wheel : public Effect {
  public:

  void Run(Context *c) {
    uint8_t hue = ((float) c->i / NUM_LEDS) * 255;
    c->c = CHSV(hue, 255, 255);
  }
};

class Pulse : public Effect {
  public:
  uint16_t pos;
  uint16_t inner;
  uint16_t width;
  CRGB pos_color;
  CRGB inner_color;
  Pulse(uint16_t p, uint16_t i, uint16_t w, CRGB pc, CRGB ic) {
    pos = p;
    inner = i;
    width = w;
    pos_color = pc;
    inner_color = ic;
  }

  void Run(Context *c) {
    //TODO: Doesn't wrap!
    if (c->i == pos) {
      c->c = pos_color;
    } else if ((pos - inner) <= c->i && c->i <= (pos + inner)) {
      c->c = inner_color;
    } else if ((pos - width) <= c->i && c->i <= (pos + width)) {
      //In the pulse
      fract8 percent = 255 - 255 * abs(pos - c->i) / width;
      c->c = blend(c->c, inner_color, percent);
    }
  }
};

// MODES
Context c;

Effect * compWheel[] = {
  new Running(1),
  new Wheel(),
  NULL,
};

Effect * comp0[] = {
  new Running(1),
  new Pulse(15, 2, 6, CRGB::Yellow, CRGB::Green),
  NULL
};

Effect * comp1[] = {
  new Running(1.5),
  new Pulse(15, 2, 6, CRGB::Black, CRGB::White),
  NULL
};

Effect * comp2[] = {
  //new RandomRunning(1,1),
  new Running(-1),
  new Pulse(15, 8, 10, CRGB::White, CRGB::Black),
  NULL
};

Effect * effects[] = {
  new ColorWipe(CRGB::Black),
  new Composition(compWheel),
  new Composition(comp0),
  new Composition(comp1),
  new Composition(comp2),
  NULL
};

void MasterRaverBaiter() {
  uint16_t _t = (millis() - _START_TIME);

  for (int _i = 0; _i < NUM_LEDS; _i++) {

    c.i = _i;
    c.t = _t/10;
    c.c = _LEDS[_i];
    
    
    for (uint8_t e = 0; effects[e] != NULL; e++) {
      effects[e]->Run(&c);
    }

    _LEDS[_i] = c.c;
  }
}


#define COOLING  55
#define SPARKING 120
bool gReverseDirection = false;
void Fire2012()
{
// Array of temperature readings at each simulation cell
  static byte heat[NUM_LEDS];

  // Step 1.  Cool down every cell a little
    for( int i = 0; i < NUM_LEDS; i++) {
      heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
    }
  
    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for( int k= NUM_LEDS - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }
    
    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if( random8() < SPARKING ) {
      int y = random8(7);
      heat[y] = qadd8( heat[y], random8(160,255) );
    }

    // Step 4.  Map from heat cells to LED colors
    for( int j = 0; j < NUM_LEDS; j++) {
      CRGB color = HeatColor( heat[j]);
      int pixelnumber;
      if( gReverseDirection ) {
        pixelnumber = (NUM_LEDS-1) - j;
      } else {
        pixelnumber = j;
      }
      _LEDS[pixelnumber] = color;
    }
}

void Rainbow() {
  static uint8_t hue = 0;
  for(int i = 0; i < NUM_STRIPS; i++) {
    for(int j = 0; j < NUM_LEDS_PER_STRIP; j++) {
      _LEDS[(i*NUM_LEDS_PER_STRIP) + j] = CHSV((32*i) + hue+j,192,255);
    }
  }

  // Set the first n leds on each strip to show which strip it is
  for(int i = 0; i < NUM_STRIPS; i++) {
    for(int j = 0; j <= i; j++) {
      _LEDS[(i*NUM_LEDS_PER_STRIP) + j] = CRGB::Red;
    }
  }

  hue++;
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
}
