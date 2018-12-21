
#define FAILURE_HANDLING

//#define ARDUINO_ARCH_ESP8266


#define CE_PIN 2
#define CSN_PIN 15

#include <nRF24L01.h>
#include <RF24_config.h>
#include <RF24.h>


/****************** User Config ***************************/
/***      Set this radio as radio number 0 or 1         ***/
bool radioNumber = 0;

/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
RF24 radio(CE_PIN, CSN_PIN);
/**********************************************************/


byte addresses[][6] = {"1Node","2Node"};


/*******************************************
 *             setup_rf24
 *******************************************/
void setup_rf24(){
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

  Serial.println(F("********** Radio init done **********"));

}

/*******************************************
 *             setup_rf24
 *******************************************/
void rf24_read(unsigned long * data) {
   if (radio.isChipConnected()) {
    if( radio.available()){
                                                                    // Variable for the received timestamp
      while (radio.available()) {                                   // While there is data ready
        radio.read( data, sizeof(unsigned long) );             // Get the payload
      }
      
      radio.stopListening();                                        // First, stop listening so we can talk   
      radio.write( data, sizeof(unsigned long) );              // Send the final one back.      
      radio.startListening();                                       // Now, resume listening so we catch the next packets.     
      Serial.print(F("Sent response "));
      Serial.println(*data);  
    }
  }
}