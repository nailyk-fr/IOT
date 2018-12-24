
#define FAILURE_HANDLING

//#define ARDUINO_ARCH_ESP8266


#define CE_PIN 2
#define CSN_PIN 15

#include <nRF24L01.h>
#include <RF24_config.h>
#include <RF24.h>

#include "../type.h"

/****************** User Config ***************************/

/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
RF24 radio(CE_PIN, CSN_PIN);
/**********************************************************/

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

  radio.setDataRate(RFSPEED);
  radio.setCRCLength(CRCLENGTH);
  radio.setRetries(15, 15);
  radio.setAutoAck(ACKENABLED);
  radio.setPayloadSize(PAYLOADSIZE);

  // Set the PA Level low to prevent power supply related issues since this is a
  // getting_started sketch, and the likelihood of close proximity of the devices. RF24_PA_MAX is default.
  radio.setPALevel(RFPOWER);

  
  // Open a writing and reading pipe on each radio, with opposite addresses
  radio.openWritingPipe(addresses[0]);
  radio.openReadingPipe(1,addresses[1]);
  radio.openReadingPipe(2,addresses[2]);
  radio.openReadingPipe(3,addresses[3]);


  // Start the radio listening for data
  radio.startListening();

  Serial.println(F("********** Radio init done **********"));

}

/*******************************************
 *             setup_rf24
 *******************************************/
unsigned char rf24_read(RFDATA * data) {
   uint8_t nodeNum = DUMMYVALUE; 
   char rx_bytes[sizeof(data->addr) + sizeof(data->rffloat.bytes)];

   char debug[64];

   if (radio.isChipConnected()) {
      if( radio.available()){
        while (radio.available(&nodeNum)) {                  // While there is data ready
          radio.read( rx_bytes, sizeof(rx_bytes) );            // Get the payload
        }

        memcpy(& data->addr, rx_bytes, sizeof(data->addr));
        memcpy(& data->rffloat.bytes, rx_bytes + sizeof(data->addr), sizeof(data->rffloat.bytes));

        sprintf(debug, "Readed %i bytes from node %i -> addr: %i , value: %0.8f", 
                       sizeof(rx_bytes), nodeNum, data->addr, data->rffloat.value); 
        Serial.println(debug);


       // ACK
       unsigned long toto = DUMMYVALUE;
       radio.stopListening();                                        // First, stop listening so we can talk   
       radio.write( &toto, sizeof(toto) );              // Send the final one back.      
       radio.startListening();                                       // Now, resume listening so we catch the next packets.     
       Serial.println(F("Sent done "));
     }
  }

  return nodeNum;
}
