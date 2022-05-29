// cal.h - interface to get calendar information from a google spreadsheet
#ifndef _CAL_H_
#define _CAL_H_


// Maximum number of calendar entries
#define CAL_SIZE 100


// Actual size (number of calendar entries) of the calendar
int    cal_size();


// For calendar entry `ix`, 0<ix<cal_size(), the label, year, month, day
String cal_label(int ix);
int    cal_year(int ix);
int    cal_month(int ix);
int    cal_day(int ix);


// Returns days since Jan 1.
int    cal_daynum(int month, int day );


// Returns index of largest record that is today (mont/day passed) or later
int    cal_findfirst(int month, int day);


// Load the calendar from the `url`.
// URL shall point to a CSV file of the form
//   mr,1978-10-17\r\n
//   annie,2002-07-02\r\n
//   boris,1999-02-04
// If 0 is returned, load was successful, and the data is available via cal_size(),cal_label(),cal_year(),cal_month,cal_day().
// Otherwise there was an error:
// - Negative values close to 0 are load errors
//     HTTPC_ERROR_CONNECTION_FAILED   (-1)
//     HTTPC_ERROR_SEND_HEADER_FAILED  (-2)
//     HTTPC_ERROR_SEND_PAYLOAD_FAILED (-3)
//     HTTPC_ERROR_NOT_CONNECTED       (-4)
//     HTTPC_ERROR_CONNECTION_LOST     (-5)
//     HTTPC_ERROR_NO_STREAM           (-6)
//     HTTPC_ERROR_NO_HTTP_SERVER      (-7)
//     HTTPC_ERROR_TOO_LESS_RAM        (-8)
//     HTTPC_ERROR_ENCODING            (-9)
//     HTTPC_ERROR_STREAM_WRITE        (-10)
//     HTTPC_ERROR_READ_TIMEOUT        (-11)
//     CAL_ERROR_UNEXPECTED            (-50)
//     CAL_ERROR_BEGIN                 (-51)
//     CAL_EMPTY                       (-52)
// - Negative values of three digits are http errors (with a minus sign) - print with https.errorToString(code)
//     Informational responses         (-100 .. -199)
//     Successful responses            (-200 .. -299)
//     Redirection messages            (-300 .. -399)
//     Client error responses          (-400 .. -499)
//     Server error responses          (-500 .. -599)
// - Positive errors parse errors of the downloaded file.
//   Least significant digit is problem code, rest is the line number in the file.
//     missing comma in record         (1)
//     missing name in record          (2)
//     date is not 10 long             (3) (or extra column on some record, giving all record including the first an extra column)
//     year-month dash missing         (4)
//     month-day dash missing          (5)
//     year out of range               (6)
//     month out of range              (7)
//     day out of range                (8)
//     out of space (SIZE param)       (9)
int cal_load(const char * url);

#define CAL_ERROR_UNEXPECTED           (-50)
#define CAL_ERROR_BEGIN                (-51)
#define CAL_ERROR_BEGIN_REDIRECT       (-52)
#define CAL_EMPTY                      (-53)


void cal_init();


#endif
