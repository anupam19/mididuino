#include "MidiClock.h"
#include "midi-common.hh"
#include "helpers.h"
#include "MidiUart.h"

MidiClockClass::MidiClockClass() {
  init();
  mode = OFF;
  setTempo(120);
  on96Callback = NULL;
  on32Callback = NULL;
  on16Callback = NULL;
  transmit = false;
}

void MidiClockClass::init() {
  state = PAUSED;
  counter = 0;
  last_clock = 0;
  div96th_counter = 0;
  div32th_counter = 0;
  div16th_counter = 0;
  mod6_counter = 0;
  indiv96th_counter = 0;
  inmod6_counter = 0;
  pll_x = 220;
}

uint16_t midi_clock_diff(uint16_t old_clock, uint16_t new_clock) {
  if (new_clock >= old_clock)
    return new_clock - old_clock;
  else
    return new_clock + (65535 - old_clock);
}

/* in interrupt */
void MidiClockClass::handleClock() {
  uint16_t cur_clock = clock;
  uint16_t diff;
  if (cur_clock > last_clock)
    diff = cur_clock - last_clock;
  else
    diff = cur_clock + (65535 - last_clock);
  last_interval = diff;
  last_clock = cur_clock;
  indiv96th_counter++;
  inmod6_counter++;
  if (inmod6_counter == 6)
    inmod6_counter = 0;

  switch (state) {
  case PAUSED:
    break;

  case STARTING:
    state = STARTED;
    break;

  case STARTED:
    if (indiv96th_counter == 2) {
      interval = diff;
    } else {
      interval =
	(((uint32_t)interval * (uint32_t)pll_x) +
	 (uint32_t)(256 - pll_x) * (uint32_t)diff) >> 8;
    }
    break;
  }
}

void MidiClockClass::handleMidiStart() {
  if (mode == EXTERNAL_MIDI || mode == EXTERNAL_UART2) {
    init();
    state = STARTING;
    mod6_counter = 0;
    div96th_counter = 0;
    div32th_counter = 0;
    div16th_counter = 0;
    counter = 0;
  }
}

void MidiClockClass::handleMidiStop() {
  if (mode == EXTERNAL_MIDI || mode == EXTERNAL_UART2) {
    state = PAUSED;
  }
}

#define PHASE_FACTOR 16
static uint32_t phase_mult(uint32_t val) {
  return (val * PHASE_FACTOR) >> 8;
}

void MidiClockClass::start() {
  if (mode == INTERNAL_MIDI) {
    state = STARTED;
    mod6_counter = 0;
    div96th_counter = 0;
    div32th_counter = 0;
    div16th_counter = 0;
    if (transmit)
      MidiUart.sendRaw(MIDI_START);
  }
}

void MidiClockClass::stop() {
  if (mode == INTERNAL_MIDI) {
    state = PAUSED;
    if (transmit)
      MidiUart.sendRaw(MIDI_STOP);
  }
}

void MidiClockClass::pause() {
  if (mode == INTERNAL_MIDI) {
    if (state == PAUSED) {
      start();
    } else {
      stop();
    }
  }
}

void MidiClockClass::setTempo(uint16_t _tempo) {
  uint8_t tmp = SREG;
  cli();
  tempo = _tempo;
  //  interval = 62500 / (tempo * 24 / 60);
  interval = (uint32_t)((uint32_t)156250 / tempo) - 16;
  SREG = tmp;
}

void MidiClockClass::handleTimerInt()  {
  if (counter == 0) {
    counter = interval;

    if (mod6_counter == 0) {
      if (on16Callback)
	on16Callback();
      if (on32Callback)
	on32Callback();
      div16th_counter++;
      div32th_counter++;
    }
    if (mod6_counter == 3) {
      if (on32Callback)
	on32Callback();
      div32th_counter++;
    }
    
    div96th_counter++;
    mod6_counter++;
    
    if (mode == EXTERNAL_MIDI || mode == EXTERNAL_UART2) {
      uint16_t cur_clock = clock;
      uint16_t diff = midi_clock_diff(last_clock, cur_clock);
      
      if ((div96th_counter < indiv96th_counter) ||
	  (div96th_counter > (indiv96th_counter + 1))) {
	div96th_counter = indiv96th_counter;
	mod6_counter = inmod6_counter;
      }
      
      if (div96th_counter <= indiv96th_counter) {
	counter -= phase_mult(diff);
      } else {
	if (counter > diff) {
	  counter += phase_mult(counter - diff);
	}
      }
    } else if (mode == INTERNAL_MIDI) {
      if (transmit)
	MidiUart.putc_immediate(MIDI_CLOCK);
    }
    
    if (on96Callback)
      on96Callback();

    if (mod6_counter == 6) {
      mod6_counter = 0;
    }
  } else {
    counter--;
  }
}
    
MidiClockClass MidiClock;
