
//#define FAILURE_HANDLING

#include "Common.h"

#define CE_PIN 9
#define CSN_PIN 10
#define RSTPIN 8

#define DEBUG_RF // Uncomment to get some debug about nRF24L on serial

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
bool radioNumber = 2;

/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
RF24 radio(CE_PIN, CSN_PIN);
/**********************************************************/


/**********************************************************/
#define DS18B20PIN 7
/* Dépendance pour le bus 1-Wire */
#include <OneWire.h>
/* Création de l'objet OneWire pour manipuler le bus 1-Wire */
OneWire ds(DS18B20PIN);
/* Code de retour de la fonction getTemperature() */
enum DS18B20_RCODES {
  READ_OK,  // Lecture ok
  NO_SENSOR_FOUND,  // Pas de capteur
  INVALID_ADDRESS,  // Adresse reçue invalide
  INVALID_SENSOR  // Capteur invalide (pas un DS18B20)
};
/**********************************************************/

#include "type.h"

/*****************************************************
 * setup()
 ****************************************************/
void setup() {
  // Physical loop with reset to be able to remote restat the µc
  digitalWrite(RSTPIN, HIGH);
  pinMode(RSTPIN, OUTPUT);

  Serial.begin(115200);
  delay(1000); // wait 1s before sending. Should be enough to open serial. 
  
  radio.begin();

  Serial.println(F("********** Radio init done **********"));

  radio.setDataRate(RFSPEED);
  radio.setCRCLength(CRCLENGTH);
  radio.setRetries(15, 15);
  radio.setAutoAck(ACKENABLED);
  radio.setPayloadSize(PAYLOADSIZE);

  // Set the PA Level low to prevent power supply related issues since this is a
  // getting_started sketch, and the likelihood of close proximity of the devices. RF24_PA_MAX is default.
  radio.setPALevel(RFPOWER);

  // Open a writing and reading pipe on each radio, with opposite addresses
  radio.openWritingPipe(addresses[radioNumber+1]); // Shift one because it is already used by the master
  radio.openReadingPipe(1,addresses[0]);

  char text[64];
  sprintf(text, "adress set to %s", addresses[radioNumber]);
  Serial.println(text);

#ifdef DEBUG_RF
  if (radio.isChipConnected()) {
    Serial.println(F("*******************************"));
    Serial.println(F("********** CONNECTED **********"));
    Serial.println(F("*******************************"));
  }else {
    Serial.println(F("*******************************"));
    Serial.println(F(" connection failure with modem "));
    Serial.println(F("*******************************"));
  }
#endif
  
  // Start the radio listening for data
  radio.startListening();


  Serial.println(F("********** Init done **********"));

}

/*****************************************************
 * loop()
 ****************************************************/
void loop() {


  
      RFDATA temp;
      temp.addr.value = 0 + (radioNumber * 2); // Looks like there is an issue into the library to detect the pipe who sent the packets. Lets use the addr byte to worakround this.  
      
      /* Lit la température ambiante à ~1Hz */
      if (getTemperature(&temp.rffloat.value, true) != READ_OK) {
        Serial.println(F("Erreur de lecture du capteur"));
        return;
      }
      
      /* Affiche la température */
      char text[64];
      sprintf(text, "value %i: %s 'C", temp.addr.value, f2s(temp.rffloat.value,2)); 
      Serial.println(text);

      send(temp);

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

      // Try again 1s later
      delay(200);

} // Loop

/*****************************************************
 * send(RFDATA)
 ****************************************************/
void send (RFDATA data) {
      // Prepare a char array to copy values into to send them easily (mostly, getting rid of endianess issues)
      char tx_bytes[sizeof(data.addr.bytes) + sizeof(data.rffloat.bytes)];
      memcpy(tx_bytes, data.addr.bytes, sizeof(data.addr.bytes)); // Looks like there is an issue into the library to detect the pipe who sent the packets. Lets use the addr byte to worakround this. 
      memcpy(tx_bytes+sizeof(data.addr.bytes), data.rffloat.bytes, sizeof(data.rffloat.bytes));
      radio.stopListening();                                    // First, stop listening so we can talk.
      
#ifdef DEBUG_RF
      Serial.print(F("Sending... "));
#endif
       if (!radio.write( tx_bytes, sizeof(tx_bytes))){
#ifdef DEBUG_RF
         Serial.print(sizeof(tx_bytes));
         Serial.println(F(" bytes FAILED!"));
#endif
       } else {
#ifdef DEBUG_RF
        Serial.print("Sent ");
        Serial.print(sizeof(tx_bytes));
        Serial.println(" bytes");
#endif
       }
          
      radio.startListening();                                    // Now, continue listening
      
      unsigned long started_waiting_at = micros();               // Set up a timeout period, get the current microseconds
      boolean timeout = false;                                   // Set up a variable to indicate if a response was received or not
      
      while ( ! radio.available() ){                             // While nothing is received
        if (micros() - started_waiting_at > 800000 ){            // If waited longer than 200ms, indicate timeout and exit while loop
            timeout = true;
            break;
        }      
      }

// ACK
      if ( timeout ){                                             // Describe the results
          Serial.println(F("Timeout ACK from master."));
      }else{
          unsigned long answer;                                 // Grab the response, compare, and send to debugging spew
          radio.read( &answer, sizeof(answer) );
          unsigned long end_time = micros();
#ifdef DEBUG_RF
          // Spew it
          char text[64];
          sprintf(text, "Sent %i(0x%x):%s, got response %lu on %i bytes", data.addr.value, data.addr.value, f2s(data.rffloat.value,6), answer, sizeof(answer)); 
          Serial.println(text);
#endif
      }
}

void software_Reset() { // Restarts program from beginning but does not reset the peripherals and registers
     // Physical loop between RSTPIN and reset pin on the microcontroller. No need to pull-up. 
     digitalWrite(RSTPIN, LOW);
}

/**
 * Function to read the DS sensor
 */
byte getTemperature(float *temperature, byte reset_search) {
  byte data[9], addr[8];
  // data[] : Données lues depuis le scratchpad
  // addr[] : Adresse du module 1-Wire détecté
  
  /* Reset le bus 1-Wire ci nécessaire (requis pour la lecture du premier capteur) */
  if (reset_search) {
    ds.reset_search();
  }
 
  /* Recherche le prochain capteur 1-Wire disponible */
  if (!ds.search(addr)) {
    // Pas de capteur
    return NO_SENSOR_FOUND;
  }
  
  /* Vérifie que l'adresse a été correctement reçue */
  if (OneWire::crc8(addr, 7) != addr[7]) {
    // Adresse invalide
    return INVALID_ADDRESS;
  }
 
  /* Vérifie qu'il s'agit bien d'un DS18B20 */
  if (addr[0] != 0x28) {
    // Mauvais type de capteur
    return INVALID_SENSOR;
  }
 
  /* Reset le bus 1-Wire et sélectionne le capteur */
  ds.reset();
  ds.select(addr);
  
  /* Lance une prise de mesure de température et attend la fin de la mesure */
  ds.write(0x44, 1);
  delay(800);
  
  /* Reset le bus 1-Wire, sélectionne le capteur et envoie une demande de lecture du scratchpad */
  ds.reset();
  ds.select(addr);
  ds.write(0xBE);
 
 /* Lecture du scratchpad */
  for (byte i = 0; i < 9; i++) {
    data[i] = ds.read();
  }
   
  /* Calcul de la température en degré Celsius */
  *temperature = (int16_t) ((data[1] << 8) | data[0]) * 0.0625; 
  
  // Pas d'erreur
  return READ_OK;
}

// %f output is NOT working on arduino. Use this workaround: 
// https://github.com/esp8266/Arduino/issues/341
/* float to string
 * f is the float to turn into a string
 * p is the precision (number of decimals)
 * return a string representation of the float.
 */
char *f2s(float f, int p){
  char * pBuff;                         // use to remember which part of the buffer to use for dtostrf
  const int iSize = 10;                 // number of bufffers, one for each float before wrapping around
  static char sBuff[iSize][20];         // space for 20 characters including NULL terminator for each float
  static int iCount = 0;                // keep a tab of next place in sBuff to use
  pBuff = sBuff[iCount];                // use this buffer
  if(iCount >= iSize -1){               // check for wrap
    iCount = 0;                         // if wrapping start again and reset
  }
  else{
    iCount++;                           // advance the counter
  }
  return dtostrf(f, 0, p, pBuff);       // call the library function
}
