
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

RFDATA temp; 


void loop() {
  /****************** Pong Back Role ***************************/
RFDATA * temp_ptr = &temp;

  rf24_read(temp_ptr);

  if ( WiFi.status() == WL_CONNECTED ) {
    ntp_client(); 
  }


  display.clearDisplay();
  lcd_ntp();
  lcd_wifi();
  lcd_modem();
  sprintf(value, "value1: %02X", temp.rffloat.value); 
  lcd_line1(value);
  sprintf(value, "value2: %f", temp.rffloat.value); 
  lcd_line2(value);
  sprintf(value, "value3: %0.2f 'C", temp.rffloat.value); 
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

} // Loop
