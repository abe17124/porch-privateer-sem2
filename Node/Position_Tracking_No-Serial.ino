//Author: Abishek Gomes
//Revision: --C
//Date: 12/2/2021
//Description: Porch Privateers Node GPS Script, this script can 
//alert stakeholder that a package has moved if it detects a 2m
//change from the last recorded position.
//Developed for the City of Wichita.

#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include "TinyGPSPlus.h"
#include <avr/dtostrf.h>

#define VBATPIN A7 //Battery Pin to get voltage


static unsigned char BatLevel[3];

//Arrays to store datapoints retrieved from GPS
char latData[10] = {}; //Raw Lattitude Array
char longData[10] = {}; //Raw Longitude Array
char speedDataMPH[10] = {}; //Speed MPH
char speedDataMPS[10] = {}; // Speed mps

//Dummy lattitude and longitude for initialization
double prevLat = 0;
double prevLong = 0;

//Dummy speeds for init
double speedMPH = 0;
double speedMPS = 0;

double currentLat = 0;
double currentLong = 0;

//Placeholder var for distance
unsigned long distanceBetween = 0;

//Boolean to track state of LoRa TX
static bool sendingPacket = false;

//Raw float recieved from VBAT pin
float vbatPinRaw = 0; //Raw Float Value empty var
TinyGPSPlus gps; //GPS Object

//Session credentials from Chirpstack
//APPEUI Can be 0 for Chirpstack
static const u1_t PROGMEM APPEUI[8]= { 0000000000000000 };
void os_getArtEui (u1_t* buf) { memcpy_P(buf, APPEUI, 8);}

//DEVEUI, Remember this needs to be in LSB (Little-Endian)
static const u1_t PROGMEM DEVEUI[8]= { 0x8f, 0x4e, 0xeb, 0x37, 0x53, 0x15, 0x9e, 0x40 };
void os_getDevEui (u1_t* buf) { memcpy_P(buf, DEVEUI, 8);}

//AppKey retrieved from Chirpstack, this can be MSB (Big-Endian)
static const u1_t PROGMEM APPKEY[16] = { 0x9d,0x74,0xb5,0x06,0xc5,0xd2,0x41,0x8e,0x00,0x86,0x91,0x23,0xd4,0x8f,0x3e,0x0e };
void os_getDevKey (u1_t* buf) {  memcpy_P(buf, APPKEY, 16);}

//This is the data packet, where all metrics will be appended to
static unsigned char mydata[45];
static osjob_t sendjob;

//Transmission Interval
const unsigned TX_INTERVAL = 30;


// Adafruit M0 RFM95 Pinout
const lmic_pinmap lmic_pins = {
    .nss = 8,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 4,
    .dio = {3, 6, LMIC_UNUSED_PIN},
    .rxtx_rx_active = 0,
    .rssi_cal = 8,              
    .spi_freq = 8000000,
};


void onEvent (ev_t ev) {
    switch(ev) {
        case EV_JOINED: //This event triggers when the node pairs successfully to the chripstack server.
            {
              u4_t netid = 0;
              devaddr_t devaddr = 0;
              u1_t nwkKey[16];
              u1_t artKey[16];
              LMIC_getSessionKeys(&netid, &devaddr, nwkKey, artKey); //Getting session keys from Chirpstack
            }
            LMIC_setLinkCheckMode(0);
            break;
            
        case EV_TXCOMPLETE: //This triggeres when a transmission has been complete from node to gateway (uplink)
            os_setCallback(&sendjob, setSendingFalse); //Create job to set sending bool to false
            break;
        default:
            Serial.print(F("Unknown event: "));
            Serial.println((unsigned) ev);
            break;
    }
}

//This is a helper function that creates a OS job to set the sendingPacket to true
//OS job so that it runs in a seperate process and doesn't intervene the main process
void setSendingTrue(osjob_t* j){
  sendingPacket = true; //Blocking variable, prevents double packaging of payload
}

//This does the opposite of the above, sets it back to false,
//Thereby allowing another packet to begin compiling.
void setSendingFalse(osjob_t* j){
  sendingPacket = false; //Resetting the variable
}

//This is the OS job that actually transmits our packet 
void do_send(osjob_t* j){
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F("OP_TXRXPEND, not sending")); //Checking for an existing TX job.
    } else {
      // Package payload and transmit if all clear
        LMIC_setTxData2(1, mydata, sizeof(mydata)-1, 0);
        //Serial.println(F("Packet queued"));
        Serial.println((char *)mydata);
    }
}

//This function below handles reading all the metrics from the GT-U7 module
void readGPS(){
  //prevLat = currentLat; //Store the last recorded lat and long as previous
  //prevLong = currentLong;
  while(Serial1.available())
  {
    gps.encode(Serial1.read()); //Read GPS Data form the GPS module
  }

  if(gps.location.isUpdated())
  {
    //Get Lat and Long, and store doubles in char array
    dtostrf(gps.location.lat(), 6, 6, latData); //Convert double to string
    dtostrf(gps.location.lng(), 6, 6, longData);
    currentLat = gps.location.lat();
    currentLong = gps.location.lng();
  }
  if(gps.speed.isUpdated()){
    //Get speed is available
    dtostrf(gps.speed.mph(), 6, 6, speedDataMPH);
    dtostrf(gps.speed.mps(), 6, 6, speedDataMPS); //Convert and store speed
    speedMPH = gps.speed.mph();
    speedMPS = gps.speed.mps();
  }
}

//This function below reads the battery level from our LiPo battery
void readBattery(){
  //Read Battery Level
  vbatPinRaw = analogRead(VBATPIN); //Read analog value from A7
  vbatPinRaw *= 2;
  vbatPinRaw *= 3.3;  
  vbatPinRaw /= 1024; // Convert Raw Value to Voltage
  
  //Basic cases are High, Medium, and Low for simplicity.
  if(vbatPinRaw >= 4.0){ //Cases for different voltages
    strcpy((char *)BatLevel, "HI");
  }
  else if(vbatPinRaw <= 3.9 && vbatPinRaw >= 3.5){
    strcpy((char *)BatLevel, "ME");
  }
  else if(vbatPinRaw < 3.5){
    strcpy((char *)BatLevel, "LO"); //Store those in BatLevel
  }
}

//This is where we setup both LoRaWAN, and the GT-U7 GPS Module.
void setup() {
    delay(2000); //Wait until all modules powered up
    //while (! Serial)
        //;
    Serial.begin(9600);
    Serial.println(F("Starting"));
    
    //---------------------GT-U7 GPS-------------------------------
    Serial1.begin(9600);//This opens up communications to the GPS 

    // Initialize LMIC
    os_init();
    LMIC_reset();

    LMIC_setLinkCheckMode(0); //Verification for connection
    LMIC_setDrTxpow(DR_SF7,14); //Setting data rate, to use Spreading Factor 7 (Highest data rate)
    LMIC_selectSubBand(1);

    // Initialize job, this sents the first JoinRequest packet.
    do_send(&sendjob);
}

//This function handles all of the processes of executing both readBattery and readGPS functions,
//and compiles the data into the mydata packet in sequential
//Once again this is added as an OS job so that it doesn't interfere with the main process
void readMeasurements(osjob_t* j){
    readBattery(); //Get Battery Level
    readGPS(); //Get Lat, Long, and Speeds

    //Setting distance between to be calculated with the recorded values.
    distanceBetween =     
     (unsigned long)TinyGPSPlus::distanceBetween(
      currentLat,
      currentLong,
      prevLat, 
      prevLong); //Check distance between last current and previous position

      
    //This block concatenates all chars into the main mydata array in order. DO NOT CHANGE THE ORDER
    //Current order is "battery_level,speedmph,speedmps,latitude,longitude"
    //Example Packet: "HI,0.1603,0.0121,37.482923,-91.382382"
    memcpy(mydata, BatLevel, sizeof(BatLevel)); //Copy Battery Level
    strcat((char *)mydata, ",");
    strcat((char *)mydata, speedDataMPH); //Concatenate mph
    strcat((char *)mydata, ",");
    strcat((char *)mydata, speedDataMPS); //Concatenate mps
    strcat((char *)mydata, ",");
    strcat((char *)mydata, latData); //Concatenate lattitude
    strcat((char *)mydata, ",");
    strcat((char *)mydata, longData); //Concatenate longitude

    //If the distance between last 2 points are > than 2 meters,
    //Set sending variable to true, and send the packet to our transmission job.
    if(distanceBetween >= 2){
      sendingPacket = true;
      os_setCallback(&sendjob, setSendingTrue);
      os_setCallback(&sendjob, do_send);
    }
    else{
      Serial.println("No Position Change");
    }
}



void loop() {
  if(!sendingPacket){ //If there's no packet being sent, read measurements off all instruments and store.
  os_setCallback(&sendjob, readMeasurements);
  delay(5000); //Delay for 5 seconds.
  }
  os_runloop_once(); //This is the main event loop.
}
