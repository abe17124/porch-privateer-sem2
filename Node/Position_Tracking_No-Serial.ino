#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include "TinyGPSPlus.h"
#include <avr/dtostrf.h>

#define VBATPIN A7 //Battery Pin to get voltage


static unsigned char BatLevel[3];

char latData[10] = {}; //Raw Lattitude Array
char longData[10] = {}; //Raw Longitude Array
char speedDataMPH[10] = {}; //Speed MPH
char speedDataMPS[10] = {}; // Speed mps

double prevLat = 32.2737283;
double prevLong = -91.2937232;

double speedMPH = 0;
double speedMPS = 0;

double currentLat = 0;
double currentLong = 0;

unsigned long distanceBetween = 0;

static bool sendingPacket = false;

float vbatPinRaw = 0; //Raw Float Value empty var
TinyGPSPlus gps; //GPS Object

//Session credentials from Chirpstack
//EUI Can be 0 for Chirpstack
static const u1_t PROGMEM APPEUI[8]= { 0000000000000000 };
void os_getArtEui (u1_t* buf) { memcpy_P(buf, APPEUI, 8);}

static const u1_t PROGMEM DEVEUI[8]= { 0x8f, 0x4e, 0xeb, 0x37, 0x53, 0x15, 0x9e, 0x40 };
void os_getDevEui (u1_t* buf) { memcpy_P(buf, DEVEUI, 8);}

static const u1_t PROGMEM APPKEY[16] = { 0x9d,0x74,0xb5,0x06,0xc5,0xd2,0x41,0x8e,0x00,0x86,0x91,0x23,0xd4,0x8f,0x3e,0x0e };
void os_getDevKey (u1_t* buf) {  memcpy_P(buf, APPKEY, 16);}

//Data packet
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
        case EV_JOINED:
            {
              u4_t netid = 0;
              devaddr_t devaddr = 0;
              u1_t nwkKey[16];
              u1_t artKey[16];
              LMIC_getSessionKeys(&netid, &devaddr, nwkKey, artKey); //Getting session keys from Chirpstack
            }
            LMIC_setLinkCheckMode(0);
            break;
            
        case EV_TXCOMPLETE:
            os_setCallback(&sendjob, setSendingFalse); //Create job to set sending bool to false
            break;
        default:
            Serial.print(F("Unknown event: "));
            Serial.println((unsigned) ev);
            break;
    }
}

void setSendingTrue(osjob_t* j){
  sendingPacket = true; //Blocking variable, prevents double packaging of payload
}

void setSendingFalse(osjob_t* j){
  sendingPacket = false; //Resetting the variable
}

void do_send(osjob_t* j){
    // Check if another job is running, shouldn't because we have blocking jobs, but just in case.
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F("OP_TXRXPEND, not sending"));
    } else {
      // Package payload
        LMIC_setTxData2(1, mydata, sizeof(mydata)-1, 0);
        //Serial.println(F("Packet queued"));
        Serial.println((char *)mydata);
    }
}

void readGPS(){
  //prevLat = currentLat; //Store the last recorded lat and long as previous
  //prevLong = currentLong;
  while(Serial1.available())
  {
    gps.encode(Serial1.read()); //Read GPS Data
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
    dtostrf(gps.speed.mph(), 6, 6, speedDataMPH);
    dtostrf(gps.speed.mps(), 6, 6, speedDataMPS); //Convert and store speed
    speedMPH = gps.speed.mph();
    speedMPS = gps.speed.mps();
  }
}

void readBattery(){
  //Read Battery Level
  vbatPinRaw = analogRead(VBATPIN); //Read analog value from A7
  vbatPinRaw *= 2;
  vbatPinRaw *= 3.3;  
  vbatPinRaw /= 1024; // Convert Raw Value to Voltage
  
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

//Module Setup
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
    LMIC_setDrTxpow(DR_SF7,14); //Set data rate and transmit power, need to use ADR sometime soon, so this will be gone. 
    LMIC_selectSubBand(1);

    // Initialize job
    do_send(&sendjob);
}
void readMeasurements(osjob_t* j){
    readBattery(); //Get Battery Level
    readGPS(); //Get Lat, Long, and Speeds

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
