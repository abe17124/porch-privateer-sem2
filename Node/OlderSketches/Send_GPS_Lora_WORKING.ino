#include <SPI.h>
#include <TinyLoRa.h>
#include "TinyGPSPlus.h"
#include <avr/dtostrf.h>

// Visit your thethingsnetwork.org device console
// to create an account, or if you need your session keys.

// Network Session Key (MSB)
uint8_t NwkSkey[16] = {0x00, 0x08, 0x7d, 0xc0, 0x6a, 0xd3, 0xe9, 0xc9, 0x1b, 0x6a, 0x8b, 0x71, 0x60, 0x07, 0x12, 0x68};

// Application Session Key (MSB)
uint8_t AppSkey[16] = {0x2a, 0x36, 0x67, 0xb8, 0xcd, 0xa0, 0xf2, 0xf6, 0x38, 0x21, 0xb3, 0x08, 0x40, 0xb3, 0x28, 0x5e};

// Device Address (MSB)
uint8_t DevAddr[4] = {0x04, 0x05, 0x06, 0x07};

/************************** Example Begins Here ***********************************/
// Data Packet to Send to TTN
unsigned char loraData[17] = {"GPS COORDS START"};

// How many times data transfer should occur, in seconds
const unsigned int sendInterval = 30;

// Pinout for Adafruit Feather M0 LoRa
TinyLoRa lora = TinyLoRa(3, 8, 4);
TinyGPSPlus gps; //GPS Object
char latData[10] = {};
char longData[10] = {};
char latandlongdata[21];
unsigned char lattest[6] = {"-37.3"};


void setup()
{
  delay(2000);
  Serial.begin(9600);
  Serial1.begin(9600);//This opens up communications to the GPS
  Serial.println("GPS Start");//Just show to the monitor that the sketch has started
  while (! Serial);
  
  // Initialize pin LED_BUILTIN as an output
  pinMode(LED_BUILTIN, OUTPUT);
  
  // Initialize LoRa
  Serial.print("Starting LoRa...");
  // define multi-channel sending
  lora.setChannel(MULTI);
  // set datarate
  lora.setDatarate(SF7BW125);
  if(!lora.begin())
  {
    Serial.println("Failed");
    Serial.println("Check your radio");
    while(true);
  }

  // Optional set transmit power. If not set default is +17 dBm.
  // Valid options are: -80, 1 to 17, 20 (dBm).
  // For safe operation in 20dBm: your antenna must be 3:1 VWSR or better
  // and respect the 1% duty cycle.

  // lora.setPower(17);

  Serial.println("OK");
}

void loop()
{

  while(Serial1.available())//While there are characters to come from the GPS
  {
    gps.encode(Serial1.read());//This feeds the serial NMEA data into the library one char at a time
  }

  if(gps.location.isUpdated())//This will pretty much be fired all the time anyway but will at least reduce it to only after a package of NMEA data comes in
  {
    //Get the latest info from the gps object which it derived from the data sent by the GPS unit
    dtostrf(gps.location.lat(), 6, 6, latData);
    dtostrf(gps.location.lng(), 6, 6, longData);
  }
  /*
  lora.frameCounter++;

  Serial.println("Sending Longitude");
  lora.sendData(longData, sizeof(latData), lora.frameCounter);
  lora.frameCounter++;
  

  */


  strcpy(latandlongdata, latData);
  strcat(latandlongdata, ",");
  strcat(latandlongdata, longData);
  
  Serial.println("Sending Location Data: ");
  Serial.println(latandlongdata);
  
  lora.sendData((unsigned char *)latandlongdata, sizeof(latandlongdata), lora.frameCounter);
  lora.frameCounter++;
  Serial.print("Frame Counter: ");
  Serial.println(lora.frameCounter);

  
  // blink LED to indicate packet sent
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
  
  Serial.println("delaying...");
  delay(sendInterval * 1000);
}
