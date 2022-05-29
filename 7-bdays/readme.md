# Birthday clock

New firmware for the ESP8266 based _303 WIFI LC 01 clock_.

![bCLC](bCLC.png)


## Introduction

The video below shows a clock from Ali Express. It is driven by an ESP8266. 
I replaced the firmware, now it is NTP synced, and it reads a birthday calendar 
from Google sheets. The lower left 'decimal point' of the display is on, 
indicating there is a birthday this week. 
Every 5 minutes, a message scrolls telling in how many days the birthday is, 
whose birthday it is, and how old (s)he will be.

[![Video](../video.png)](https://youtu.be/UseIozlsI0w)


## bCLC firmware

See the [Arduino](bCLC) project for the new firmware.

If you compile yourself, chose "Generic ESP8266 module" as board, 
otherwise the EEPROM layout will not match the hardware.

(end)

