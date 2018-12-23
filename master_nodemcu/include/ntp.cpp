
// Libraries for internet time
// NTP client: https://simple-circuit.com/nodemcu-internet-clock-weather-station/
#include <WiFiUdp.h>
#include <NTPClient.h>          // include NTPClient library
#include <TimeLib.h>            // include Arduino time library
 
// Libraries for internet weather
#include <ESP8266HTTPClient.h>  // http web access library
#include "wifi.h"

#include <ESP8266WiFi.h>

// include munin server
#include "munin.cpp"

/******************** NTP ********************/
WiFiUDP ntpUDP;
// 'time.nist.gov' is used (default server) with +1 hour offset (3600 seconds) 60 seconds (60000 milliseconds) update interval
NTPClient timeClient(ntpUDP, "pool.ntp.org", 3600, 60000);
 

char Time[] = "  :  :  ";
char Date[] = "  -  -20  ";
byte second_, minute_, hour_, wday, day_, month_, year_;

/*******************************************
 *             setup_ntp
 *******************************************/

void setup_wifintp() {
  WiFi.begin(ssid, password);

  timeClient.begin();
  
}

/*******************************************
 *             ntp_client
 *******************************************/
void ntp_client(){
  timeClient.update();
  unsigned long unix_epoch = timeClient.getEpochTime();   // get UNIX Epoch time
  
  second_ = second(unix_epoch);        // get seconds from the UNIX Epoch time
  minute_ = minute(unix_epoch);      // get minutes (0 - 59)
  hour_   = hour(unix_epoch);        // get hours   (0 - 23)
  wday    = weekday(unix_epoch);     // get minutes (1 - 7 with Sunday is day 1)
  day_    = day(unix_epoch);         // get month day (1 - 31, depends on month)
  month_  = month(unix_epoch);       // get month (1 - 12 with Jan is month 1)
  year_   = year(unix_epoch) - 2000; // get year with 4 digits - 2000 results 2 digits year (ex: 2018 --> 18)
  
  Time[7] = second_ % 10 + '0';
  Time[6] = second_ / 10 + '0';
  Time[4] = minute_ % 10 + '0';
  Time[3] = minute_ / 10 + '0';
  Time[1] = hour_   % 10 + '0';
  Time[0] = hour_   / 10 + '0';
  Date[9] = year_   % 10 + '0';
  Date[8] = year_   / 10 + '0';
  Date[4] = month_  % 10 + '0';
  Date[3] = month_  / 10 + '0';
  Date[1] = day_    % 10 + '0';
  Date[0] = day_    / 10 + '0';
}
