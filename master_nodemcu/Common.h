
/*
 * Adresse used by all nodes for the RF24 radio
 *  master always use [0], slave depend on jumpers
 */

#ifndef _RFCOMMON_H
#define _RFCOMMON_H

byte addresses[][6] = {"1Node","2Node","3Node","4Node"};

// Number of max connected nodes
#define MAXNODES 3

// Number of retries per node before stopping displaying it (will still be queried)
#define MAXRETRIES 10

// value used to autodetect if a sensors is connected. This value needs to be something which will never happend
#define NOSENSOR (-999) // needs to be a float, two decimal rounded

// Dummy value used for autodetect if a sensor is connected or not on the slave
#define DUMMYVALUE 55

// Enable or disable autoACK functionnalities on RF24
#define ACKENABLED true

#define CRCLENGTH RF24_CRC_16
#define RFSPEED RF24_1MBPS
#define RFPOWER RF24_PA_HIGH
#define PAYLOADSIZE 8

#endif //_RFCOMMON_H
