#include "MD.h"
#include "MDPattern.hh"
#include "Elektron.hh"
#include "MDMessages.hh"
#include "helpers.h"
#include "MDParams.hh"
#include "GUI.h"

#ifdef HOST_MIDIDUINO
#include <stdio.h>
#endif

// #include "GUI.h"

void MDPattern::init() {
  numRows = 0;

	m_memclr(this, sizeof(MDPattern));

	m_memset(lockTracks, sizeof(lockTracks), -1);
	m_memset(lockParams, sizeof(lockParams), -1);
	m_memset(locks, sizeof(locks), -1);
	m_memset(paramLocks, sizeof(paramLocks), -1);

	//  accentPattern = 0;
	slidePattern = 0;
	swingPattern = 0;
	//  accentAmount = 0;
  accentEditAll = 1;
  swingEditAll = 1;
  slideEditAll = 1;
	//	doubleTempo = 0;

  patternLength = 16;
  swingAmount = 50 << 14;
	//  origPosition = 0;
	//  kit = 0;
	//  scale = 0;
}

bool MDPattern::fromSysex(uint8_t *data, uint16_t len) {
  init();
  
  if ((len != (0xACA - 6)) && (len != (0x1521 - 6)))  {
#ifdef AVR
    GUI.flash_string_fill("WRONG LENGTH");
#else
    printf("WRONG LENGTH: %x\n", len);
#endif
    return false;
  }

	isExtraPattern = (len == (0x1521 - 6));
  
	if (!ElektronHelper::checkSysexChecksum(data, len)) {
    return false;
  }

	uint8_t *ptr = data + 3;
  origPosition = *ptr;
	MDSysexDecoder decoder(DATA_ENCODER_INIT(data + 0xA - 6, 74));
	decoder.get32(trigPatterns, 16);

	decoder.init(DATA_ENCODER_INIT(data + 0x54 - 6, 74));
	decoder.get32(lockPatterns, 16);

	decoder.init(DATA_ENCODER_INIT(data + 0x9e - 6, 19));

#if 0
	accentPattern = decoder.gget32();
	slidePattern = decoder.gget32();
	swingPattern = decoder.gget32();
	swingAmount = decoder.gget32();
#else

		decoder.get32(&accentPattern);
		decoder.get32(&slidePattern);
		decoder.get32(&swingPattern);
		decoder.get32(&swingAmount);
#endif

	ptr = data + 0xb1 - 6;
  accentAmount = *(ptr++);
  patternLength = *(ptr++);
  doubleTempo = (*(ptr++) == 1);
  scale = *(ptr++);
  kit = *(ptr++);
  numLockedRows = *(ptr++);

	decoder.init(DATA_ENCODER_INIT(data + 0xb7 - 6, 2341));
	for (uint8_t i = 0; i < 64; i++) {
		decoder.get(locks[i], 32);
	}

	decoder.init(DATA_ENCODER_INIT(data + 0x9DC - 6, 234));
	{
		uint32_t tmp[3];
		decoder.get32(tmp, 3);
		accentEditAll = (tmp[0] == 1);
		slideEditAll = (tmp[1] == 1);
		swingEditAll = (tmp[2] == 1);
	}
	decoder.get32(accentPatterns, 16);
	decoder.get32(slidePatterns, 16);
	decoder.get32(swingPatterns, 16);

  for (int i = 0; i < 16; i++) {
    for (int j = 0; j < 24; j++) {
      if (IS_BIT_SET32(lockPatterns[i], j)) {
				paramLocks[i][j] = numRows;
				lockTracks[numRows] = i;
				lockParams[numRows] = j;
				numRows++;
      }
    }
  }

  if (isExtraPattern) {
		decoder.init(DATA_ENCODER_INIT(data + 0xac6 - 6, 2647));
		decoder.get32hi(trigPatterns, 16);
		decoder.get32hi(&accentPattern);
		decoder.get32hi(&slidePattern);
		decoder.get32hi(&swingPattern);
		for (uint8_t i = 0; i < 64; i++) {
			decoder.get(locks[i] + 32, 32);
		}
		decoder.get32hi(accentPatterns, 16);
		decoder.get32hi(slidePatterns, 16);
		decoder.get32hi(swingPatterns, 16);
  }

  return true;
}

uint16_t MDPattern::toSysex(uint8_t *data, uint16_t len) {
	isExtraPattern = patternLength > 32;

	cleanupLocks();
  recalculateLockPatterns();

	uint16_t sysexLength = isExtraPattern ? 0x151d : 0xac6;
  if (len < (sysexLength + 5))
    return 0;

  m_memcpy(data + 1, machinedrum_sysex_hdr, sizeof(machinedrum_sysex_hdr));
  data[6] = MD_PATTERN_MESSAGE_ID;
  data[7] = 0x03; // version
  data[8] = 0x01;
  data[9] = origPosition;
	
	MDDataToSysexEncoder encoder(DATA_ENCODER_INIT(data + 10, 74));
	encoder.pack32(trigPatterns, 16);
	encoder.finish();

	encoder.init(DATA_ENCODER_INIT(data + 0x54, 74));
	encoder.pack32(lockPatterns, 16);
	encoder.finish();

	encoder.init(DATA_ENCODER_INIT(data + 0x9e, 19));
	encoder.pack32(accentPattern);
	encoder.pack32(slidePattern);
	encoder.pack32(swingPattern);
	encoder.pack32(swingAmount);
	encoder.finish();
    
  data[0xB1] = accentAmount;
  data[0xB2] = patternLength;
  data[0xb3] = doubleTempo ? 1 : 0;
  data[0xb4] = scale;
  data[0xb5] = kit;
  data[0xb6] = numLockedRows;

  uint16_t cnt = 0;

	encoder.init(DATA_ENCODER_INIT(data + 0xB7, 2341));
	uint8_t lockData[64][32];
	m_memclr(lockData, 64 * 32);

	uint8_t lockIdx = 0;
  for (int track = 0; track < 16; track++) {
    for (int param = 0; param < 24; param++) {
      int8_t lock = paramLocks[track][param];
      if (lock != -1) {
				m_memcpy(lockData[lockIdx], locks[lock], 32);
				lockIdx++;
				cnt++;
      }
    }
  }
	encoder.pack((uint8_t *)lockData, 64 * 32);
	encoder.finish();
  
	encoder.init(DATA_ENCODER_INIT(data + 0x9dc, 234));
	encoder.pack32(accentEditAll ? 1 : 0);
	encoder.pack32(slideEditAll ? 1 : 0);
  encoder.pack32(swingEditAll ? 1 : 0);

	encoder.pack32(accentPatterns, 16);
	encoder.pack32(slidePatterns, 16);
	encoder.pack32(swingPatterns, 16);
	
	encoder.finish();

	if (isExtraPattern) {
		encoder.init(DATA_ENCODER_INIT(data + 0xac6, 2647));
		for (int i = 0; i < 16; i++) {
			encoder.pack32hi(trigPatterns[i]);
		}
		encoder.pack32hi(accentPattern);
		encoder.pack32hi(slidePattern);
		encoder.pack32hi(swingPattern);

		lockIdx = 0;
		m_memclr(lockData, 32 * 64);
		for (int track = 0; track < 16; track++) {
			for (int param = 0; param < 24; param++) {
				int8_t lock = paramLocks[track][param];
				if (lock != -1) {
					
					m_memcpy(lockData[lockIdx], locks[lock] + 32, 32);
					lockIdx++;
					cnt++;
				}
			}
		}
		encoder.pack((uint8_t *)lockData, 64 * 32);
		encoder.finish();
	}

	ElektronHelper::calculateSysexChecksum(data, sysexLength);

  return sysexLength + 5;
}

bool MDPattern::isTrackEmpty(uint8_t track) {
	return ((trigPatterns[track] == 0) &&
					(lockPatterns[track] == 0));
}

bool MDPattern::isLockPatternEmpty(uint8_t idx) {
  for (int i = 0; i < 64; i++) {
    if (locks[idx][i] != 255)
      return false;
  }
  return true;
}

bool MDPattern::isLockPatternEmpty(uint8_t idx, uint64_t trigs) {
  for (int i = 0; i < 64; i++) {
    if (locks[idx][i] != 255 && IS_BIT_SET64(trigs, i))
      return false;
  }

  return true;
}

void MDPattern::cleanupLocks() {
  for (int i = 0; i < 64; i++) {
    if (lockTracks[i] != -1) {
			//			printf("checking lock %d for track %d and param %d\n", i, lockTracks[i], lockParams[i]);
      if (isLockPatternEmpty(i, trigPatterns[lockTracks[i]])) {
				if (lockParams[i] != -1) {
					paramLocks[lockTracks[i]][lockParams[i]] = -1;
				}
				lockTracks[i] = -1;
				lockParams[i] = -1;
      }
    } else {
      lockParams[i] = -1;
    }
  }
}

void MDPattern::clearPattern() {
  init();
}

void MDPattern::clearTrack(uint8_t track) {
  if (track >= 16)
    return;
  for (int i = 0; i < 64; i++) {
    clearTrig(track, i);
  }
	// XXX swing and slide
  clearTrackLocks(track);
}

void MDPattern::clearLockPattern(uint8_t lock) {
  if (lock >= 64)
    return;
  
  for (int i = 0; i < 64; i++) {
    locks[lock][i] = 255;
  }
  if (lockTracks[lock] != -1 && lockParams[lock] != -1) {
    paramLocks[lockTracks[lock]][lockParams[lock]] = -1;
  }
  lockTracks[lock] = -1;
  lockParams[lock] = -1;
}

bool MDPattern::isParamLocked(uint8_t track, uint8_t param) {
  return paramLocks[track][param] != -1;
}

void MDPattern::clearParamLocks(uint8_t track, uint8_t param) {
  int8_t idx = paramLocks[track][param];
  if (idx != -1) {
    clearLockPattern(idx);
    paramLocks[track][param] = -1;
  }
}

void MDPattern::clearTrackLocks(uint8_t track) {
  for (int i = 0; i < 24; i++) {
    clearParamLocks(track, i);
  }
}

void MDPattern::clearTrig(uint8_t track, uint8_t trig) {
  CLEAR_BIT64(trigPatterns[track], trig);
  for (int i = 0; i < 24; i++) {
    clearLock(track, trig, i);
  }
}

int8_t MDPattern::getNextEmptyLock() {
  for (int i = 0; i < 64; i++) {
    if (lockTracks[i] == -1 && lockParams[i] == -1) {
      return i;
    }
  }
  return -1;
}

bool MDPattern::addLock(uint8_t track, uint8_t trig, uint8_t param, uint8_t value) {
  int8_t idx = paramLocks[track][param];
  if (idx == -1) {
    idx = getNextEmptyLock();
    if (idx == -1)
      return false;
    paramLocks[track][param] = idx;
    lockTracks[idx] = track;
    lockParams[idx] = param;
    for (int i = 0; i < 64; i++) {
      locks[idx][i] = 255;
    }
  }
  locks[idx][trig] = value;
  return true;
}

void MDPattern::clearLock(uint8_t track, uint8_t trig, uint8_t param) {
  int8_t idx = paramLocks[track][param];
  if (idx == -1)
    return;
  locks[idx][trig] = 255;

  if (isLockPatternEmpty(idx, trigPatterns[track])) {
    paramLocks[track][param] = -1;
    lockTracks[idx] = -1;
    lockParams[idx] = -1;
  }
}

void MDPattern::recalculateLockPatterns() {
  for (int track = 0; track < 16; track++) {
    lockPatterns[track] = 0;
    for (int param = 0; param < 24; param++) {
      if (paramLocks[track][param] != -1) {
				SET_BIT32(lockPatterns[track], param);
      }
    }
  }
}

uint8_t MDPattern::getLock(uint8_t track, uint8_t trig, uint8_t param) {
  int8_t idx = paramLocks[track][param];
  if (idx == -1)
    return 255;
  return locks[idx][trig];
}
			 
