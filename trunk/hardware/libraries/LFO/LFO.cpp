#include "WProgram.h"
#include "LFO.h"

LFOClass::LFOClass() {
  type = LFO_NONE;
  acc = 0;
  inc = 0;
  oldacc = 0;
  oldval = 0;
}

uint8_t LFOClass::getValue() {
  uint8_t acc8 = acc >> 24;
  switch (type) {
  case LFO_TRIANGLE:
    if (acc8 > 127)
      curValue = 255 - acc8;
    else
      curValue = acc8;
    break;

  case LFO_SQUARE:
    if (acc8 > 127)
      curValue = 127;
    else
      curValue = 0;
    break;

  case LFO_RANDOM:
    if (acc8 != oldacc) {
      oldacc = acc8;
      oldval = random(127);
    }
    curValue = oldval;
    break;

  default:
    curValue = 0;
  }

  return curValue;
}

LFOClass LFO[4];
