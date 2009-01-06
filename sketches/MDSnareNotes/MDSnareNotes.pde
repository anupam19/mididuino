void setup() {
  LCD.line1("MD NOTE TEST");
  MD::trackModels[0] = EFM_SD_MODEL;
  MD::trackModels[1] = EFM_SD_MODEL;
  MD::trackModels[2] = EFM_SD_MODEL;
  MD::trackModels[3] = EFM_SD_MODEL;
}

void loop() {
}

void handleGui() {
  uint8_t i;
  for (i = 0; i < 4; i++) {
    if (BUTTON_PRESSED(i + 4))
      MD::sendNoteOn(i, scalePitch(random(47, 80), 47, minorScale), 100);
  }
}
