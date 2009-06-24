#include "Encoders.hh"

#include "Midi.h"

/* handlers */

void CCEncoderHandle(Encoder *enc) {
  CCEncoder *ccEnc = (CCEncoder *)enc;
  MidiUart.sendCC(ccEnc->channel, ccEnc->cc, ccEnc->getValue());
}

void TempoEncoderHandle(Encoder *enc) {
  MidiClock.setTempo(enc->getValue());
}

/* Encoder */
Encoder::Encoder(char *_name, encoder_handle_t _handler) {
  old = 0;
  cur = 0;
  redisplay = false;
  setName(_name);
  handler = _handler;
}

void Encoder::checkHandle() {
  if (cur != old) {
    if (handler != NULL)
      handler(this);
  }
  
  old = cur;
}

void Encoder::setName(char *_name) {
  if (_name != NULL)
    m_strncpy_fill(name, _name, 4);
  name[3] = '\0';
}

void Encoder::setValue(int value, bool handle) {
  if (handle) {
    cur = value;
    checkHandle();
  } else {
    old = cur = value;
  }
  redisplay = true;
}

void Encoder::displayAt(int i) {
  GUI.put_value(i, getValue());
  redisplay = false;
}

bool Encoder::hasChanged() {
  return old != cur;
}

void Encoder::clear() {
  old = 0;
  cur = 0;
}

int Encoder::update(encoder_t *enc) {
  cur = cur + enc->normal + 5 * enc->button;
  return cur;
}

/* EnumEncoder */
void EnumEncoder::displayAt(int i) {
  GUI.put_string(i, enumStrings[getValue()]);
  redisplay = false;
}

/* RangeEncoder */

int RangeEncoder::update(encoder_t *enc) {
  int inc = enc->normal + 5 * enc->button;
  
  cur = limit_value(cur, inc, min, max);
  return cur;
}

/* CharEncoder */
CharEncoder::CharEncoder()  : RangeEncoder(0, 37) {
}

char CharEncoder::getChar() {
  uint8_t val = getValue();
  if (val == 0) {
    return ' ';
  }
  if (val < 27) 
    return val - 1+ 'A';
  else
    return (val - 27) + '0';
}

void CharEncoder::setChar(char c) {
  if (c >= 'A' && c <= 'Z') {
    setValue(c - 'A' + 1);
  } else if (c >= '0' && c <= '9') {
    setValue(c - '0' + 26 + 1);
  } else {
    setValue(0);
  }
}

