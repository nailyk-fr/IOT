
#include "ntp.cpp"
#include "rf24.cpp"


#define SSD1306_128_32

// https://learn.adafruit.com/adafruit-gfx-graphics-library/graphics-primitives#bitmaps-3-32
// Image converter (awesome): http://javl.github.io/image2cpp/
#include "logo_wifi.c"
#include "logo_modem.c"

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET LED_BUILTIN  //4
Adafruit_SSD1306 display(OLED_RESET);


#if ! defined(SSD1306_LCDHEIGHT)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif


void setup_lcd(){
  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  // Clear the buffer.
  display.clearDisplay();
  Wire.setClock(400000);
  Serial.println(F("********** LCD init done **********"));
  
}

void simple_output(char * string) {
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,18);
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
