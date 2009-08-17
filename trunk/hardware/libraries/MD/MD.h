#ifndef MD_H__
#define MD_H__

#include "WProgram.h"

#include "Elektron.hh"

class MDCallback {
};
typedef void(MDCallback::*md_callback_ptr_t)();
typedef void(MDCallback::*md_status_callback_ptr_t)(uint8_t type, uint8_t param);


#include "MDSysex.hh"
#include "MDParams.hh"
#include "MDMessages.hh"

extern uint8_t machinedrum_sysex_hdr[5];

typedef struct tuning_s {
  uint8_t model;
  uint8_t base;
  uint8_t len;
  uint8_t offset;
  const uint8_t *tuning;
} tuning_t;

#ifdef MIDIDUINO_USE_GUI
class MDEncoder : public CCEncoder {
public:
  uint8_t track;
  uint8_t param;

  virtual uint8_t getCC();
  virtual uint8_t getChannel();
  virtual void initCCEncoder(uint8_t _channel, uint8_t _cc);
  void initMDEncoder(uint8_t _track = 0, uint8_t _param = 0, char *_name = NULL, uint8_t init = 0) {
    track = _track;
    param = _param;
    setName(_name);
    setValue(init);
  }

  MDEncoder(uint8_t _track = 0, uint8_t _param = 0, char *_name = NULL, uint8_t init = 0);
  void loadFromKit();
};

class MDFXEncoder : public RangeEncoder {
 public:
  uint8_t effect;
  uint8_t param;

  void initMDFXEncoder(uint8_t _param = 0, uint8_t _effect = MD_FX_ECHO, char *_name = NULL, uint8_t init = 0) {
    effect = _effect;
    param = _param;
    setName(_name);
    setValue(init);
  }
  MDFXEncoder(uint8_t _param = 0, uint8_t _effect = MD_FX_ECHO, char *_name = NULL, uint8_t init = 0);
  void loadFromKit();
};

class MDLFOEncoder : public RangeEncoder {
 public:
  uint8_t track;
  uint8_t param;

  void initMDLFOEncoder(uint8_t _param = MD_LFO_TRACK, uint8_t _track = 0,
			char *_name = NULL, uint8_t init = 0) {
    param = _param;
    track = _track;
    if (_name == NULL) {
      setLFOParamName();
    } else {
      setName(_name);
    }
    setValue(init);
  }

  MDLFOEncoder(uint8_t _param = MD_LFO_TRACK, uint8_t _track = 0,
	       char *_name = NULL, uint8_t init = 0);
  
  void loadFromKit();
  void setLFOParamName();
  void setParam(uint8_t _param) {
    param = _param;
    setLFOParamName();
  }

  virtual void displayAt(int i);
};

class MDTrackFlashEncoder : public RangeEncoder {
 public:
 MDTrackFlashEncoder(char *_name = NULL, uint8_t init = 0) : RangeEncoder(0, 15, _name, init) {
  }

  virtual void displayAt(int i);
};

class MDMelodicTrackFlashEncoder : public MDTrackFlashEncoder {
 public:
 MDMelodicTrackFlashEncoder(char *_name = NULL, uint8_t init = 0) : MDTrackFlashEncoder(_name, init) {
  }

  virtual void displayAt(int i);
};
#endif

extern uint8_t track_pitches[16];

class MDClass {
 public:
  MDClass();
  int currentGlobal;
  int currentKit;
  int currentPattern;

  bool loadedKit;
  MDKit kit;
  bool loadedGlobal;
  MDGlobal global;

  void parseCC(uint8_t channel, uint8_t cc, uint8_t *track, uint8_t *param);
  void triggerTrack(uint8_t track, uint8_t velocity);
  void setTrackParam(uint8_t track, uint8_t param, uint8_t value);

  void sendFXParam(uint8_t param, uint8_t value, uint8_t type);
  void setEchoParam(uint8_t param, uint8_t value);
  void setReverbParam(uint8_t param, uint8_t value);
  void setEQParam(uint8_t param, uint8_t value);
  void setCompressorParam(uint8_t param, uint8_t value);

  void sendRequest(uint8_t type, uint8_t param);

  uint8_t trackGetPitch(uint8_t track, uint8_t pitch);
  uint8_t trackGetCCPitch(uint8_t track, uint8_t cc, int8_t *offset = NULL);
  
  void sendNoteOn(uint8_t track, uint8_t pitch,
			    uint8_t velocity);

  void sliceTrack32(uint8_t track, uint8_t from, uint8_t to, bool correct = true);
  void sliceTrack16(uint8_t track, uint8_t from, uint8_t to);

  const tuning_t* getModelTuning(uint8_t model);
  uint8_t noteToTrack(uint8_t pitch);
  bool isMelodicTrack(uint8_t track);

  void setLFOParam(uint8_t track, uint8_t param, uint8_t value);
  void setLFO(uint8_t track, MDLFO *lfo);
  
  void assignMachine(uint8_t track, uint8_t model);
  
  void setMachine(uint8_t track, MDMachine *machine);

  void muteTrack(uint8_t track, bool mute = true);
  void unmuteTrack(uint8_t track) {
    muteTrack(track, false);
  }

  void setStatus(uint8_t id, uint8_t value);
  void loadGlobal(uint8_t id);
  void loadKit(uint8_t kit);
  void loadPattern(uint8_t pattern);
  void loadSong(uint8_t song);
  void setSequencerMode(uint8_t mode);
  void setLockMode(uint8_t mode);
  
  void saveCurrentKit(uint8_t pos);

  PGM_P getMachineName(uint8_t machine);
  void getPatternName(uint8_t pattern, char str[5]);

  void requestKit(uint8_t kit);
  void requestPattern(uint8_t pattern);
  void requestSong(uint8_t song);
  void requestGlobal(uint8_t global);

  /* check channel settings to see if MD can receive and send CC for params */
  bool checkParamSettings();
  bool checkTriggerSettings();
  bool checkClockSettings();
};

extern MDClass MD;

#include "MDTask.hh"

#endif /* MD_H__ */
