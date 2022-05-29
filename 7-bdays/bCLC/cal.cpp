// cal.cpp - get calendar information from a google spreadsheet
#include <Arduino.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include "cal.h"


#define CAL_INCLUDE_TEST  0


typedef struct cal_s {
  String label;
  int    year;
  int    month;
  int    day;
} cal_t;
#define       CAL_SIZE 100
static cal_t  cal_list[CAL_SIZE];
static int    cal_size_act;



int    cal_size() {
  return cal_size_act;
}


String cal_label(int ix) {
  if( ix<0 || ix>=CAL_SIZE ) return "----";
  return cal_list[ix].label;  
}


int    cal_year(int ix) {
  if( ix<0 || ix>=CAL_SIZE ) return 1;
  return cal_list[ix].year;  
}


int    cal_month(int ix) {
  if( ix<0 || ix>=CAL_SIZE ) return 1;
  return cal_list[ix].month;  
}


int    cal_day(int ix) {
  if( ix<0 || ix>=CAL_SIZE ) return 1;
  return cal_list[ix].day;  
}


static int cal_days_in_month(int month ) {
  // We assume feb to have 29, because that happens now and then (and the range check should not fail).
  static int num[12] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
  if( 1<=month && month<=12 ) return num[month-1]; else return 0;
}


// Returns days since Jan 1.
int cal_daynum(int month, int day ) {
  // We assume feb to have 28 so that future events appear closer
  static int num[13] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 };
  return num[month-1]+day;
}


// Returns index of largest record that is today (mont/day passed) or later
int    cal_findfirst(int month, int day) {
  int today = cal_daynum(month,day);
  int ix = 0;
  while( ix<cal_size_act && cal_daynum(cal_list[ix].month,cal_list[ix].day)<today ) ix++;
  if( ix==cal_size_act ) ix = 0; // wrap around
  return ix;
}


static int cal_getfile(const char * url, String & filecontent) {
  // Function either returns 0 (ok case, `filecontent` has content)
  // or returns an error code (and filecontent is "")
  filecontent = "";
  
  // We need to specify which headers we want to look at (only: location for redirect)
  static const char * headerkeys[] = {"location"} ;
  static const size_t headercount = sizeof(headerkeys)/sizeof(const char *);

  // We need an SSL client, because google sheets uses https (but we don't care about the server fingerprint)
  std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
  client->setInsecure(); // happy to ignore the SSL certificate, else client->setFingerprint(uint8_t[20]);
  HTTPClient https;

  // Setup the request
  bool ok = https.begin(*client, url);
  https.collectHeaders(headerkeys,headercount);
  if( !ok ) {
    Serial.printf("ERROR cal unable to begin\n");
    return CAL_ERROR_BEGIN;
  } 

  // Get the response
  int httpCode = https.GET();

  // GET failed?
  if( httpCode<0 ) {
    https.end();
    Serial.printf("ERROR cal load '%s' (%d)\n", https.errorToString(httpCode).c_str(), httpCode );
    return httpCode; 
  }

  // GET successful
  if( httpCode == HTTP_CODE_OK ) {
    filecontent = https.getString();
    https.end();
    return 0;
  } 

  // Not redirect, bail out
  if( httpCode!=HTTP_CODE_TEMPORARY_REDIRECT ) {
    // Don't know how to handle
    https.end();
    Serial.printf("ERROR cal http %d\n", httpCode );
    return -httpCode; // Make negative (positives are for parsing)
  }

  // Extract the temporary location (url) for the redirect
  String location = https.header("location");
  // New https request
  https.end();
  ok = https.begin(*client, location);
  if( !ok ) {
    Serial.printf("ERROR cal unable to begin redirect\n");
    return CAL_ERROR_BEGIN;
  }
  httpCode = https.GET();
  if( httpCode == HTTP_CODE_OK  ) {
    filecontent = https.getString();
    https.end();
    return 0;
  }
  https.end();
  Serial.printf("ERROR cal redirect http %d\n", httpCode );
  return -httpCode; // Make negative (positives are for parsing), 1000 lower (to show this is for redirect)
}


// qsort requires you to create a sort function
static int cal_lt(const void *cmp1, const void *cmp2) {
  // Need to cast the void * to int *
  cal_t * a = (cal_t *)cmp1;
  cal_t * b = (cal_t *)cmp2;
  // The comparison
  if( a->month > b->month  ) return +1;
  if( a->month < b->month  ) return -1;
  if( a->day   > b->day    ) return +1;
  if( a->day   < b->day    ) return -1;
  // We don't care about year, but anyhow let's put oldest first
  if( a->year  > b->year   ) return +1;
  if( a->year  < b->year   ) return -1;
  return 0;
}

// 0 for no errors (added to cal_list_cal_size_act), otherwise error code
static int cal_parseline(String content, int from, int till) {
  // Last line is empty is ok
  if( from==till ) return 0;
  // find comma
  int pos = content.indexOf(',',from);
  //Serial.printf("\n'%s' #=%d %d..%d, ix=%d, /,/%d\n",content.c_str(),content.length(),from,till,ix, pos);
  if( pos == -1 ) return 1; // missing comma in record
  if( pos == 0 ) return 2; // missing name in record
  if( pos+11 != till ) return 3; // date is not 10 long (or extra column on some record, giving all record including the first an extra column)
  if( content[pos+5]!='-' ) return 4; // year-month dash missing
  if( content[pos+8]!='-' ) return 5; // month-day dash missing
  // Extract year
  int year = content[pos+1]-'0';
  year = year*10 + content[pos+2]-'0';
  year = year*10 + content[pos+3]-'0';
  year = year*10 + content[pos+4]-'0';
  if( year<1900 || year>2100 ) return 6; // year out of range
  // Extract month
  int month = content[pos+6]-'0';
  month = month*10 + content[pos+7]-'0';
  if( month<1 || month>12 ) return 7; // month out of range
  // Extract day
  int day = content[pos+9]-'0';
  day = day*10 + content[pos+10]-'0';
  if( day<1 || day>cal_days_in_month(month) ) return 8; // day out of range
  // Space
  if( cal_size_act==CAL_SIZE ) return 9; // out of space
  // Fill record
  cal_list[cal_size_act].label= content.substring(from,pos);
  cal_list[cal_size_act].year=year;
  cal_list[cal_size_act].month=month;
  cal_list[cal_size_act].day=day;
  cal_size_act = cal_size_act+1;
  return 0;
}


static int cal_parse(String content) {
  int from = 0;
  int pos;
  while( (pos=content.indexOf("\r\n",from)) >= 0 ) {
    int res = cal_parseline(content, from, pos);
    if( res>0 ) return res;
    from = pos+2;
  }
  // Last line typically has no CR LF
  return cal_parseline(content, from, content.length());
}


int cal_load(const char * url) {
  // Clear existing list
  cal_size_act = 0;
  
  // First download calendar
  String content;
  int error1 = cal_getfile(url,content);
  if( error1>0 ) { Serial.printf("cal : ERROR code expected to be negative (%d)",error1 ); return CAL_ERROR_UNEXPECTED; }
  if( error1!=0 ) return error1;
  
  // Next, parse the file content
  int error2 = cal_parse(content);
  // Sort even on error, so that the part till the error is usable
  qsort( cal_list, cal_size_act, sizeof(cal_list[0]), cal_lt );
  if( !( 0<=error2 && error2<10 ) ) { Serial.printf("cal : ERROR code expected to be 1..9 (%d)",error2 ); return CAL_ERROR_UNEXPECTED; }
  if( error2!=0 ) { int report = 10*(cal_size_act+1) + error2; Serial.printf("cal : record %d has error %d\n",cal_size_act+1,error2); return report; }

  // Do we have a calendar?
  if( cal_size_act==0 ) { Serial.printf("ERROR cal empty\n"); return CAL_EMPTY; }

  // Feedback
  Serial.printf("cal : loaded %d\n",cal_size_act);

  return 0;
}


#if CAL_INCLUDE_TEST
  static int cal_test(int id,String content,int expect,int xsize) {
    int actual = cal_load(content);
    int ok = (expect==actual) && (xsize==cal_size_act);
    Serial.printf("%3d %d=%d %d=%d %s\n",id,expect,actual,xsize,cal_size_act,ok?"ok":"FAIL");
    return !ok;
  }
  static void cal_tests() {
    Serial.printf("=== CAL TESTING BEGIN ===\n");
    int id=0;
    int error_count=0;
    error_count += cal_test(id++,"mike1;1978-10-17",1,0);
    error_count += cal_test(id++,",1978-10-17",2,0);
    error_count += cal_test(id++,"1,1978-10-17",0,1);
    error_count += cal_test(id++,"mike12,",3,0);
    error_count += cal_test(id++,"mike123,1960",3,0);
    error_count += cal_test(id++,"mike1234,1978-10-2",3,0);
    error_count += cal_test(id++,"mike12345,1978-10-17",0,1);
    error_count += cal_test(id++,"mike123456,1978/10-17",4,0);
    error_count += cal_test(id++,"mike1234567,1978-10/17",5,0);
    error_count += cal_test(id++,"mike12345678,1978-10-17\r",3,0); // \r is added to date
    error_count += cal_test(id++,"mr,1978-10-17\r\n",0,1);
    error_count += cal_test(id++,"mr,1978-10-17\r\nAnn,2002-07-02",0,2);
    error_count += cal_test(id++,"mr,1978-10-17\r\nannie,2002-07-02\r\n",0,2);
    error_count += cal_test(id++,"mr,1878-10-17\r\n",6,0);
    error_count += cal_test(id++,"mr,2178-10-17\r\n",6,0);
    error_count += cal_test(id++,"mr,1978-00-17\r\n",7,0);
    error_count += cal_test(id++,"mr,1978-13-17\r\n",7,0);
    error_count += cal_test(id++,"mr,1978-10-00\r\n",8,0);
    error_count += cal_test(id++,"mr,1978-10-32\r\n",8,0);
    error_count += cal_test(id++,"mr,1978-10-17\r\nannie,2002-07-02\r\nboris,1999-02-04",0,3);
    Serial.printf("Errors %d\n",error_count);
    Serial.printf("=== CAL TESTING END ===\n");
  }
#else
  #define cal_tests() (void)0
#endif


void cal_init() {
  cal_tests();
  Serial.printf("cal : init\n");
}
