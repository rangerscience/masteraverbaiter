#define USE_OCTOWS2811
#include<OctoWS2811.h>
#include <FastLED.h>

// 0
// 23
// 19
// 17

#define NUM_LEDS_PER_STRIP 300
#define NUM_STRIPS 8
#define OFFSET 0 //The current wiring has us starting in the middle
#define CAP 360
#define NUM_LEDS NUM_LEDS_PER_STRIP * NUM_STRIPS

CRGB leds[NUM_LEDS];

int BRIGHTNESS = 32;
uint32_t NUM_MODES = 6;
uint8_t MODE = 0; // Master
uint16_t  start_time;

void setup() {
  delay(3000); // sanity delay
  FastLED.addLeds<OCTOWS2811>(leds, NUM_LEDS_PER_STRIP);
  FastLED.setBrightness( BRIGHTNESS );

  pinMode(0, INPUT_PULLUP);
  pinMode(23, INPUT_PULLUP);
  pinMode(19, INPUT_PULLUP);
  pinMode(17, INPUT_PULLUP);

  start_time = millis();
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
  uint16_t outer;
  CRGB pos_color;
  CRGB inner_color;

  uint8_t l;
  uint8_t h;
  
  Pulse(uint16_t p, uint16_t i, uint16_t o, CRGB pc, CRGB ic) {
    pos = p;
    inner = i;
    outer = o;
    pos_color = pc;
    inner_color = ic;

    if(ic == (CRGB) CRGB::Black) {
      l = 255 / 16;
      h = 255 * 10 / 16;
    } else {
      l = 255 / 4;
      h = 255 * 3 /4;
    }
  }

  void Run(Context *c) {
    //TODO: Doesn't wrap!

    // "Peak" of the pulse
    if( c->i == (pos-1) || c->i == (pos+1) ) {
      c->c = blend(inner_color, pos_color, l);
    } else if (c->i == pos) {
      c->c = pos_color;
    }

    // "Trough"
    else if( c->i >= (pos - 1 - inner) && c->i <= (pos + 1 + inner) ) {
      c->c = inner_color; 
    }

    // "Slope" (25% trough to 75% trough)
    else if( c->i >= (pos - 1 - inner - outer) && c->i <= (pos + 1 + inner + outer) ) {
      const uint8_t d = abs(pos - c->i) - 1 - inner - 1;
      fract8 percent = l + (h - l) * d / (outer - 1);
      c->c = blend(inner_color, c->c, percent);
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
  new Pulse(15, 2, 6, CRGB::Yellow, CRGB::Black),
  NULL
};

Effect * comp1[] = {
  new Running(1.5),
  new Pulse(15, 2, 6, CRGB::Red, CRGB::Black),
  NULL
};

Effect * comp2[] = {
  //new RandomRunning(1,1),
  new Running(-1),
  new Pulse(315, 4, 4, CRGB::Blue, CRGB::Black),
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
  uint16_t _t = (millis() - start_time);

  for (int _i = 0; _i < NUM_LEDS; _i++) {
    c.i = _i;
    c.t = _t/10;
    c.c = leds[_i];
    
    
    for (uint8_t e = 0; effects[e] != NULL; e++) {
      effects[e]->Run(&c);
    }

    leds[_i] = c.c;
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
      leds[pixelnumber] = color;
    }
}

void Rainbow() {
  static uint8_t hue = 0;
  for(int i = 0; i < NUM_STRIPS; i++) {
    for(int j = 0; j < NUM_LEDS_PER_STRIP; j++) {
      leds[(i*NUM_LEDS_PER_STRIP) + j] = CHSV((32*i) + hue+j,192,255);
    }
  }

  // Set the first n leds on each strip to show which strip it is
  for(int i = 0; i < NUM_STRIPS; i++) {
    for(int j = 0; j <= i; j++) {
      leds[(i*NUM_LEDS_PER_STRIP) + j] = CRGB::Red;
    }
  }

  hue++;
}

void dimmer(uint8_t down, uint8_t up) {
  if(digitalRead(down) == LOW) {
    BRIGHTNESS -= 1;
  }

  if(digitalRead(up) == LOW) {
    BRIGHTNESS += 1;
  }

  if(BRIGHTNESS < 0) {
    BRIGHTNESS = 0;
  }
  if(BRIGHTNESS > 255) {
    BRIGHTNESS = 255;
  }
  
  FastLED.setBrightness( BRIGHTNESS );
}

int _MODE = 0;
void modeSelect(uint8_t forward, uint8_t backward) {
  uint8_t count = 50;

  if(digitalRead(forward) == LOW) {
    _MODE -= 1;
  }

  if(digitalRead(backward) == LOW) {
    _MODE += 1;
  }

  if(_MODE < 0) {
    _MODE = 0;
  }
  
  if(_MODE > NUM_MODES * count) {
    _MODE = NUM_MODES * count;
  }

  MODE = _MODE / (count);
}

void loop() {

  dimmer(0, 23);
  modeSelect(19, 17);

  if(MODE == 0) {
    MasterRaverBaiter();
  }
  if(MODE == 1) {
    Rainbow();
  }
  if(MODE == 2) {
    for( int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::Red;
    }
  }
  if(MODE == 3) {
    for( int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::Blue;
    }
  }
  if(MODE == 4) {
    for( int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::Green;
    }
  }
  if(MODE == 5) {
    Fire2012();
  }

  FastLED.show(); // display this frame
}
