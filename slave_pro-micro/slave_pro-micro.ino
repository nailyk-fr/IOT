
//#define FAILURE_HANDLING


#define CE_PIN 9
#define CSN_PIN 10
#define RSTPIN 8

/*
* Getting Started example sketch for nRF24L01+ radios
* This is a very basic example of how to send data from one node to another
* Updated: Dec 2014 by TMRh20
*/
#include <SPI.h>

#include <nRF24L01.h>
#include <RF24_config.h>
#include <RF24.h>

/****************** User Config ***************************/
/***      Set this radio as radio number 0 or 1         ***/
bool radioNumber = 1;

/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
RF24 radio(CE_PIN, CSN_PIN);
/**********************************************************/


byte addresses[][6] = {"1Node","2Node"};


void setup() {
  digitalWrite(RSTPIN, HIGH);
  pinMode(RSTPIN, OUTPUT);
  
  Serial.begin(115200);
  Serial.println(F("RF24/examples/GettingStarted"));
  Serial.println(F("*** PRESS 'T' to begin transmitting to the other node"));


  
  radio.begin();

  Serial.println(F("********** Radio init done **********"));



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



  if (radio.isChipConnected()) {
    Serial.println(F("*******************************"));
    Serial.println(F("********** CONNECTED **********"));
    Serial.println(F("*******************************"));
  }else {
    Serial.println(F("*******************************"));
    Serial.println(F(" connection failure with modem "));
    Serial.println(F("*******************************"));
  }

  
  // Start the radio listening for data
  radio.startListening();


  Serial.println(F("********** Init done **********"));


}



void loop() {

      
      radio.stopListening();                                    // First, stop listening so we can talk.
      
      
      Serial.println(F("Now sending"));
      unsigned long start_time = micros();                             // Take the time, and send it.  This will block until complete
       if (!radio.write( &start_time, sizeof(unsigned long) )){
         Serial.println(F("failed"));
       }
          
      radio.startListening();                                    // Now, continue listening
      
      unsigned long started_waiting_at = micros();               // Set up a timeout period, get the current microseconds
      boolean timeout = false;                                   // Set up a variable to indicate if a response was received or not
      
      while ( ! radio.available() ){                             // While nothing is received
        if (micros() - started_waiting_at > 200000 ){            // If waited longer than 200ms, indicate timeout and exit while loop
            timeout = true;
            break;
        }      
      }
          
      if ( timeout ){                                             // Describe the results
          Serial.println(F("Failed, response timed out."));
      }else{
          unsigned long got_time;                                 // Grab the response, compare, and send to debugging spew
          radio.read( &got_time, sizeof(unsigned long) );
          unsigned long end_time = micros();
          
          // Spew it
          Serial.print(F("Sent "));
          Serial.print(start_time);
          Serial.print(F(", Got response "));
          Serial.print(got_time);
          Serial.print(F(", Round-trip delay "));
          Serial.print(end_time-start_time);
          Serial.println(F(" microseconds"));
      }
      // Try again 1s later
      delay(1000);


  /****************** Change Roles via Serial Commands ***************************/
  if ( Serial.available() )
  {
      char c = toupper(Serial.read());
      Serial.print(c);


      if ( c == 'S'){
        Serial.println(F("*** resetting in ***"));      
        Serial.print(F("3..."));      
        delay(500);
        Serial.print(F("2..."));      
        delay(500);
        Serial.print(F("1..."));      
        delay(500);
        Serial.println(F("go!"));      
        software_Reset();  //call reset
         
      }
  }
} // Loop

void software_Reset() // Restarts program from beginning but does not reset the peripherals and registers
{
     digitalWrite(RSTPIN, LOW);
}  
