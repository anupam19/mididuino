#ifndef MIDIDUINO_PRIVATE_H__
#define MIDIDUINO_PRIVATE_H__

#include "helpers.h"

void init(void);

void delayMicroseconds(unsigned int us);
void delay(unsigned int ms);

inline void setLed(void) {
  CLEAR_BIT(PORTE, PE4);
}

inline void clearLed(void) {
  SET_BIT(PORTE, PE4);
}

inline void setLed2(void) {
  CLEAR_BIT(PORTE, PE5);
}

inline void clearLed2(void) {
  SET_BIT(PORTE, PE5);
}


extern uint16_t read_clock(void);
extern uint16_t read_slowclock(void);
extern volatile uint16_t clock;
extern volatile uint16_t slowclock;

uint16_t clock_diff(uint16_t old_clock, uint16_t new_clock);
uint8_t u_limit_value(uint8_t value, int8_t encoder, uint8_t min, uint8_t max);

#define FIRMWARE_LENGTH_ADDR ((uint16_t *)0x00)
#define FIRMWARE_CHECKSUM_ADDR ((uint16_t *)0x02)
#define START_MAIN_APP_ADDR ((uint16_t *)0x04)

void start_bootloader(void);

extern "C" {
  #include "helpers.h"
}

#endif /* MIDIDUIDNO_PRIVATE_H__ */
