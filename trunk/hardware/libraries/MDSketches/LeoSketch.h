#ifndef LEO_SKETCH_H__
#define LEO_SKETCH_H__

#include <MD.h>
#include <Scales.h>

scale_t *currentScale = &minorMin7Arp;

class LeoPage : public EncoderPage {
  public:
  MDTrackFlashEncoder trackStartEncoder;
  BoolEncoder triggerOnOffEncoder;
  RangeEncoder kitSelectEncoder;
  RangeEncoder patternSelectEncoder;
  
  LeoPage() : trackStartEncoder("STR"), triggerOnOffEncoder("TRG"),
     kitSelectEncoder(0, 64, "KIT"), patternSelectEncoder(0, 127, "PAT") {
       encoders[0] = &trackStartEncoder;
       encoders[1] = &triggerOnOffEncoder;
       encoders[2] = &kitSelectEncoder;
       encoders[3] = &patternSelectEncoder;
  }
  
  virtual void loop() {
    if (kitSelectEncoder.hasChanged()) {
      MD.loadKit(kitSelectEncoder.getValue());
    }
    if (patternSelectEncoder.hasChanged()) {
      MD.loadPattern(patternSelectEncoder.getValue());
    }
  }
  
  virtual void display() {
    EncoderPage::display();
    if (patternSelectEncoder.hasChanged()) {
      uint8_t pattern = patternSelectEncoder.getValue();
      char name[5];
      MD.getPatternName(pattern, name);
      GUI.put_string_at(12, name);
    }
  }
  
  virtual bool handleEvent(gui_event_t *event) {
    for (uint8_t i = Buttons.BUTTON1; i <= Buttons.BUTTON4; i++) {
      if (EVENT_PRESSED(event, i)) {
        static const uint8_t ccValues[8] = {
          10, 20,  30, 50, 70, 90, 100, 110
        };
        uint8_t track = trackStartEncoder.getValue() + (i - Buttons.BUTTON1);
        uint8_t pitch = randomScalePitch(currentScale, 1) ;
        uint8_t realPitch = MD.trackGetPitch(track, pitch);
        MD.setTrackParam(track, 0, ccValues[random(8)]);
        
        if (triggerOnOffEncoder.getValue() == 1) {
          MD.triggerTrack(track, 100);
        }
      }
    }
  }
};

class LeoSnareNotesSketch : public Sketch, public MDCallback {
  LeoPage leoPage;
  public:
  virtual void setup() {
    MDTask.addOnKitChangeCallback(this, (md_callback_ptr_t)&LeoSnareNotesSketch::onKitChanged);
    setPage(&leoPage);
  }
  
  void onKitChanged() {
    GUI.setLine(GUI.LINE1);
    GUI.flash_p_string_fill(PSTR("SWITCH KIT"));
    GUI.setLine(GUI.LINE2);
    GUI.flash_string_fill(MD.kit.name);
  }

};


#endif /* LEO_SKETCH_H__ */
