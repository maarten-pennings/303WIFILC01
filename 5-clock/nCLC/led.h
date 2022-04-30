// led.h - interface to control the LED on the 303WIFILC01 board
#ifndef _LED_H_
#define _LED_H_

void led_init();
void led_set(int on);
int  led_get();
void led_on();
void led_off();

#endif
