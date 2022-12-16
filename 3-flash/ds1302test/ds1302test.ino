#include <ErriezDS1302.h> // https://github.com/Erriez/ErriezDS1302
// board: Generic ESP8266 module


// Connect DS1302 data pin to Arduino DIGITAL pin
#define DS1302_CLK_PIN      16
#define DS1302_IO_PIN       14
#define DS1302_CE_PIN       5

// Create RTC object
ErriezDS1302 rtc = ErriezDS1302(DS1302_CLK_PIN, DS1302_IO_PIN, DS1302_CE_PIN);

#define CHK(err) {                      \
    if (!err) {                         \
        Serial.print(F("Failure: "));   \
        Serial.print((strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__));     \
        Serial.print(F(":"));           \
        Serial.println(__LINE__);       \
        Serial.flush();                 \
        noInterrupts();                 \
        while (1);                      \
    }                                   \
}

// A test epoch "Sunday, September 6, 2020 18:20:30"
#define EPOCH_TEST      1599416430UL


void setup() {
    struct tm dtw;
    struct tm dtr;
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
    uint8_t mday;
    uint8_t mon;
    uint16_t year;
    uint8_t wday;

    // Initialize serial port
    delay(500);
    Serial.begin(115200);
    while (!Serial);
    Serial.println("\n\n -- DS1302 begin test");

    // Initialize RTC
    Serial.println("DS1302 init");
    CHK(rtc.begin());

    // Test isRunning() / clockEnable()
    if (!rtc.isRunning()) {
        CHK(rtc.clockEnable(true));
    }
    CHK(rtc.isRunning());

    // Test setEpoch()
    Serial.println("DS1302 set epoch");
    CHK(rtc.setEpoch(EPOCH_TEST));
    delay(1500);

    // Verify epoch struct tm
    CHK(rtc.read(&dtr));
    CHK(dtr.tm_sec == 31);
    CHK(dtr.tm_min == 20);
    CHK(dtr.tm_hour == 18);
    CHK(dtr.tm_mday == 6);
    CHK(dtr.tm_mon == 8);
    CHK(dtr.tm_year == 120);
    CHK(dtr.tm_wday == 0);

    // Verify getEpoch()
    CHK(rtc.getEpoch() == (EPOCH_TEST + 1));

    // Test write()
    Serial.println("DS1302 test write/read");
    dtw.tm_hour = 12;
    dtw.tm_min = 34;
    dtw.tm_sec = 56;
    dtw.tm_mday = 29;
    dtw.tm_mon = 1; // 0=January
    dtw.tm_year = 2020 - 1900;
    dtw.tm_wday = 6; // 0=Sunday
    CHK(rtc.write(&dtw));

    // Test read()
    CHK(rtc.read(&dtr));
    CHK(dtw.tm_sec == dtr.tm_sec);
    CHK(dtw.tm_min == dtr.tm_min);
    CHK(dtw.tm_hour == dtr.tm_hour);
    CHK(dtw.tm_mday == dtr.tm_mday);
    CHK(dtw.tm_mon == dtr.tm_mon);
    CHK(dtw.tm_year == dtr.tm_year);
    CHK(dtw.tm_wday == dtr.tm_wday);

    // Test setTime()
    Serial.println("DS1302 test setTime");
    CHK(rtc.setTime(12, 34, 56));
    delay(1500);
    CHK(rtc.getTime(&hour, &min, &sec));
    CHK((hour == 12) && (min == 34) && (sec == 57));

    // Test setDateTime()   13:45:09  31 December 2019  Tuesday
    Serial.println("DS1302 test setDateTime");
    CHK(rtc.setDateTime(13, 45, 9, 31, 12, 2019, 2));
    delay(1500);

    // Test getDateTime()
    CHK(rtc.getDateTime(&hour, &min, &sec, &mday, &mon, &year, &wday));
    CHK(hour == 13);
    CHK(min == 45);
    CHK(sec == 10);
    CHK(mday == 31);
    CHK(mon == 12);
    CHK(year == 2019);
    CHK(wday == 2);

    // Verify setDateTime() with read()
    CHK(rtc.read(&dtr));
    CHK(dtr.tm_sec == 10);
    CHK(dtr.tm_min == 45);
    CHK(dtr.tm_hour == 13);
    CHK(dtr.tm_mday == 31);
    CHK(dtr.tm_mon == 11);    // Month 0..11
    CHK(dtr.tm_year == (2019 - 1900)); // Year - 1900
    CHK(dtr.tm_wday == 2);    // 0=Sun, 1=Mon, 2=Tue

    // Test RTC RAM
    Serial.println("DS1302 read internal RAM");
    uint8_t buf[DS1302_NUM_RAM_REGS];
    memset(buf, 0, DS1302_NUM_RAM_REGS);
    rtc.readBufferRAM(buf, DS1302_NUM_RAM_REGS);

    for (int i = 0; i < DS1302_NUM_RAM_REGS; i++) {
        if (i % 8 == 0) {
            Serial.printf("\n %02x  ", i);
        }
        Serial.printf(" %02x", buf[i]);
    }
    Serial.println("");

    Serial.println("DS1302 test internal RAM");
    for (int i = 0; i < DS1302_NUM_RAM_REGS; i++) {
        buf[i] = i;
    }
    rtc.writeBufferRAM(buf, DS1302_NUM_RAM_REGS);
    memset(buf, 0, DS1302_NUM_RAM_REGS);
    rtc.readBufferRAM(buf, DS1302_NUM_RAM_REGS);
    for (int i = 0; i < DS1302_NUM_RAM_REGS; i++) {
        CHK(buf[i] == i);
    }

    // Completed
    Serial.println(" -- DS1302 Test passed\n");
}

void loop() {
    struct tm dtr;
    CHK(rtc.read(&dtr));
    Serial.printf("Time: %d %02d %02d %d - %02d:%02d:%02d\n", dtr.tm_year + 1900, dtr.tm_mon + 1, dtr.tm_mday, dtr.tm_wday, dtr.tm_hour, dtr.tm_min, dtr.tm_sec);

    delay(1000);
}
