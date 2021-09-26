# Porch Pirate IoT LoRaWAN Device Semester 2

# Description
Porch Pirate IoT BLE Device for Senior Design Project Semester 2 @ Wichita State University. This product allows the Wichita Police Department to track stolen assets/packages and retrieve them. This device is an IoT node that can be placed directly onto critical assets/packages to track the status of the package and alert interested parties through frontend application (Grafana or equivalent) for both iPhone and Android 10+.

# Parts List 
* Adafruit Feather M0 with RFM95 LoRa Radio 
* Adafruit 9-DOF Orientation IMU Fusion Breakout - BNO085
* 3D Printed housing
* GPS Module GT-U7
* uFL SMT Antenna Connector
* uFL to RP-SMA antenna adapter cable
* RGB LED (Common Anode)


# Method of Operation
This device uses a Adafruit BNO085 9 DOF sensor to percieve its current state and translates the raw data using the Adafruit Sensor Libraries into motion classifications such as On Table/Ground, Stable, and In Motion. The data is then forwarded to the Adafruit Feather M0 RFM95 where it is processed to send a LoRaWAN "Device is moving" packet through the HopeRF RFM95 chip to nearby Gateways. This data can be accessed through chirpstack and viewed through frontend application for Law Enforcement Officers to understand the state of a critical asset. Next steps include: Identifying motion through GPS coordinates, packaging coordinates as chars, transmitting through LoRaWAN to end gateways.


# Schematic
![image](https://user-images.githubusercontent.com/69644136/115118302-29b3da80-9f68-11eb-86fe-3ad450fbcc1e.png)
![WhatsApp Image 2021-09-17 at 11 07 53 AM](https://user-images.githubusercontent.com/69644136/133819804-879bdd46-a264-48e3-852f-415aea89bee7.jpeg)

# Scripts
The two scripts included in the repo are test revisions only meant for device development purposes and not intended for production deployment. You will see two scripts in the repo as shown below:
* LORAWAN_GPS_MOTION_CLASSIFY.ino
* hello_LoRa-abp-test.ino

The first script functions as mentioned in the previous sections, where the accelerometer identifies that the device has moved which then pings the GPS to get current lattitude, longitude, and sattelite counts. This is then packaged in a LoRaWAN packet and sent along with a "Device has moved" packet.
The last script is a basic functionality testing script that authenticates with the Chirpstack server using ABP and send a packet at regular intervals.

# Dependancies and Installation
The following dependancies need to be installed in for proper operation of the device, this device uses the Arduino IDE and other Adafruit Libraries for functioning properly. To properly install the dependancies, please follow the guides below.
* [Arduino IDE 1.8.13 or higher](https://www.arduino.cc/en/software)
* [Adafruit SAMD21 Board Dependancies](https://learn.adafruit.com/adafruit-feather-m0-basic-proto/setup)
* [Adafruit Feather M0 RFM95](https://learn.adafruit.com/adafruit-feather-m0-radio-with-lora-radio-module/using-the-rfm-9x-radio)
* [Adafruit BNO08x Library](https://learn.adafruit.com/adafruit-9-dof-orientation-imu-fusion-breakout-bno085/arduino)
* [TinyGPSPlus](https://github.com/mikalhart/TinyGPSPlus) (Install through Arduino Library Manager)
* [TinyLora](https://github.com/adafruit/TinyLoRa) (Install through Arduino Library Manager)

# Authors
Abishek Gomes, Devan Mears, Joseph Wackowski, Austin Major

