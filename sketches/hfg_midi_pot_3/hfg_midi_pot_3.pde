// Einbinden der Ruinwesen MIDI Library
#include <MidiUart.h>
#include <Midi.h>
MidiClass Midi;

int potPin = 0;
int tasterPin = 3;

void setup() {
  // Initialisieren der MIDI Schnittstelle
  MidiUart.init();
  pinMode(tasterPin, INPUT);
  digitalWrite(tasterPin, HIGH);
}

int tasterStatus = HIGH;
int pitch = 0;
int speed = 300;

int pitches[7] = { 0, 2, 3, 5, 7, 8, 9 };

int alterCCWert = -1;

void loop() {
  int tasterWert = digitalRead(tasterPin);
  
  int potWert = analogRead(0);
  int ccWert = map(potWert, 0, 1023, 0, 127);
  
  if (abs(ccWert - alterCCWert) >= 1) {
    alterCCWert = ccWert;
    if (digitalRead(tasterPin) == HIGH) {
      MidiUart.sendCC(4, ccWert);
    } else {
      speed = map(potWert, 0, 1023, 30, 500);
    }
  }
  
      pitch = pitches[random(0, 6)];
      int octave = random(0, 2) * 12 + 48;
      pitch = pitch + octave;
      
      MidiUart.sendNoteOn(pitch, 100);
      
      delay(30);
      // taster losgelassen
      MidiUart.sendNoteOff(pitch);
      
      delay(speed);
}
