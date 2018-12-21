
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


void setup() {
  digitalWrite(RSTPIN, HIGH);
  pinMode(RSTPIN, OUTPUT);

  Serial.begin(115200);

delay(5000); // wait 1s before sending. Should be enough to open serial. 
  
  Serial.println(F("RF24/examples/GettingStarted"));
  Serial.println(F("*** PRESS 'T' to begin transmitting to the other node"));


  
  radio.begin();

  Serial.println(F("********** Radio init done **********"));



  radio.setDataRate(RF24_250KBPS);
  radio.setCRCLength(RF24_CRC_8);
  radio.setRetries(15, 15);
  radio.setAutoAck(false);

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
      RFDATA temp;
      temp.addr = 1; 
      
      /* Lit la température ambiante à ~1Hz */
      if (getTemperature(&temp.rffloat.value, true) != READ_OK) {
        Serial.println(F("Erreur de lecture du capteur"));
        return;
      }
      
      /* Affiche la température */
      Serial.print(F("Temperature : "));
      Serial.print(temp.rffloat.value, 2);
      Serial.write(176); // Caractère degré
      Serial.write('C');
      Serial.println();

      char blabla[64];
        sprintf(blabla, "value2: %f", (float)temp.rffloat.value); 

      //sprintf(blabla,"Gonna send: %f-", temp.value);
      Serial.print(blabla);
      Serial.println(".");


for (int i=0; i<5; i++)
{
 Serial.print(temp.rffloat.bytes[i], HEX); // Print the hex representation of the float
 Serial.print(' ');
}
Serial.println(".");

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

void send (RFDATA data) {

      char tx_bytes[sizeof(data.addr) + sizeof(data.rffloat.bytes)];
      memcpy(tx_bytes, data.addr, sizeof(data.addr)); 
      memcpy(tx_bytes+sizeof(data.addr), data.rffloat.bytes, sizeof(data.rffloat.bytes));
      radio.stopListening();                                    // First, stop listening so we can talk.
      
      
      Serial.println(F("Now sending"));
       if (!radio.write( tx_bytes, sizeof(tx_bytes))){
         Serial.println(F("failed"));
       } else {
        Serial.print("Sent ");
        Serial.print(sizeof(tx_bytes));
        Serial.println(" bytes");
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
          Serial.println(F("Failed, response timed out."));
      }else{
          unsigned long got_time;                                 // Grab the response, compare, and send to debugging spew
          radio.read( &got_time, sizeof(unsigned long) );
          unsigned long end_time = micros();
          
          // Spew it
          Serial.print(F("Sent "));
          char debug[64];
//          sprintf(debug, "%02X %02X %02X %02X %02X", data[0], data[1], data[2], data[3], data[4]); 
          sprintf(debug, "%0f", data); 
          Serial.print(debug);
          Serial.println();
          Serial.print(F(", Got response "));
          Serial.print(got_time);
                    Serial.println();


      }
  
}

void software_Reset() // Restarts program from beginning but does not reset the peripherals and registers
{
     digitalWrite(RSTPIN, LOW);
}  

/**
 * Fonction de lecture de la température via un capteur DS18B20.
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
