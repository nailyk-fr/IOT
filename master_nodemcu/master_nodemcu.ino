
#include "include/lcd.cpp"

// Number of max connected nodes
#define MAXNODES 3

// Number of retries per node before stopping displaying it (will still be queried)
#define MAXRETRIES 10

// value used to autodetect if a sensors is connected. This value needs to be something which will never happend
#define NOSENSOR (-999) // needs to be a float, two decimal rounded

// Used to store text to display on LCD & serial
char value[256]; 

// Transitional value to store nodes sensors
RFDATA temp; 

// Store each node sensors values
float values[MAXNODES*2];

 // Count number of retries per nodes (sensors independant)
unsigned char retries[MAXNODES]; 

void setup() {

  setup_rf24();
  setup_lcd();

  lcd_line2("almost done");

  setup_wifintp();

  Serial.println(F("********** Init done **********"));
  lcd_line2("initialised! ");
  
  std::fill_n(values, MAXNODES*2,NOSENSOR);
  std::fill_n(retries, MAXNODES, MAXRETRIES);
}


void loop() {
  /****************** Pong Back Role ***************************/
  RFDATA * temp_ptr = &temp;

  for(int i=0; i<MAXNODES;i++){
    // Increase retry counter for node tested by for loop
    retries[i] = (retries[i]) +1;

    // Read data & retrieve node number
    unsigned char num = rf24_read(temp_ptr) -1;

    // if there was a receive, store it on the right position
    if (num != (DUMMYVALUE-1)) {
      values[num+(temp.addr)] = temp.rffloat.value;
      retries[num] = 0; 
      sprintf(value, "Node %i: %0.2f in [%i]", num, values[num+(temp.addr)], num+(temp.addr)); 
      Serial.println(value);
    }

    // ensure retries counter does not loop
    if ( retries[i] > MAXRETRIES ) {
      retries[i] = MAXRETRIES;
    }
  }
  
  if ( WiFi.status() == WL_CONNECTED ) {
    ntp_client(); 
  }

  display.clearDisplay();
  lcd_ntp();
  lcd_wifi();
  lcd_modem();

  // if node has been received recently display it
  if (retries[0] < MAXRETRIES) {
    // if node have two sensors, display both
    if ( (int)values[1] != (int)NOSENSOR) {
      sprintf(value, "1: %0.2f / %0.2f'C", values[0], values[1]); 
    } else {
      sprintf(value, "1: %0.2f 'C", values[0]); 
    }
    lcd_line1(value);
  }
  if (retries[1] < MAXRETRIES) {
    // if node have two sensors, display both
    if ( (int)values[3] != (int)NOSENSOR) {
      sprintf(value, "2: %0.2f / %0.2f'C", values[2], values[3]); 
    } else {
      sprintf(value, "2: %0.2f 'C", values[2]); 
    }
    lcd_line2(value);
  }
  if (retries[2] < MAXRETRIES) {
    // if node have two sensors, display both
    if ( (int)values[5] != (int)NOSENSOR) {
      sprintf(value, "3: %0.2f / %0.2f'C", values[4], values[5]); 
    } else {
      sprintf(value, "3: %0.2f 'C", values[4]); 
    }
    lcd_line3(value);
  }
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
