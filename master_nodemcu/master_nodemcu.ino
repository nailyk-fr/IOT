
#include "include/lcd.cpp"



void setup() {

  setup_rf24();
  setup_lcd();

  simple_output("almost done");

  setup_wifintp();

  Serial.println(F("********** Init done **********"));
  simple_output("initialised! ");

}

char value[64]; 

unsigned long got_time = 127;

void loop() {
  /****************** Pong Back Role ***************************/
  

  rf24_read(&got_time);
      
  sprintf(value, "value: %i", got_time); 

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
