#include <SPI.h>
#include <TinyLoRa.h>
#include "TinyGPSPlus.h"
#include <avr/dtostrf.h>

#define VBATPIN A7

//-----------------SETUP FOR LORAWAN----------------------------

//---------------Node Credentials Setup-----------------
// Network Session Key (MSB)
uint8_t NwkSkey[16] = {0x00, 0x08, 0x7d, 0xc0, 0x6a, 0xd3, 0xe9, 0xc9, 0x1b, 0x6a, 0x8b, 0x71, 0x60, 0x07, 0x12, 0x68};

// Application Session Key (MSB)
uint8_t AppSkey[16] = {0x2a, 0x36, 0x67, 0xb8, 0xcd, 0xa0, 0xf2, 0xf6, 0x38, 0x21, 0xb3, 0x08, 0x40, 0xb3, 0x28, 0x5e};

// Device Address (MSB)
uint8_t DevAddr[4] = {0x04, 0x05, 0x06, 0x07};



//----------------Node Data Setup---------------------
// Data Packet to Send to TTN
unsigned char loraData[17] = {"GPS COORDS START"};

// Data transfer interval, in seconds
const unsigned int sendInterval = 15;


//Empty Char Arrays for lat, long, battery, and payload
char latData[10] = {}; //Raw Lattitude Array
char longData[10] = {}; //Raw Longotide Array

char BatLevel[3] = {};

char payloadData[25];


// Setting up an object with M0 RFM95 Pinout
TinyLoRa lora = TinyLoRa(3, 8, 4);
TinyGPSPlus gps; //GPS Object


float vbatPinRaw = 0; //Raw Float Value empty var

void setup()
{
  delay(2000);
  //--------------------------------------------------------------
  //---------------------GT-U7 GPS-------------------------------
  //Serial.begin(9600);
  Serial1.begin(9600);//This opens up communications to the GPS
  //while (! Serial);



  //----------------------------------------------------------------
  //----------------------LoRaWAN RFM95 Setup-----------------------
  // Initialize pin LED_BUILTIN as an output
  pinMode(LED_BUILTIN, OUTPUT);
  
  // Set to send to multiple channels
  lora.setChannel(MULTI);
  // Set Datarate, based on Datarate table from chirpstack
  lora.setDatarate(SF7BW125); 
  if(!lora.begin())
  {
    //Serial.println("Failed to initialize LoRa");
    while(true);
  }
}

void loop()
{
  //Read Battery Level
  vbatPinRaw = analogRead(VBATPIN);
  vbatPinRaw *= 2;
  vbatPinRaw *= 3.3;  
  vbatPinRaw /= 1024; // Convert Raw Value to Voltage

  if(vbatPinRaw >= 4.0){
    strcpy(BatLevel, "HI");
  }
  else if(vbatPinRaw <= 3.9 && vbatPinRaw >= 3.5){
    strcpy(BatLevel, "ME");
  }
  else if(vbatPinRaw < 3.5){
    strcpy(BatLevel, "LO");
  }

  
  while(Serial1.available())
  {
    gps.encode(Serial1.read());
  }

  if(gps.location.isUpdated())
  {
    //Get Lat and Long, and store doubles in char array
    dtostrf(gps.location.lat(), 6, 6, latData);
    dtostrf(gps.location.lng(), 6, 6, longData);
  }

  //Concatenate Battery Level, Lattitude, Longitude into Payload
  strcat(payloadData, BatLevel);
  strcat(payloadData, ",");
  strcat(payloadData, latData);
  strcat(payloadData, ",");
  strcat(payloadData, longData);
  
  //Serial.println("Sending Location Data: ");
  //Serial.println(payloadData);
  
  lora.sendData((unsigned char *)payloadData, sizeof(payloadData), lora.frameCounter);
  lora.frameCounter++;
  //Serial.print("Frame Counter: ");
  //Serial.println(lora.frameCounter);

  strcpy(BatLevel, ""); //Empty out both the battery level, and the payload from previous txmissions
  strcpy(payloadData, "");


  
  // Blind LED to indicate package sent
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);

  delay(sendInterval * 1000);
}
