#ifndef MD_H__
#define MD_H__

#include <inttypes.h>
#include <avr/pgmspace.h>
#include "WProgram.h"

#include "Elektron.hh"
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

typedef struct machine_name_s {
  char name[7];
  uint8_t id;
} machine_name_t;

#ifdef MIDIDUINO_USE_GUI
class MDEncoder : public RangeEncoder {
public:
  uint8_t track;
  uint8_t param;

  MDEncoder(uint8_t _track, uint8_t _param, char *_name = NULL, uint8_t init = 0);
  void handle(uint8_t val);
};
#endif

class MDClass {
 public:
  MDClass() {
    currentGlobal = -1;
    currentKit = -1;
    baseChannel = 0;
  }
  int currentGlobal;
  int currentKit;
  uint8_t baseChannel;
  MDKit kit;
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
  
};

extern MDClass MD;

#endif /* MD_H__ */