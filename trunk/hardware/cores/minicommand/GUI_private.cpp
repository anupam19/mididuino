#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "MidiDuino.h"
#include "GUI_private.h"
#include "LCD.h"

#define SR165_OUT    PD5
#define SR165_SHLOAD PD6
#define SR165_CLK    PD7

#define SR165_DATA_PORT PORTD
#define SR165_DDR_PORT  DDRD
#define SR165_PIN_PORT  PIND

#define SR165_DELAY() { } // asm("nop"); } // asm("nop");  asm("nop");  }

inline void SR165Class::clk() {
  CLEAR_BIT8(SR165_DATA_PORT, SR165_CLK);
  SET_BIT8(SR165_DATA_PORT, SR165_CLK);
}

inline void SR165Class::rst() {
  CLEAR_BIT8(SR165_DATA_PORT, SR165_SHLOAD);
  SET_BIT8(SR165_DATA_PORT, SR165_SHLOAD);
}

SR165Class::SR165Class() {
  SR165_DDR_PORT |= _BV(SR165_SHLOAD) | _BV(SR165_CLK);
  CLEAR_BIT8(SR165_DDR_PORT, SR165_OUT);
  CLEAR_BIT8(SR165_DATA_PORT, SR165_OUT);
  SET_BIT8(SR165_DATA_PORT, SR165_CLK);
  SET_BIT8(SR165_DATA_PORT, SR165_SHLOAD);
}

uint8_t SR165Class::read() {
  rst();

  uint8_t res = 0;
  uint8_t i = 0;
  for (i = 0; i < 8; i++) {
    res <<= 1;
    res |= IS_BIT_SET8(SR165_PIN_PORT, SR165_OUT);
    clk();
  }

  return res;
}

uint8_t SR165Class::read_norst() {
  uint8_t res = 0;
  uint8_t i = 0;
  for (i = 0; i < 8; i++) {
    res <<= 1;
    res |= IS_BIT_SET8(SR165_PIN_PORT, SR165_OUT);
    clk();
  }

  return res;
}


uint16_t SR165Class::read16() {
  rst();

  uint16_t res = 0;
  uint8_t i = 0;
  for (i = 0; i < 16; i++) {
    res <<= 1;
    res |= IS_BIT_SET8(SR165_PIN_PORT, SR165_OUT);
    clk();
  }

  return res;
}

/**********************************************/

#define ENCODER_NORMAL(i) (encoders[(i)].normal)
#define ENCODER_SHIFT(i)  (encoders[(i)].shift)
#define ENCODER_BUTTON(i) (encoders[(i)].button)
#define ENCODER_BUTTON_SHIFT(i) (encoders[(i)].button_shift)

EncodersClass::EncodersClass() {
  clearEncoders();
  sr_old = 0;
}

void EncodersClass::clearEncoders() {
  for (uint8_t i = 0; i < GUI_NUM_ENCODERS; i++) {
    ENCODER_NORMAL(i) = ENCODER_BUTTON(i) = ENCODER_SHIFT(i) =
      ENCODER_BUTTON_SHIFT(i) = 0;
  }
}

void EncodersClass::poll(uint16_t sr) {
  uint16_t sr_tmp = sr;

  for (uint8_t i = 0; i < GUI_NUM_ENCODERS; i++) {
    if (IS_BIT_SET8(sr, 1) == !IS_BIT_SET8(sr_old, 1)) {
      volatile int8_t *val = &(ENCODER_NORMAL(i));
      
      if (BUTTON_DOWN(i)) {
       	if (BUTTON_DOWN(Buttons.SHIFT))
		  val = &(ENCODER_BUTTON_SHIFT(i));
	else
	  val = &(ENCODER_BUTTON(i));
      } else if (BUTTON_DOWN(Buttons.SHIFT)) {
	val = &(ENCODER_SHIFT(i));
      }

      if (IS_BIT_SET8(sr, 0) == IS_BIT_SET8(sr, 1)) {
	if (*val < 64)
	  (*val)++;
      } else {
	if (*val > -64)
	  (*val)--;
      }
    }
    sr >>= 2;
    sr_old >>= 2;
  }

  sr_old = sr_tmp;
}

/**********************************************/

ButtonsClass::ButtonsClass() {
  clear();
}

void ButtonsClass::clear() {
  for (int i = 0; i < GUI_NUM_BUTTONS; i++) {
    CLEAR_B_DOUBLE_CLICK(i);
    CLEAR_B_CLICK(i);
    CLEAR_B_LONG_CLICK(i);
    STORE_B_OLD(i, B_CURRENT(i));
  }
}

void ButtonsClass::poll(uint8_t but) {
  uint8_t but_tmp = but;

  for (uint8_t i = 0; i < GUI_NUM_BUTTONS; i++) {
    STORE_B_CURRENT(i, IS_BIT_SET8(but_tmp, 0));

    uint16_t sclock = read_slowclock();
    if (BUTTON_PRESSED(i)) {
      B_PRESS_TIME(i) =  sclock;
      
      if (B_PRESSED_ONCE(i)) {
	uint16_t diff = clock_diff(B_LAST_PRESS_TIME(i), B_PRESS_TIME(i));
	if (diff < DOUBLE_CLICK_TIME) {
	  SET_B_DOUBLE_CLICK(i);
	  CLEAR_B_PRESSED_ONCE(i);
	}
      } else {
	B_LAST_PRESS_TIME(i) = B_PRESS_TIME(i);
	SET_B_PRESSED_ONCE(i);
      }
    }
    
    if (BUTTON_DOWN(i) && B_PRESSED_ONCE(i)) {
      uint16_t diff = clock_diff(B_LAST_PRESS_TIME(i), sclock);
      if (diff > LONG_CLICK_TIME) {
	SET_B_LONG_CLICK(i);
	CLEAR_B_PRESSED_ONCE(i);
      }
    }

    if (BUTTON_UP(i) && B_PRESSED_ONCE(i)) {
      uint16_t diff = clock_diff(B_LAST_PRESS_TIME(i), sclock);
      if (diff > LONG_CLICK_TIME) {
	CLEAR_B_PRESSED_ONCE(i);
      } else if (diff > DOUBLE_CLICK_TIME) {
	CLEAR_B_PRESSED_ONCE(i);
	SET_B_CLICK(i);
      }
    }

    but_tmp >>= 1;
  }
}

SR165Class SR165;
EncodersClass Encoders;
ButtonsClass Buttons;
