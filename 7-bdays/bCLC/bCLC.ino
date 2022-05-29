// bCLC.ino - alternative firmware for 303WIFILC01: NTP clock with birthday calendar
// board: Generic ESP8266 module
#define VERSION "1.1"


// 20220529  1.1  Maarten  Font tweaked, no double bday, better error msgs, fixed bug in bday log
// 20220528  1.0  Maarten  Created initial version by merging nCLC and webcfg


#include <coredecls.h> // Only needed for settimeofday_cb()
#include <core_version.h> // ARDUINO_ESP8266_RELEASE
#include <time.h>
#include <Cfg.h> // A library that lets a user configure an ESP8266 app (https://github.com/maarten-pennings/Cfg)
#include "led.h"
#include "but.h"
#include "disp.h"
#include "wifi.h"
#include "cal.h"


// A demo spreadsheet
#define CALURL "https://docs.google.com/spreadsheets/d/1gv-RtwrtDsTuNT-kzcBqK_BdRv3ASNXGBXvXiZIdj4E/export?format=csv&gid=690945139"


NvmField cfg_fields[] = {
  {"Access points"   , ""                           ,  0, "The clock uses internet to get time. Supply credentials for one or more WiFi access points (APs). " },
  {"Ssid.1"          , "SSID for AP1"               , 32, "The ssid of the first wifi network the clock could connect to (mandatory)." },
  {"Password.1"      , "Password for AP1"           , 32, "The password of the first wifi network the clock could connect to (mandatory). "},
  {"Ssid.2"          , "SSID for AP2"               , 32, "The ssid of the second wifi network (optional, may be blank)." },
  {"Password.2"      , "Password for AP2"           , 32, "The password of the second wifi network (optional, may be blank). "},
  {"Ssid.3"          , "SSID for AP3"               , 32, "The ssid of the third wifi network (optional, may be blank)." },
  {"Password.3"      , "Password for AP3"           , 32, "The password of the third wifi network (optional, may be blank). "},
  
  {"Time management" , ""                           ,  0, "Time is obtained from so-called NTP servers. They provide UTC time, so also the time-zone must be entered. " },
  {"NTP.server.1"    , "pool.ntp.org"               , 32, "The hostname of the first NTP server." },
  {"NTP.server.2"    , "europe.pool.ntp.org"        , 32, "The hostname of a second NTP server." },
  {"NTP.server.3"    , "north-america.pool.ntp.org" , 32, "The hostname of a third NTP server. " },
  {"Timezone"        , "CET-1CEST,M3.5.0,M10.5.0/3" , 48, "The timezone string (including daylight saving), see <A href='https://www.gnu.org/software/libc/manual/html_node/TZ-Variable.html'>details</A>. " },

  {"Rendering"       , ""                           ,  0, "Determines how time and date is shown on the display. " },
  {"hours"           , "24"                         ,  3, "Use <b>24</b> or <b>12</b> for 24 or 12 hour clock; append <b>a</b> or <b>p</b> to use decimal point for am or pm." },
  {"dateorder"       , "d"                          ,  2, "Use <b>d</b> for day-month (europe) or <b>m</b> month-day (US) order." },
  {"monthnames"      , "JaFeMrApMYJnJlAuSeOcNoDe"   , 24, "Supply 12 pairs of letters for month names, otherwise month will be numbered. " },

  {"Calendar "       , ""                           ,  0, "Loads a calendar from some webserver and shows birthdays on the display. " },
  {"calurl"          , CALURL                       ,128, "URL to a CSV file with name,YYYY-MM-DD lines (if blank, no calendar)." },
  {"caldays"         , "7"                          ,  3, "Birthdays are listed if they are within 'caldays' days." },
  {"calmin"          , "5"                          ,  3, "Birthdays are displayed if minutes is divisable by 'calmin'. " },

  {0                 , 0                            ,  0, 0},  
};


#define CFG_BUT_PIN 0 // Button to select Config mode (mapped to the SET button)
#define CFG_LED_PIN 2 // LED to indicate Config mode (mapped to the (only) LED on the board
Cfg cfg("bCLC", cfg_fields, CFG_SERIALLVL_USR, CFG_LED_PIN);


int render_hours_len; // 12 or 24
#define      RENDER_HOURS_FLAG_AM   0
#define      RENDER_HOURS_FLAG_PM   1
#define      RENDER_HOURS_FLAG_NO   2
int          render_hours_flag; // RENDER_HOURS_FLAG_AM, RENDER_HOURS_FLAG_PM or RENDER_HOURS_FLAG_NO
const char * render_hours_flag_names[3] = {"am","pm","no"};
int          render_dayfirst; 
const char * render_months=" 1 2 3 4 5 6 7 8 9101112";


// Do we need to load the calendar; this flag is raised true on start-up, at midnight, and on user request (button press)
bool cal_tobe_loaded;

// Margin for birthdays; if they are this close, show them.
int  caldays;

// Birthdays are displayed if minutes is divisable by 'calmin'.
int  calmin;


void setup() {
  Serial.begin(115200);
  do delay(500); while( !Serial );
  Serial.printf("\n\n\n\n");
  Serial.printf("main: Welcome to bCLC, an NTP clock with a birthday calendar, version %s\n\n",VERSION);

  Serial.printf("main: Nvm %s\n",NVM_VERSION);
  Serial.printf("main: Cfg %s\n",CFG_VERSION);
  Serial.printf("main: Arduino ESP32 " ARDUINO_ESP8266_RELEASE "\n" );
  Serial.printf("main: compiler " __VERSION__ "\n" );
  Serial.printf("main: arduino %d\n",ARDUINO );
  Serial.printf("main: compiled " __DATE__ ", " __TIME__ "\n" );

  // Identify ourselves, regardless if we go into config mode or the real app
  disp_init();
  disp_power_set(1);
  disp_show("bClC");

  // On boot: check if config button is pressed
  cfg.check(100,CFG_BUT_PIN); // Wait 100 flashes (of 50ms) for a change on pin CFG_BUT_PIN
  // if in config mode, do config setup (when config completes, it restarts the device)
  if( cfg.cfgmode() ) { cfg.setup(); return; }
  Serial.printf("main: no configuration requested, starting clock\n\n");

  // LED on
  led_init();
  led_on(); 
  but_init();

  // Preprocess config for rendering
  disp_show("NtP");
  if( cfg.getval("hours")[0]=='1' && cfg.getval("hours")[1]=='2' ) render_hours_len=12; else render_hours_len=24;
  if( cfg.getval("hours")[2]=='a' ) render_hours_flag = RENDER_HOURS_FLAG_AM;
  else if( cfg.getval("hours")[2]=='p' ) render_hours_flag = RENDER_HOURS_FLAG_PM;
  else render_hours_flag = RENDER_HOURS_FLAG_NO;
  Serial.printf("rend: hours %d, flag %s\n",render_hours_len, render_hours_flag_names[render_hours_flag]);
  render_dayfirst = cfg.getval("dateorder")[0]!='m';
  if( strlen(cfg.getval("monthnames"))==24 ) render_months=cfg.getval("monthnames");
  Serial.printf("rend: date %s, names '%s'\n",render_dayfirst?"day:month":"month:day",render_months);

  // WiFi and NTP
  wifi_init(cfg.getval("Ssid.1"),cfg.getval("Password.1"), cfg.getval("Ssid.2"),cfg.getval("Password.2"), cfg.getval("Ssid.3"),cfg.getval("Password.3"));
  configTime( cfg.getval("Timezone"), cfg.getval("NTP.server.1"), cfg.getval("NTP.server.2"), cfg.getval("NTP.server.3"));
  Serial.printf("clk : init: %s %s %s\n", cfg.getval("NTP.server.1"), cfg.getval("NTP.server.2"), cfg.getval("NTP.server.3"));
  Serial.printf("clk : timezone: %s\n", cfg.getval("Timezone") );
  settimeofday_cb( [](){Serial.printf("clk : NTP sync\n");} );  // Pass lambda function to print SET when time is set
  // Give now a chance to the settimeofday callback, because it is *always* deferred to the next yield()/loop()-call.
  yield();
  
  // Calendar
  cal_init();
  cal_tobe_loaded = cfg.getval("calurl")[0] != '\0'; // if Cfg has a calender URL, the calendar must be loaded
  if( cal_tobe_loaded ) Serial.printf("cal : csv at %s\n", cfg.getval("calurl") ); else Serial.printf("cal : no URL\n");
  caldays = String(cfg.getval("caldays")).toInt();
  calmin = String(cfg.getval("calmin")).toInt();
  if( calmin<0 ) calmin = 1;
  Serial.printf("cal : every %d minutes, max %d days\n", calmin, caldays);
  
  // App starts running
  Serial.printf("\n");
}


// Record last received seconds, to blink colon
int       colon_prev_sec = -1;
uint32_t  colon_msecs;


// Showing time or date
#define   MODE_TIME    1
#define   MODE_DATE    2
#define   MODE_BDAYS   3
int       mode_tag = MODE_TIME; // one of MODE_XXX, what to display
String    mode_bdays;      // The string (with bdays) to show when mode_tag==MODE_BDAYS
int       mode_step;       // Character index into mode_bdays for scrolling
uint32_t  mode_stepms;     // time stamp fro scrolling
#define   MODE_STEP_MS 500 // Scroll time for one step
bool      flag_bdays_avail = false;

void loop() {
  // If in config mode, do config loop (when config completes, it restarts the device)
  if( cfg.cfgmode() ) { cfg.loop(); return; }

  // In normal application mode
  led_set( !wifi_isconnected() );     // LED is on when not connected

  // Check buttons
  but_scan();
  if( but_wentdown(BUT3) ) disp_brightness_set( disp_brightness_get()%8 + 1 );
  if( but_wentdown(BUT2) ) mode_tag = mode_tag==MODE_DATE ? MODE_TIME : MODE_DATE; 
  if( but_wentdown(BUT1) ) cal_tobe_loaded = true; // Explicit request by user to load the calendar

  // Get time in parts
  time_t      tnow= time(NULL);       // Returns seconds (and writes to the passed pointer, when not NULL) - note `time_t` is just a `long`.
  struct tm * snow= localtime(&tnow); // Returns a struct with time fields (https://www.tutorialspoint.com/c_standard_library/c_function_localtime.htm)
  bool        sync= snow->tm_year>120;// We miss-use "old" time as indication of "time not yet set" (year is 1900 based)

  // If seconds changed: print to console, optionally reload call (midnight), optionally show bdays
  if( snow->tm_sec != colon_prev_sec ) {
    // In `snow` the `tm_year` field is 1900 based, `tm_mon` is 0 based, rest is as expected
    Serial.printf("main: %d-%02d-%02d %02d:%02d:%02d (dst=%d) %s\n", snow->tm_year + 1900, snow->tm_mon + 1, snow->tm_mday, snow->tm_hour, snow->tm_min, snow->tm_sec, snow->tm_isdst, sync?"":"NO NTP" );
    // Record that seconds changed, for colon
    colon_prev_sec = snow->tm_sec; 
    colon_msecs = millis();
    // Reload cal every midnight
    if( (snow->tm_hour==0) && (snow->tm_min==0) && (snow->tm_sec==0) ) cal_tobe_loaded = true;
    // Show cal every calmin minutes
    if( flag_bdays_avail && (mode_tag!=MODE_BDAYS ) && (snow->tm_sec==0) && (snow->tm_min % calmin == 0) ) {
      mode_tag = MODE_BDAYS;
      mode_step = 0;
      mode_stepms = millis()-MODE_STEP_MS;
    }
  }

  if( sync ) {
    // Update the display
    switch( mode_tag ) {
      case MODE_TIME: {
        bool pm = snow->tm_hour >= 12;
        int  hr = snow->tm_hour % render_hours_len;
        char buf[5];
        sprintf(buf,"%2d%02d", hr, snow->tm_min );
        int dots = millis()-colon_msecs<500 ? DISP_DOTNO : DISP_DOTCOLON;
        if( render_hours_flag==RENDER_HOURS_FLAG_AM && !pm ) dots |= DISP_DOT1;
        if( render_hours_flag==RENDER_HOURS_FLAG_PM &&  pm ) dots |= DISP_DOT1;
        if( flag_bdays_avail ) dots |= DISP_DOT4;
        disp_show(buf,dots);
        break; 
      }
      case MODE_DATE: {
        char buf[5];
        if( render_dayfirst ) 
          sprintf(buf,"%2d%c%c", snow->tm_mday, render_months[snow->tm_mon*2], render_months[snow->tm_mon*2+1] );
        else
          sprintf(buf,"%c%c%2d", render_months[snow->tm_mon*2], render_months[snow->tm_mon*2+1], snow->tm_mday );
        int dots = flag_bdays_avail ? DISP_DOT4 : DISP_DOTNO;
        disp_show(buf,dots);
        break; 
      }
      case MODE_BDAYS: {
        if( millis()-mode_stepms > MODE_STEP_MS ) {
          if( mode_step==0 ) Serial.printf("cal : show '%s'\n",mode_bdays.c_str() );
          char buf[5];
          buf[0]= mode_bdays[mode_step];        
          buf[1]= mode_bdays[mode_step+1];
          buf[2]= mode_bdays[mode_step+2];
          buf[3]= mode_bdays[mode_step+3];
          buf[4]='\0';
          disp_show(buf);
          // Serial.printf("cal : %s\n",buf);
          mode_step++;
          mode_stepms = millis();
          if( mode_step+5 > mode_bdays.length() ) mode_tag = MODE_TIME;
        }
        break;
      }
    }

    // Get calendar
    if( cal_tobe_loaded ) {
      int error = cal_load( cfg.getval("calurl") );
      if( error<0 ) {
        Serial.printf("cal : load error %d\n",error);
        mode_bdays = String("Error ") + error + " lOAd";
      } else if( error>0 ){
        Serial.printf("cal : file error %d\n",error);
        mode_bdays = String("Error ") + (error%10) + " lINE " + (error/10);
      } else if( cal_size==0 ) {
        Serial.printf("cal : empty\n");
        mode_bdays = "Error no RECS";
      } else {
        int today = cal_daynum( snow->tm_mon+1,snow->tm_mday);
        int ix1 = cal_findfirst(snow->tm_mon+1,snow->tm_mday);
        // Serial.printf("cal : first bday %d/%s %04d-%02d-%02d\n",ix1,cal_label(ix1),cal_year(ix1),cal_month(ix1),cal_day(ix1));
        int ix=ix1;
        mode_bdays = "";
        while( 1 ) {
          int daynum = cal_daynum( cal_month(ix), cal_day(ix) );
          Serial.printf("cal : bday in %d days %s %04d-%02d-%02d\n",daynum-today,cal_label(ix).c_str(),cal_year(ix),cal_month(ix),cal_day(ix));
          if( daynum < today+caldays ) {
            int age = snow->tm_year + 1900 - cal_year(ix);
            if( ix<ix1 ) age++;
            if( mode_bdays!="" ) mode_bdays += "  -  ";
            mode_bdays = mode_bdays+(daynum-today)+" "+cal_label(ix)+" "+age;
            flag_bdays_avail = true;
          }
          ix = (ix+1)%cal_size(); // with wrap around
          if( ix==0 ) today -=365; // add one year (by antidating today)
          if( ix==ix1 ) break; // stop if we are at begin
        }
        if( mode_bdays=="" ) mode_bdays = "no-bdays"; 
        Serial.printf("cal : bdays %s\n",mode_bdays.c_str());
      }
      // Prepare for scroll  
      mode_tag = MODE_BDAYS;
      mode_bdays = "    " + mode_bdays+"     "; // to start and end the display clean (one extra at end)
      mode_step = 0;
      mode_stepms = millis()-MODE_STEP_MS;
      cal_tobe_loaded = false;
    }
  }

  // No delay - run at full speed
}
