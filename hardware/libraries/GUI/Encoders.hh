#ifndef ENCODERS_H__
#define ENCODERS_H__

#include <inttypes.h>
#include <MidiClock.h>
#include <Midi.h>
#include "WProgram.h"
#include "GUI.h"

class Encoder;

typedef void (*encoder_handle_t)(Encoder *enc);

void CCEncoderHandle(Encoder *enc);
void TempoEncoderHandle(Encoder *enc);

class Encoder {
 protected:
  int old, cur;
  char name[4];

 public:
  encoder_handle_t handler;
  
  Encoder(char *_name = NULL, encoder_handle_t _handler = NULL);
  void clear();

  virtual char *getName() { return name; }
  virtual void setName(char *_name);
  
  bool redisplay;
  virtual int update(encoder_t *enc);
  virtual void checkHandle();
  virtual bool hasChanged();
  
  virtual int getValue() { return cur; }
  virtual int getOldValue() { return old; }
  virtual void setValue(int value, bool handle = false);

  virtual void displayAt(int i);
};

class RangeEncoder : public Encoder {
 public:
  int min;
  int max;

  RangeEncoder(int _max = 127, int _min = 0, char *_name = NULL, int init = 0,
	       encoder_handle_t _handler = NULL) : Encoder(_name, _handler) {
    if (_min > _max) {
      min = _max;
      max = _min;
    } else {
      min = _min;
      max = _max;
    }
    setValue(init);
  }
  virtual int update(encoder_t *enc);
};

class EnumEncoder : public RangeEncoder {
public:
  const char **enumStrings;
  int cnt;

  EnumEncoder(const char *strings[] = NULL, int _cnt = 0, char *_name = NULL, int init = 0,
	      encoder_handle_t _handler = NULL) :
    RangeEncoder(_cnt - 1, 0, _name, init, _handler) {
    enumStrings = strings;
    cnt = _cnt;
  }

  void initEnumEncoder(const char *strings[], int _cnt, char *_name = NULL, int init = 0) {
    enumStrings = strings;
    cnt = _cnt;
    min = 0;
    max = _cnt - 1;
    setValue(init);
    setName(_name);
  }

  virtual void displayAt(int i);
};

class PEnumEncoder : public EnumEncoder {
public:
  PEnumEncoder(const char *strings[], int _cnt, char *_name = NULL, int init = 0,
	      encoder_handle_t _handler = NULL) :
    EnumEncoder(strings, _cnt, _name, init, _handler) {
  }

  virtual void displayAt(int i);
};


class CCEncoder : public RangeEncoder {
 public:
  uint8_t cc;
  uint8_t channel;

  virtual uint8_t getCC() {
    return cc;
  }
  virtual uint8_t getChannel() {
    return channel;
  }
  virtual void initCCEncoder(uint8_t _channel, uint8_t _cc) {
    cc = _cc;
    channel = _channel;
  }
  
  CCEncoder(uint8_t _cc = 0, uint8_t _channel = 0, char *_name = NULL, int init = 0) :
    RangeEncoder(127, 0, _name, init) {
    initCCEncoder(_channel, _cc);
    handler = CCEncoderHandle;
  }
};

class TempoEncoder : public RangeEncoder {
  public:
  TempoEncoder(char *_name = NULL) : RangeEncoder(255, 20, _name) {
    handler = TempoEncoderHandle;
  }
};

class CharEncoder : public RangeEncoder {
public:
  CharEncoder();
  char getChar();
  void setChar(char c);
};

#include "RecordingEncoder.hh"


#endif /* ENCODERS_H__ */
