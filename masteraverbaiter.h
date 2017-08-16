#ifndef MASTERAVERBAITER__LIB
#define MASTERAVERBAITER__LIB

// COMPONENTS

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
#endif
