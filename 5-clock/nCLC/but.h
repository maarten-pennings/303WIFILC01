// but.h - interface to control the buttons on the 303WIFILC01 board
#ifndef _BUT_H_
#define _BUT_H_


#define BUT1 1
#define BUT2 2
#define BUT3 4


void but_scan();                // Scans the button state and records that; check with but_wentdown().
int  but_wentdown(int buttons); // Returns a mask of buttons pressed during the last `but_scan` and not pressed during the `but_scan` before that.
void but_init();                // Initializes the button driver. Configures the GPIO block for the button pin.


#endif
