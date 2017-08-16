#ifndef MENU__LIB
#define MENU__LIB


class Bounce;

bool modeSelect(Bounce &forward, Bounce &backward, int &mode, const int num_modes);
bool dimmer(unsigned char down, unsigned char up, int &brightness);

#endif
