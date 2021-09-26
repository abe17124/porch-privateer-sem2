// Initial LoRaWAN_Motion Classification Script
// Author: Abishek Gomes
// Current Revision: REV A
// Revision Date: 9/15/2021

// Description: This program allows the feather node to
// classify its motion state using the BNO085 IMU module
// which outputs "On Table/Ground", "Stable", and "In Motion"
// states, it also authenticates a LoRaWAN connection using 
// ABP with GoCreates Chirpstack server, and send an Alert packet
// when it detects that the feather is in motion.


#include <TinyGPSPlus.h>
#include <Adafruit_BNO08x.h>
#include <Arduino.h>
#include <SPI.h>
#include <TinyLoRa.h>

//------------------SETUP for 9DOF Sensor -----------------------
#define BNO08X_CS 10
#define BNO08X_INT 9
#define BNO08X_RESET -1

Adafruit_BNO08x bno08x(BNO08X_RESET);
sh2_SensorValue_t sensorValue;
TinyGPSPlus gps;


//-----------------SETUP FOR LORAWAN----------------------------

//---------------Node Credentials Setup-----------------
// Network Session Key Big Endian
uint8_t NetworkSeshKey[16] = {0x00, 0x08, 0x7d, 0xc0, 0x6a, 0xd3, 0xe9, 0xc9, 0x1b, 0x6a, 0x8b, 0x71, 0x60, 0x07, 0x12, 0x68};

// Application Session Key Big Endian
uint8_t AppSeshKey[16] = {0x2a, 0x36, 0x67, 0xb8, 0xcd, 0xa0, 0xf2, 0xf6, 0x38, 0x21, 0xb3, 0x08, 0x40, 0xb3, 0x28, 0x5e};

// Device Address Big Endian
uint8_t DeviceAddress[4] = {0x04, 0x05, 0x06, 0x07};



//----------------Node Data Setup---------------------
// Data Packet to Send to Chirpstack with Char Array, Chars + 1 bit
unsigned char messageData[17] = {"Device is moving"};
unsigned char latData[7] = {};
unsigned char longData[7] = {};

// Data interval to ping Chirpstack
const unsigned int messageInterval = 10;

// Setting up an object with M0 RFM95 Pinout
TinyLoRa lora = TinyLoRa(3, 8, 4);


void setup(void)
{
  delay(500);
  Serial.begin(9600);

  
  //--------------------------------------------------------------
  //---------------------GT-U7 GPS-------------------------------
  Serial.println("GPS Start");//Just show to the monitor that the sketch has started
  Serial1.begin(9600); //Sets up Coms with GT-U7 RX and TX pins
  

  //----------------------------------------------------------------
  //----------------------LoRaWAN RFM95 Setup-----------------------
  
  // Initialize LED, used to show when packet sent
  pinMode(LED_BUILTIN, OUTPUT);
  
  lora.setChannel(MULTI); //Transmit to multiple channels
  lora.setDatarate(SF7BW125); //Setting Data Rate: Uplink 903.9 to 905.3, Downlink: 923.3 to 927.5

   if(!lora.begin()) //Initialize LoRa, if failed, check frame counter on chirpstack or ABP creds
  {
    Serial.println("Failed to initialize LoRa, possible Frame Counter Issue");
    while(true);
  }

  
  //---------------------------------------------------------------
  //----------------------9 DOF Sensor Setup-----------------------
  
  // Initialize the sensor with I2C
  if (!bno08x.begin_I2C()) {
    Serial.println("Unable to detect BNO085");
    while (1) {
      delay(10);
    }

    Serial.println("BNO085 Detected");
    Initialize_Reports();
  
    Serial.println("Reading events");
    delay(100);
    }

  Serial.println("Modules Initialized OK");
}



void Initialize_Reports(void) {
  //Enables Accelerometer reading reports
  if (!bno08x.enableReport(SH2_ACCELEROMETER)) {
    Serial.println("Could not enable accelerometer");
  }

  //Enables stability classification report (Stable, On Ground, In Motion)
  if (!bno08x.enableReport(SH2_STABILITY_CLASSIFIER)) {
    Serial.println("Could not enable stability classifier");
  }

  //Enables shake detection report, TO BE TESTED
  if (!bno08x.enableReport(SH2_SHAKE_DETECTOR)) {
    Serial.println("Could not enable shake detector");
  }
}





/*------------TODO---------
 * Set Device in ARM and DISARM mode of those strings are recieved using Button
 * Device will loop and check for State bool of ARM is true or false, and 
 * sets 9DOF sensor in standby mode or active more (Check Datasheet for low power/deepsleep mode)
 */
void loop(void)
{
//Check to see if the BNO085 sensor was hard reset, if so, reinitialize reports
  delay(500);
  if (bno08x.wasReset()) {
    Serial.print("sensor was reset ");
    Initialize_Reports();
  }

  //If you're unable to getSensorEvent
  if (!bno08x.getSensorEvent(&sensorValue)) {
    return;
  }

  //Listen for sensorID's recieved from 9DOF sensor and match it to cases below
  if(sensorValue.sensorId){
    if(SH2_STABILITY_CLASSIFIER){ //If SensorID Returns Stability Classifier 
      sh2_StabilityClassifier_t stability = sensorValue.un.stabilityClassifier;
      switch (stability.classification) {
        case STABILITY_CLASSIFIER_UNKNOWN:
          //Serial.println("Unknown");
          break;
        case STABILITY_CLASSIFIER_ON_TABLE: //This classification is when there is no movement on any axis, and the device is place somewhere
          Serial.println("On Table/Ground");
          break;
        case STABILITY_CLASSIFIER_STABLE: //This classification activates when the device is in a constant acceleration with little deviation, but not in motion, EX: Held steady by hand
          Serial.println("Stable");
          break;
        case STABILITY_CLASSIFIER_MOTION: //This classification activates when the device has considerable changes in any axes
          Serial.println("In Motion");
          Serial.println("Sending Alert Packet");
          lora.sendData(messageData, sizeof(messageData), lora.frameCounter);
          lora.frameCounter++;

          digitalWrite(LED_BUILTIN, HIGH);
          delay(500); //Blink LED when packet is sent
          digitalWrite(LED_BUILTIN, LOW);

          while(Serial1.available()) //Retrieve NMEA Data from GPS
          {
            gps.encode(Serial1.read()); // Feed data into TinyGPS library to get Lat Long
          }
            if(gps.location.isUpdated()){
              Serial.println("Latitude:");
              Serial.println(gps.location.lat(), 6);
              latData = gps.location.lat();
              Serial.println("Longitude:");
              Serial.println(gps.location.lng(), 6);
              longData = gps.location.lng();
              //Compile Lat and Long, and store them into array
            }

          //Send Lattitude data
          lora.sendData(latData, sizeof(latData), lora.frameCounter);
          lora.frameCounter++;

          //Send Longitude Data
          lora.sendData(longData, sizeof(longData), lora.frameCounter);            
          Serial.print("Final Frame Counter: ");Serial.println(lora.frameCounter); //Display the final frame counter
          lora.frameCounter++;

          //Delay Node when for interval before next "Alert" packet is sent or program is released to recheck status
          delay(messageInterval * 1000);
          break;
      }
    }
  }
}
