#include "menu.h"
#include <Bounce2.h>
#include <EEPROM.h>

bool modeSelect(Bounce &forward, Bounce &backward, int &mode, const int num_modes) {
  
  forward.update();
  backward.update();

  bool updated = false;
  
  if(forward.rose()) {
      mode++;
      updated = true;
  } else if (backward.rose()) {
    mode--;
    updated = true;
  }

  if(mode < 0) {
    mode = num_modes - 1;
  }

  if(mode >= num_modes) {
    mode = 0;
  }

  return updated;
}

bool dimmer(unsigned char down, unsigned char up, int &brightness) {
  bool updated = false;
  
  if(digitalRead(down) == LOW) {
    brightness -= 1;
    updated = true;
  }

  if(digitalRead(up) == LOW) {
    brightness += 1;
    updated = true;
  }

  if(brightness < 0) {
    brightness = 0;
    updated = true;
  }
  if(brightness > 255) {
    brightness = 255;
    updated = true;
  }

  return updated;
}

