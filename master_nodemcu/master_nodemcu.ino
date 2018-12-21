
#define FAILURE_HANDLING

//#define ARDUINO_ARCH_ESP8266


#define CE_PIN 2
#define CSN_PIN 15


#define SSD1306_128_32

// Libraries for internet time
// NTP client: https://simple-circuit.com/nodemcu-internet-clock-weather-station/
#include <WiFiUdp.h>
#include <NTPClient.h>          // include NTPClient library
#include <TimeLib.h>            // include Arduino time library
 
// Libraries for internet weather
#include <ESP8266HTTPClient.h>  // http web access library
#include "wifi.h"

// https://learn.adafruit.com/adafruit-gfx-graphics-library/graphics-primitives#bitmaps-3-32
// Image converter (awesome): http://javl.github.io/image2cpp/
#include "include/logo_wifi.c"
#include "include/logo_modem.c"

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET LED_BUILTIN  //4
Adafruit_SSD1306 display(OLED_RESET);


#if ! defined(SSD1306_LCDHEIGHT)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

/*
* Getting Started example sketch for nRF24L01+ radios
* This is a very basic example of how to send data from one node to another
* Updated: Dec 2014 by TMRh20
*/

#include <nRF24L01.h>
#include <RF24_config.h>
#include <RF24.h>

#include <ESP8266WiFi.h>

/****************** User Config ***************************/
/***      Set this radio as radio number 0 or 1         ***/
bool radioNumber = 0;

/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
RF24 radio(CE_PIN, CSN_PIN);
/**********************************************************/


byte addresses[][6] = {"1Node","2Node"};

/******************** NTP ********************/
WiFiUDP ntpUDP;
// 'time.nist.gov' is used (default server) with +1 hour offset (3600 seconds) 60 seconds (60000 milliseconds) update interval
NTPClient timeClient(ntpUDP, "pool.ntp.org", 3600, 60000);
 

void setup() {
  Serial.begin(115200);
  Serial.println(F("RF24/examples/GettingStarted"));
  Serial.println(F("*** PRESS 'T' to begin transmitting to the other node"));

#if defined (ARDUINO_ARCH_ESP8266)
  /* 
   * On most ESP8266 there is a watchdog with a 1 sec value which cause issue with the delay loop in transmitting mode 
   * Let's change the watchdog default to something bigger than waiting delay. 
   */
  ESP.wdtDisable();
  ESP.wdtEnable(5000);
#endif
  
  radio.begin();

  Serial.println(F("********** Radio init done **********"));

  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  // Clear the buffer.
  display.clearDisplay();
  Wire.setClock(400000);
  Serial.println(F("********** LCD init done **********"));


/* 
 *  On some nRF24L chips, the AutoACK can cause issues. 
 *  Uncomment (on both transmitter & receiver) to workaround
 */
  //radio.setAutoAck(false);

  radio.setDataRate(RF24_250KBPS);
  radio.setCRCLength(RF24_CRC_8);
  radio.setRetries(15, 15);
  radio.setAutoAck(true);

  // Set the PA Level low to prevent power supply related issues since this is a
  // getting_started sketch, and the likelihood of close proximity of the devices. RF24_PA_MAX is default.
  radio.setPALevel(RF24_PA_LOW);

  
  // Open a writing and reading pipe on each radio, with opposite addresses
  if(radioNumber){
    radio.openWritingPipe(addresses[1]);
    radio.openReadingPipe(1,addresses[0]);
  }else{
    radio.openWritingPipe(addresses[0]);
    radio.openReadingPipe(1,addresses[1]);
  }


  Serial.print(F("adress set to "));
  Serial.println(*addresses[radioNumber]);


  
  // Start the radio listening for data
  radio.startListening();

  simple_output("almost done");

  WiFi.begin(ssid, password);

  timeClient.begin();


  Serial.println(F("********** Init done **********"));
  simple_output("initialised! ");

}

char Time[] = "  :  :  ";
char Date[] = "  -  -20  ";
byte second_, minute_, hour_, wday, day_, month_, year_;
char value[64]; 


void loop() {
  /****************** Pong Back Role ***************************/
  
  unsigned long got_time;

  if (radio.isChipConnected()) {
    if( radio.available()){
                                                                    // Variable for the received timestamp
      while (radio.available()) {                                   // While there is data ready
        radio.read( &got_time, sizeof(unsigned long) );             // Get the payload
      }
      
      radio.stopListening();                                        // First, stop listening so we can talk   
      radio.write( &got_time, sizeof(unsigned long) );              // Send the final one back.      
    
      sprintf(value, "value: %i", got_time); 
    
      radio.startListening();                                       // Now, resume listening so we catch the next packets.     
      Serial.print(F("Sent response "));
      Serial.println(got_time);  
    }
  }

  if ( WiFi.status() == WL_CONNECTED ) {
    ntp_client(); 
  }

  display.clearDisplay();
  lcd_ntp();
  lcd_wifi();
  lcd_modem();
  simple_output(value);
  display.display();


  /****************** Change Roles via Serial Commands ***************************/
  if ( Serial.available() )
  {
    char c = toupper(Serial.read());
    Serial.print(c);
  
    if (c == 'S') {
       system_restart();
    }
  }

  delay(250); 
} // Loop

void simple_output(char * string) {
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,12);
  display.println(string);
}

void lcd_wifi() {
  // arduino-1.8.8/libraries/WiFi/extras/wifiHD/src/wl_definitions.h:28
  if ( WiFi.status() == WL_CONNECTED )
  {
    display.drawBitmap( 128-WIFILOGO_W, 0, WIFILOGO_OK, WIFILOGO_W, WIFILOGO_H, WHITE);
  } else {
    display.drawBitmap( 128-WIFILOGO_W, 0, WIFILOGO_ERR, WIFILOGO_W, WIFILOGO_H, WHITE);
  }
}

void lcd_modem() {
  if (radio.isChipConnected()) {
    display.drawBitmap( 0, 0, MODEM_OK, MODEMLOGO_W, MODEMLOGO_H, WHITE);
  }else {
    display.drawBitmap( 0, 0, WIFILOGO_ERR, WIFILOGO_W, WIFILOGO_H, WHITE);
  }
}

void lcd_ntp(){
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 24);
  display.print(Time);        // display time (format: hh:mm:ss)
  display.setCursor(64, 24);
  display.print(Date);        // display date (format: dd-mm-yyyy)
}

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
