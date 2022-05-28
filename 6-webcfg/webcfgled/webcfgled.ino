// webcfgled.ino - use a google spreadsheet to configure how often the LED should flash
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>


// ===== WebCfg =========================================================================
// download config file from web server


// Make a spreadsgeet in google sheets (this app use only cell A1 from the first tab)
//   Note: you must share the file for public viewing (everybody with the link can then read it)  
// Open the spreadsheet in google sheets, then open the correct tab (first in this demo), and look at the url when the tab is open  
//   https://docs.google.com/spreadsheets/d/1mcsbrp1kGm0c29l3nXLqQ833ojlBLRUvd5v7cRqZNAw/edit#gid=0
// The long number is the "file id"  
//   1mcsbrp1kGm0c29l3nXLqQ833ojlBLRUvd5v7cRqZNAw
// The short number is the "tab id" (first tab is always 0, next tabs have a 9 (?) digit number.
//   0
//To URL to the tab in CSV format is then 
//   https://docs.google.com/spreadsheets/d/1mcsbrp1kGm0c29l3nXLqQ833ojlBLRUvd5v7cRqZNAw/export?format=csv&gid=0


#define WEBCFG_URL  "https://docs.google.com/spreadsheets/d/1mcsbrp1kGm0c29l3nXLqQ833ojlBLRUvd5v7cRqZNAw/export?format=csv&gid=0"


String webcfg_getfile(const char * url, String & filecontent) {
  // Function either returns "" (ok case, `filecontent` has content)
  // or returns a non-empty error string (and filecontent is "").
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
    return "unable to begin";
  } 

  // Get the response
  int httpCode = https.GET();

  // GET failed?
  if( httpCode<0 ) {
    https.end();
    return String("code ")+httpCode+" "+https.errorToString(httpCode);
  }

  // GET successful (server with no redirect)
  if( httpCode == HTTP_CODE_OK ) {
    filecontent = https.getString();
    https.end();
    return "";
  } 

  // GET with redirect (as google docs does)
  if( httpCode==HTTP_CODE_TEMPORARY_REDIRECT ) {
    // Extract the temporary location (url)
    String location = https.header("location");
    // New https request
    https.end();
    ok = https.begin(*client, location);
    if( !ok ) {
      return "unable to begin redirect";
    }
    httpCode = https.GET();
    if( httpCode == HTTP_CODE_OK  ) {
      filecontent = https.getString();
      https.end();
      return "";
    }
    https.end();
    return String("redirect code ")+httpCode+" "+https.errorToString(httpCode);
  }

  // Still here?
  https.end();
  return String("can't handle code ")+httpCode;
}


// ===== LED ==========================================================================
// Driver for the built-in LED - used for simple signalling to the user


#define LED_PIN LED_BUILTIN


// Switches the signalling LED on the ESP8266 board off.
void led_off() {
  digitalWrite(LED_PIN, HIGH); // low active
}


// Switches the signalling LED on the ESP8266 board on.
void led_on() {
  digitalWrite(LED_PIN, LOW); // low active
}


// Initializes the LED driver.
// Configures the GPIO block for the LED pin.
void led_init() {
  pinMode(LED_PIN, OUTPUT);
  led_off();
  Serial.printf("led   : init\n");
}


// ===== WIFI ===========================================================================


#define WIFI_SSID   "GuestFamPennings"
#define WIFI_PASSWD "there_is_no_password"


void wifi_init() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWD);
  Serial.printf("wifi  : connecting ...");
  while( WiFi.status()!=WL_CONNECTED ) { 
    led_on(); 
    delay(300); 
    led_off(); 
    delay(300); 
    Serial.printf("."); 
  } 
  Serial.printf("\n");
  Serial.printf("wifi  : connected %s\n",WiFi.localIP().toString().c_str() );  
}


// ===== APP ============================================================================


void setup() {
  Serial.begin(115200);
  do delay(500); while( !Serial );
  Serial.printf("\n\n\n\n");
  Serial.printf("Welcome to webcfgled\n\n");

  led_init();
  wifi_init();

  Serial.printf("\n");
}


void loop() {
  String content;
  String error = webcfg_getfile(WEBCFG_URL,content);
  if( error!="" ) {
    Serial.printf("ERROR %s\n", error.c_str() );
    // 5 quick flashes signals error
    for( int i=0; i<5; i++ ) {
      led_on();
      delay(50);
      led_off();
      delay(50);
    }
  } else {
    int num = content.toInt();
    Serial.printf("webcfg: %d\n",num);
    // First a quick pulse to show successful read
    led_on();
    delay(50);
    led_off();
    delay(500);
    for( int i=0; i<num; i++ ) {
      led_on();
      delay(500);
      led_off();
      delay(500);
    }
  }
  delay(10*1000);
}
