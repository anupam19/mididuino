/**
 **# USB Strings
 **/

#ifndef MIDI_LINK_STR_H__
#define MIDI_LINK_STR_H__


static PROGMEM uint8_t usb_manufacturer_string[] = {
  18 + 2,
  USB_DESCRIPTOR_STRING,
  'M', 0, 'i', 0, 'd', 0, 'i', 0, 'D', 0, 'u', 0, 'i', 0, 'n', 0, 'o', 0,
};

static PROGMEM uint8_t usb_product_string[] = {
  20 + 2,
  USB_DESCRIPTOR_STRING,
  'B', 0, 'o', 0, 'o', 0, 't', 0, 'l', 0, 'o', 0, 'a', 0, 'd', 0, 'e', 0,
   'r', 0
};


#endif /* MIDI_LINK_STR_H__ */
