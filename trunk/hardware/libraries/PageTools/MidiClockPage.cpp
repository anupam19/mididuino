#include "MidiClockPage.h"
#include <SDCard.h>
#include <Merger.h>

Merger merger;

const char *MidiClockPage::clockSourceEnum[] = {
    "NO", "IN1", "IN2"
  };

static const char *mergerConfigStrings[] = {
  "NONE   ",
  "CC     ",
  "NOTE   ",
  "SYX    ",
  "CC+NOTE",
  "CC+SYX ",
  "NOT+SYX",
  "ALL    "
};

static uint8_t mergerConfigMasks[] = {
  0,
  Merger::MERGE_CC_MASK,
  Merger::MERGE_NOTE_MASK,
  Merger::MERGE_SYSEX_MASK,
  Merger::MERGE_CC_MASK | Merger::MERGE_NOTE_MASK,
  Merger::MERGE_CC_MASK | Merger::MERGE_SYSEX_MASK,
  Merger::MERGE_NOTE_MASK | Merger::MERGE_SYSEX_MASK,
  Merger::MERGE_CC_MASK | Merger::MERGE_NOTE_MASK | Merger::MERGE_SYSEX_MASK,
};

void MidiClockPage::writeClockSettings() {
  uint8_t buf[2];
  buf[0] = MidiClock.mode;
  buf[1] = MidiClock.transmit ? 1 : 0;
  if (!SDCard.writeFile("/ClockSettings.txt", buf, 2, true)) {
    GUI.flash_strings_fill("ERROR SAVING", "CLOCK SETUP");
  }
}

void MidiClockPage::readClockSettings() {
  uint8_t buf[2];
  if (!SDCard.readFile("/ClockSettings.txt", buf, 2)) {
    GUI.flash_strings_fill("ERROR READING", "CLOCK SETUP");
  } else {
    MidiClock.mode = (MidiClockClass::clock_mode_t)buf[0];
    MidiClock.transmit = buf[1];
    if (MidiClock.mode == MidiClock.EXTERNAL_MIDI || MidiClock.mode == MidiClock.EXTERNAL_UART2) {
      MidiClock.start();
    }
  }
}

void MidiClockPage::writeMergeSettings() {
  uint8_t buf[2];
  buf[0] = merger.mask;
  if (!SDCard.writeFile("/MergeSettings.txt", buf, 2, true)) {
    GUI.flash_strings_fill("ERROR SAVING", "MERGE SETUP");
  }
}

void MidiClockPage::readMergeSettings() {
  uint8_t buf[2];
  if (!SDCard.readFile("/MergeSettings.txt", buf, 1)) {
    GUI.flash_strings_fill("ERROR READING", "MERGE SETUP");
  } else {
    merger.setMergeMask(buf[0]);
  }
}

void MidiClockPage::setup() {
  clockSourceEncoder.initEnumEncoder(clockSourceEnum, 3, "CLK");
  transmitEncoder.initBoolEncoder("SND");
  mergerEncoder.initEnumEncoder(mergerConfigStrings, countof(mergerConfigStrings), "MRG");
  readClockSettings();
  switch (MidiClock.mode) {
  case MidiClockClass::OFF:
    clockSourceEncoder.setValue(0);
    break;
  case MidiClockClass::EXTERNAL_MIDI:
    clockSourceEncoder.setValue(1);
    break;
  case MidiClockClass::EXTERNAL_UART2:
    clockSourceEncoder.setValue(2);
    break;
  }
  if (MidiClock.transmit) {
    transmitEncoder.setValue(1);
  }

  readMergeSettings();
  for (uint8_t i = 0; i < countof(mergerConfigMasks); i++) {
    if (mergerConfigMasks[i] == merger.mask) {
      mergerEncoder.setValue(i);
    }
  }
  
  encoders[0] = &clockSourceEncoder;
  encoders[1] = &transmitEncoder;
  encoders[2] = &mergerEncoder;
  
}

void MidiClockPage::show() {
    GUI.flash_strings_fill("MIDI CLOCK", "SETUP");
    redisplayPage();
  }

void MidiClockPage::loop() {
  bool changed = false;
  if (clockSourceEncoder.hasChanged()) {
    switch (clockSourceEncoder.getValue()) {
    case 0: // NONE
      MidiClock.stop();
      MidiClock.mode = MidiClock.OFF;
      //      GUI.flash_strings_fill("MIDI CLOCK SRC", "NONE");
      break;
      
    case 1: // MIDI 1
      MidiClock.stop();
      MidiClock.mode = MidiClock.EXTERNAL_MIDI;
      //      GUI.flash_strings_fill("MIDI CLOCK SRC", "MIDI PORT 1");
      MidiClock.start();
      break;
      
    case 2: // MIDI 2
      MidiClock.stop();
      MidiClock.mode = MidiClock.EXTERNAL_UART2;
      //      GUI.flash_strings_fill("MIDI CLOCK SRC", "MIDI PORT 2");
      MidiClock.start();
      break;
    }
    changed = true;
  }
  if (transmitEncoder.hasChanged()) {
    if (transmitEncoder.getBoolValue()) {
      MidiClock.transmit = true;
      //      GUI.flash_strings_fill("MIDI CLOCK OUT", "ACTIVATED");
    } else {
      MidiClock.transmit = false;
      //      GUI.flash_strings_fill("MIDI CLOCK OUT", "DEACTIVATED");
    }
    changed = true;
  }
  if (mergerEncoder.hasChanged()) {
      uint8_t mask = mergerConfigMasks[mergerEncoder.getValue()];
      merger.setMergeMask(mask);
      changed = true;
  }

  if (changed) {
    writeMergeSettings();
    writeClockSettings();
  }
}

 bool MidiClockPage::handleEvent(gui_event_t *event) {
  if (EVENT_PRESSED(event, Buttons.BUTTON1)) {
    GUI.sketch->popPage(this);
    return true;
  }
  return true;
}

MidiClockPage midiClockPage;