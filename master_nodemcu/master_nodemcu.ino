
#include "include/lcd.cpp"



void setup() {

  setup_rf24();
  setup_lcd();

  lcd_line2("almost done");

  setup_wifintp();

  Serial.println(F("********** Init done **********"));
  lcd_line2("initialised! ");

}

char value[64]; 

float got_time;

void loop() {
  /****************** Pong Back Role ***************************/

  rf24_read(&got_time);

  if ( WiFi.status() == WL_CONNECTED ) {
    ntp_client(); 
  }

          char debug[64];
          sprintf(debug, "%0.2f 'C", got_time); 
                    Serial.print("received: ");

          Serial.print(debug);
          Serial.println();

  display.clearDisplay();
  lcd_ntp();
  lcd_wifi();
  lcd_modem();
  sprintf(value, "value1: %02X", got_time); 
  lcd_line1(value);
  sprintf(value, "value2: %f", got_time); 
  lcd_line2(value);
  sprintf(value, "value3: %0.2f °C", got_time); 
  lcd_line3(value);

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
