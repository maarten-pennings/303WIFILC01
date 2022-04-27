// basicclock.ino - alternative firmware for 303WIFILC01: basic NTP clock
// board: Generic ESP8266 module
#define VERSION "1.0"

#include <coredecls.h> // Only needed for settimeofday_cb()
#include <time.h>
#include <Cfg.h> // A library that lets a user configure an ESP8266 app (https://github.com/maarten-pennings/Cfg)
#include "led.h"
#include "but.h"
#include "disp.h"
#include "wifi.h"


NvmField cfg_fields[] = {
  {"Access points"   , ""                           ,  0, "The clock uses internet to get time. Supply credentials for one or more WiFi access points (APs). " },
  {"Ssid.1"          , "SSID for AP1"               , 32, "The ssid of the first wifi network the WordClock could connect to (mandatory)." },
  {"Password.1"      , "Password for AP1"           , 32, "The password of the first wifi network the WordClock could connect to (mandatory). "},
  {"Ssid.2"          , "SSID for AP2"               , 32, "The ssid of the second wifi network (optional, may be blank)." },
  {"Password.2"      , "Password for AP2"           , 32, "The password of the second wifi network (optional, may be blank). "},
  {"Ssid.3"          , "SSID for AP3"               , 32, "The ssid of the third wifi network (optional, may be blank)." },
  {"Password.3"      , "Password for AP3"           , 32, "The password of the third wifi network (optional, may be blank). "},
  
  {"Time management" , ""                           ,  0, "Time is obtained from so-called NTP servers. They provide UTC time, so also the time-zone must be entered. " },
  {"NTP.server.1"    , "pool.ntp.org"               , 32, "The hostname of the first NTP server." },
  {"NTP.server.2"    , "europe.pool.ntp.org"        , 32, "The hostname of a second NTP server." },
  {"NTP.server.3"    , "north-america.pool.ntp.org" , 32, "The hostname of a third NTP server. " },
  {"Timezone"        , "CET-1CEST,M3.5.0,M10.5.0/3" , 48, "The timezone string (including daylight saving), see <A href='https://www.gnu.org/software/libc/manual/html_node/TZ-Variable.html'>details</A>. " },
  {0                 , 0                            ,  0, 0},  
};
#define CFG_BUT_PIN 0
#define CFG_LED_PIN 2
Cfg cfg("Clock", cfg_fields, CFG_SERIALLVL_USR, CFG_LED_PIN);


void setup() {
  Serial.begin(115200);
  Serial.printf("\n\n");
  Serial.printf("main: Welcome to basicclock %s\n",VERSION);

  // On boot: check if config button is pressed
  cfg.check(60,CFG_BUT_PIN); // Wait 60 flashes (of 50ms) for a change on pin CFG_BUT_PIN
  // if in config mode, do config setup (when config completes, it restarts the device)
  if( cfg.cfgmode() ) { cfg.setup(); return; }
  Serial.printf("main: no configuration requested, started clock %s\n\n",VERSION);
  
  led_init();
  but_init();
  wifi_init(cfg.getval("Ssid.1"),cfg.getval("Password.1"), cfg.getval("Ssid.2"),cfg.getval("Password.2"), cfg.getval("Ssid.3"),cfg.getval("Password.3"));
  disp_init();

  configTime( cfg.getval("Timezone"), cfg.getval("NTP.server.1"), cfg.getval("NTP.server.2"), cfg.getval("NTP.server.3"));
  Serial.printf("clk : init: %s %s %s\n", cfg.getval("NTP.server.1"), cfg.getval("NTP.server.2"), cfg.getval("NTP.server.3"));
  Serial.printf("clk : timezone: %s\n", cfg.getval("Timezone") );
  settimeofday_cb( [](){Serial.printf("main: NTP sync\n");} );  // Pass lambda function to print SET when time is set

  // Show the app is running, and local time not yet synced
  disp_power_set(1);
  disp_show("SYNC");
  Serial.printf("\n");
  led_on(); 
}


// Record last received seconds, to blink colon
int       colon_prev = -1;
uint32_t  colon_msecs;

// Showing time or date
int       show_date; 

void loop() {
  // if in config mode, do config loop (when config completes, it restarts the device)
  if( cfg.cfgmode() ) { cfg.loop(); return; }

  // In normal application mode
  led_set( !wifi_isconnected() );     // LED is on when not connected
  but_scan();

  if( but_wentdown(BUT3) ) disp_brightness_set( disp_brightness_get()%8 + 1 );
  if( but_wentdown(BUT2) ) show_date = !show_date;
  
  time_t      tnow= time(NULL);       // Returns seconds (and writes to the passed pointer, when not NULL) - note `time_t` is just a `long`.
  struct tm * snow= localtime(&tnow); // Returns a struct with time fields (https://www.tutorialspoint.com/c_standard_library/c_function_localtime.htm)
  bool        sync= snow->tm_year>120;// We miss-use "old" time as indication of "time not yet set" (year is 1900 based)

  // If seconds changed: print
  if( snow->tm_sec != colon_prev ) {
    // In `snow` the `tm_year` field is 1900 based, `tm_month` is 0 based, rest is as expected
    //Serial.printf("main: %d-%02d-%02d %02d:%02d:%02d (dst=%d) %s\n", snow->tm_year + 1900, snow->tm_mon + 1, snow->tm_mday, snow->tm_hour, snow->tm_min, snow->tm_sec, snow->tm_isdst, sync?"":"NO NTP" );
    // Record that seconds changed, for colon
    colon_prev=snow->tm_sec; 
    colon_msecs=millis();
  }

  if( sync && show_date ) {
    char bnow[5];
    sprintf(bnow,"%02d%02d", snow->tm_mday, snow->tm_mon+1 );
    disp_show(bnow,DISP_DOTCOLON);
  }
  
  if( sync && !show_date ) {
    char bnow[5];
    sprintf(bnow,"%02d%02d", snow->tm_hour, snow->tm_min );
    int dots = millis()-colon_msecs<500 ? DISP_DOTNO : DISP_DOTCOLON;
    disp_show(bnow,dots);
  }
}
