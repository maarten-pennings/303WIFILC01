// disptest.ino - tests for the TM1650 display controller
// board: Generic ESP8266 module

#include <Wire.h>
#include <TM1650.h> // 7 segment display driver for TM1650 by 2015 Anatoli Arkhipenko (Sketch > Include library)

TM1650 tm1650;

#define SCL_PIN 12
#define SDA_PIN 13

void setup() {
  Serial.begin(115200);
  Serial.printf("\n\ndisptest.ino\n\n");
  Wire.begin(SDA_PIN,SCL_PIN);
  Serial.printf("i2c : init\n");
  tm1650.init();
  Serial.printf("disp: init\n");
  tm1650.displayOn();
}

// The segments are strangely ordered. Need to fix this: the TM1650.h font is thus wrong
int segments[] = { 1<<5, 1<<7, 1<<2, 1<<3, 1<<4, 1<<6, 1<<1, 1<<0 };
int pos;
int seg;

void loop() {
  tm1650.setPosition(pos, segments[seg]);
  delay(200);
  seg = (seg+1)%8;
  if( seg==0 ) {
    tm1650.setPosition(pos,0);
    pos = (pos+1)%4;
  }
}
