#ifndef WProgram_h
#define WProgram_h

extern "C" {
#include <inttypes.h>
#include <avr/interrupt.h>
}

#include "mididuino_private.h"

#ifdef __cplusplus
#include "LCD.h"
#include "GUI.h"
#include "Midi.h"
#include "MidiUart.h"
#include "USBMidiUart.h"
#include "MidiDuino.h"
#include "EncoderPage.h"
#include "MidiClock.h"

uint16_t makeWord(uint16_t w);
uint16_t makeWord(uint8_t h, uint8_t l);

#define word(...) makeWord(__VA_ARGS__)

unsigned long pulseIn(uint8_t pin, uint8_t state, unsigned long timeout = 1000000L);

// WMath prototypes
long random(long);
long random(long, long);
void randomSeed(unsigned int);
long map(long, long, long, long, long);
#endif

/* default config flags */
#define MIDIDUINO_POLL_GUI     1
#define MIDIDUINO_POLL_GUI_IRQ 1
#define MIDIDUINO_HANDLE_SYSEX 1
#define MIDIDUINO_MIDI_CLOCK   1
#define MIDIDUINO_ENABLE_USB   1
#define MIDIDUINO_RESET_COMBO  1
#define MIDIDUINO_ENABLE_LFOS  1

#endif /* WProgram_h */

