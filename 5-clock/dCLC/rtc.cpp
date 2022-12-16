// rtc.cpp - driver for the rtc DS1302
#include <Arduino.h>
#include <Wire.h>
#include <ErriezDS1302.h> // https://github.com/Erriez/ErriezDS1302
#include "rtc.h"

#define CHK(err) {                      \
    if (!err) {                         \
        Serial.print(F("rtc : Failure: "));   \
        Serial.print((strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__));     \
        Serial.print(F(":"));           \
        Serial.println(__LINE__);       \
        Serial.flush();                 \
        noInterrupts();                 \
        while (1);                      \
    }                                   \
}


// Connect DS1302 data pin to Arduino DIGITAL pin
#define DS1302_CLK_PIN      16
#define DS1302_IO_PIN       14
#define DS1302_CE_PIN       5

// The number of registers in the DS1307
#define  ID_STR_LENGTH      8
static uint8_t *ID_STR = (uint8_t *)"dCLC_id";

// Create RTC object
ErriezDS1302 rtc = ErriezDS1302(DS1302_CLK_PIN, DS1302_IO_PIN, DS1302_CE_PIN);


bool rtc_init() {
  Serial.printf("rtc : init\n");

  // Initialize RTC
  CHK(rtc.begin());

  uint8_t buf[9];
  CHK(rtc.readBuffer(0, buf, 9));
  Serial.printf("rtc : regs - %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
                buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8]);

  // Trickle-Charge register - 0x8
  //            TCS BITS               Hex
  //    7   6   5   4   3   2   1   0        FUNCTION
  //    X   X   X   X   X   X   0   0        Disabled
  //    X   X   X   X   0   0   X   X        Disabled
  //    X   X   X   X   1   1   X   X        Disabled
  //    1   0   1   0   0   1   0   1   A5   1 Diode, 2kΩ
  //    1   0   1   0   0   1   1   0   A6   1 Diode, 4kΩ
  //    1   0   1   0   0   1   1   1   A7   1 Diode, 8kΩ
  //    1   0   1   0   1   0   0   1   A9   2 Diodes, 2kΩ
  //    1   0   1   0   1   0   1   0   AA   2 Diodes, 4kΩ
  //    1   0   1   0   1   0   1   1   AB   2 Diodes, 8kΩ
  //    0   1   0   1   1   1   0   0   5C   Disabled / Initial power-on state
  // activate Trickle-Charge. Set - 1 Diode, 2kΩ
  CHK(rtc.writeRegister(0x8, 0xA5));

  return true;
}

bool rtc_check() {
  uint8_t buf[ID_STR_LENGTH];

  if (!rtc.isRunning()) {
    Serial.printf("rtc : does not have a valid time. RTC not running.\n");
    return false;
  }

  rtc.readBufferRAM(buf, ID_STR_LENGTH);
  for (int i = 0; i < ID_STR_LENGTH; i++) {
    if (ID_STR[i] != buf[i]) {
      Serial.printf("rtc : read ID %02x %02x %02x %02x %02x %02x %02x %02x\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
      Serial.printf("rtc : Wrong char at i=%d  ID char: %02x - read char: %02x\n", i, ID_STR[i], buf[i]);
      Serial.printf("rtc : does not have a valid time. ID string not match.\n");
      return false;
    }
  }
  Serial.print("rtc : has a valid time\n");
  return true;
}

bool rtc_get(struct tm *dt) {
  CHK(rtc.read(dt));
  return true;
}

bool rtc_set(struct tm *dt) {
  CHK(rtc.clockEnable(true));
  rtc.writeBufferRAM(ID_STR, ID_STR_LENGTH);
  CHK(rtc.write(dt));
  return true;
}
