// disp.h - interface for driver for the 303WIFILC01 display/TM1650
#ifndef _DISP_H_
#define _DISP_H_

// Constants for dots in disp_show()
#define DISP_DOTNO     0
#define DISP_DOT1      1
#define DISP_DOTCOLON  2 // Hardware maps DOT3 to the colon
#define DISP_DOT3      4
#define DISP_DOT4      8
#define DISP_ALL       ( DISP_DOT1 | DISP_DOT2 | DISP_DOTCOLON | DISP_DOT4 )

void disp_brightness_set(int brightness);       // Sets brightness level 1..8
int  disp_brightness_get();                     // Gets brightness level
void disp_power_set(int power);                 // Sets power 0 (off) or 1 (on)
int  disp_power_get();                          // Gets power level
                                              
void disp_init();                               // Initializes display (prints error to Serial)
void disp_show(const char * s, uint8_t dots=0); // Puts (first 4 chars of) `s` (padded with spaces) on display, using flags in `dots` for P

#endif
