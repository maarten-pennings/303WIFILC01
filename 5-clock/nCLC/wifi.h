// wifi.h - interface to control the wifi
#ifndef _WIFI_H_
#define _WIFI_H_


void wifi_init(char*s1,char*p1,char*s2,char*p2, char*s3,char*p3); // Initializes the WiFi driver
bool wifi_isconnected(); // Prints WiFi status to the user (over Serial, only when changed), and returns true iff connected


#endif
