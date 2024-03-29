#ifndef MDPATTERN_H__
#define MDPATTERN_H__

#include <inttypes.h>
#include "ElektronPattern.hh"

class MDPatternShort {
public:
	uint8_t origPosition;
	uint8_t kit;
	uint8_t patternLength;

	MDPatternShort() {
	}

	bool fromSysex(uint8_t *sysex, uint16_t len);
};

class MDPattern : public ElektronPattern {
public:
  uint8_t origPosition;

	uint8_t getPosition() { return origPosition; }
	void setPosition(uint8_t _pos) { origPosition = _pos; }

	/* SUPER IMPORTANT DO NOT CHANGE THE ORDER OF DECLARATION OF THESE VARIABLES */

  uint64_t trigPatterns[16];
  uint32_t lockPatterns[16];

  uint64_t accentPattern;
  uint64_t slidePattern;
  uint64_t swingPattern;
  uint64_t swingAmount;

  uint8_t accentAmount;
  uint8_t patternLength;
  uint8_t doubleTempo;
  uint8_t scale;

  uint8_t kit;
  uint8_t numLockedRows; // unused

	uint8_t getKit() { return kit; }
	void    setKit(uint8_t _kit) { kit = _kit; }

	uint8_t getLength() { return patternLength; }
	void    setLength(uint8_t _len) { patternLength = _len; }

  uint32_t accentEditAll;
  uint32_t slideEditAll;
  uint32_t swingEditAll;
  uint64_t accentPatterns[16];
  uint64_t slidePatterns[16];
  uint64_t swingPatterns[16];

  uint8_t numRows;
  int8_t paramLocks[16][24];
	int8_t getLockIdx(uint8_t track, uint8_t param) {
		return paramLocks[track][param];
	}
	void setLockIdx(uint8_t track, uint8_t param, int8_t value) {
		paramLocks[track][param] = value;
	}

  bool isExtraPattern;

  MDPattern() : ElektronPattern() {
		maxSteps = 64;
		maxParams = 24;
		maxTracks = 16;
		maxLocks = 64;

    isExtraPattern = false;
		init();
  }

  /* XXX TODO extra pattern 64 */

  bool fromSysex(uint8_t *sysex, uint16_t len);
  uint16_t toSysex(uint8_t *sysex, uint16_t len);
	uint16_t toSysex(ElektronDataToSysexEncoder &encoder);

	bool isTrackEmpty(uint8_t track);
	
  void clearPattern();
  void clearTrack(uint8_t track);
  
  void clearTrig(uint8_t track, uint8_t trig);
  inline void setTrig(uint8_t track, uint8_t trig) {
		SET_BIT64(trigPatterns[track], trig);
	}
	inline bool isTrigSet(uint8_t track, uint8_t trig) {
		return IS_BIT_SET64(trigPatterns[track], trig);
	}
	void setNote(uint8_t track, uint8_t step, uint8_t pitch);

	virtual void recalculateLockPatterns();
};

#endif /* MDPATTERN_H__ */

