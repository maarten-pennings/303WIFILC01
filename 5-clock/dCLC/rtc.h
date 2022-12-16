// rtc.h - interface for driver for the RTC DS1307
#ifndef _RTC_H_
#define _RTC_H_


bool rtc_init();                 // Initializes DS1307 (prints error to Serial)
bool rtc_check();                // Checks for the correct time in the DS1307
bool rtc_get(struct tm *dt);     // Get current time from DS1307
bool rtc_set(struct tm *dt);     // Set current time in DS1307

#endif
