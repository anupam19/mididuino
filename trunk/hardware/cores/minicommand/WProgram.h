#ifndef WProgram_h
#define WProgram_h

extern "C" {
#include <inttypes.h>
#include <avr/interrupt.h>
}

extern "C" {
  #include "helpers.h"
}

#include "mididuino_private.h"

#ifdef __cplusplus
#include "LCD.h"
#include "GUI_private.h"
#include "MidiUart.h"
#include "MidiClock.h"
#include "Stack.h"
#include "GUI.h"
#include "Midi.h"
#include "WMath.h"

#endif

/* default config flags */
#define MIDIDUINO_POLL_GUI     1
#define MIDIDUINO_POLL_GUI_IRQ 1
#define MIDIDUINO_HANDLE_SYSEX 1
#define MIDIDUINO_MIDI_CLOCK   1

#endif /* WProgram_h */
