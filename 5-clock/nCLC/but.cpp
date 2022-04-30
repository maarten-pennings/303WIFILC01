// but.cpp - control the buttons on the 303WIFILC01 board

#include <Arduino.h>
#include "but.h"


#define BUT1_PIN 0
#define BUT2_PIN 4
#define BUT3_PIN 15


// The button state of the last two `but_scan` calls.
static int but_state_prev;
static int but_state_cur;


// Scans the button state and records that; check with but_wentdown().
void but_scan() {
  but_state_prev = but_state_cur;
  but_state_cur = 0;
  if( digitalRead(BUT1_PIN)==LOW  ) but_state_cur |= BUT1; // Low active
  if( digitalRead(BUT2_PIN)==LOW  ) but_state_cur |= BUT2; // Low active
  if( digitalRead(BUT3_PIN)==HIGH ) but_state_cur |= BUT3; // High active
}


// Returns a mask of buttons pressed during the last `but_scan` and not pressed during the `but_scan` before that.
int but_wentdown(int buttons) {
  return (but_state_prev ^ but_state_cur) & but_state_cur & buttons;
}


// Initializes the button driver.
// Configures the GPIO block for the button pin.
void but_init() {
  pinMode(BUT1_PIN, INPUT_PULLUP); // Low active
  pinMode(BUT2_PIN, INPUT_PULLUP); // Low active
  pinMode(BUT3_PIN, INPUT);        // High active
  but_scan();
  but_scan();
  Serial.printf("but : init\n");
}
