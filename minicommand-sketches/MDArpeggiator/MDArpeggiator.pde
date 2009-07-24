#include <MD.h>
#include "MidiTools.h"
#include "Arpeggiator.hh"

MDArpeggiatorClass arpeggiator;

bool loadedKit = false;
bool triggerRecording = false;
bool recording = false;
bool endRecording = false;

void handleGuiSysex() {
}

void setup() {
  setupMidi();
  setupPages();
}

void loop() {
  GUI.updatePage();
  
  loopPages();
  
  if (MidiUart.rxRb.overflow > 0) {
    setLed();
  }
  GUI.update();
}

void handleGui() {
  handleGuiSysex();
  handleGuiPages();
}
