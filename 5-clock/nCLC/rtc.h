// rtc.h - interface for driver for the RTC DS1307
#ifndef _RTC_H_
#define _RTC_H_


bool rtcInit();                 // Initializes DS1307 (prints error to Serial)
bool rtcCheck();                // Checks for the correct time in the DS1307
bool rtcGet(struct tm *dt);     // Get current time from DS1307
bool rtcSet(struct tm *dt);     // Set current time in DS1307

#endif
