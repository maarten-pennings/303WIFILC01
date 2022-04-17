# PCB analysis of the 303WIFILC01 clock

Reverse engineering the board.

## First impressions

![PCB](pcb.png)

The product (PCB) has no branding
 - Board ID: 303 WIFI LC 01
 - Brand: LC
 - Shop: [AliExpress CFsunbird Store](https://www.aliexpress.com/item/1005003486084193.html)
 - Documentation: [best found](https://dailystuffstore.com/display-screen/707-1903820-0-8-inch-display-screen-digital-tube-wifi-clock-module-automatic-clock-diy-digital.html)

Notes
 - The back has a "header" P1 that can be connected to an "FTDI" header.
 - The labels for P1 are confusing; they are _below_ the holes
 - The P1 5V is indeed a 5V - ie connected to + on power header P3 or the 5V of the USB
 - S1 needed extra soldering (top contact was loose) on two different boards.

## Components

 - U1: AMS1117 low dropout voltage regulator
   -- [datasheet](http://www.advanced-monolithic.com/pdf/ds1117.pdf).

 - U2 : ESP8266EX microcontroller 
   -- [datasheet](https://www.espressif.com/sites/default/files/documentation/0a-esp8266ex_datasheet_en.pdf).

 - U3: DS1302 Trickle-Charge Timekeeping Chip
   -- [datasheet](https://eu.mouser.com/datasheet/2/256/DS1302-1292062.pdf).

 - U4: not present

 - U5: LED drive controller / keyboard scan ASIC TM1650
   -- [page with link to datasheet](https://components101.com/ics/tm1650-led-driver-ic).

 - U6: P25Q80H Ultra Low Power, 8M-bit Serial Multi I/O Flash Memory
   -- [datasheet](https://datasheet.lcsc.com/szlcsc/PUYA-P25Q80H-SSH-IT_C194872.pdf).

## Power domains

![Power domains](pcb-power.png)  


## GPIO nets

![GPIO nets](pcb-gpio.png)  


(end)
  
