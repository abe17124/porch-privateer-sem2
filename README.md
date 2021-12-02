# Porch Pirate IoT LoRaWAN Device Semester 2

# Description
Porch Pirate IoT Device for Senior Design Project Semester 2 @ Wichita State University. This product allows the Wichita Police Department to track stolen assets/packages and retrieve them. This device is an IoT node that can be placed directly onto critical assets/packages to track the status of the package and alert interested parties through frontend application (Grafana or equivalent) for both iPhone and Android 10+.

The Porch Pirate IoT node actively tracks its own position and alerts LEO’s when its position has changed 2m relative to its position when it was armed. This hardware node can be placed inside packages or affixed to critical city assets such as city bikes, and construction equipment to monitor their location. Once the hardware node detects a position change, it packages its current latitude, longitude, and speed information into an end-to-end encrypted payload that is sent to gateways via LoRaWAN. The information is then decoded and displayed onto a frontend application where LEO’s can track down the package as it actively moves and transmits. 

# Parts List 

* Adafruit Feather M0 with RFM95 LoRa Radio 
* 3D Printed housing (See Node Folder)
* GPS Module GT-U7
* uFL SMT Antenna Connector
* 900MHz Antenna
* 1500 mAh Li Poly battery
* [Bill of Materials]:(https://docs.google.com/document/d/1Mle4k8iCFTbPMJ-krMxWjjYFfz29U2Eb0DUpucMsKYo/edit?usp=sharing)


# Operation and Maintainence
Operation has been simplified to allows users to have ease of use. Once you have identified your target location outdoors, simply turn on the node (Slide switch shown below moved to the right) while stationary, for proper signal sync of the GPS module. Once turned on, the GPS will have a steady light (standby mode), and when the signal has synced, the GPS led will start flashing every second, this will take a few seconds. At this point, the node is armed and ready to detect changes in position relative to the last position it recorded. Turning off the node (Slide switch moved to the left), will disarm and stop all connectivity/transmission. To charge the battery housed inside the enclosure, connect a microusb into the jack on the Feather board shown below. This will turn on a yellow light to indicate charging, which turns off when the battery is fully charged.


# Schematic
![image](https://user-images.githubusercontent.com/69644136/142782587-fe75c587-a839-469c-b799-a6234c22e914.png)
![WhatsApp Image 2021-11-04 at 10 41 32 AM](https://user-images.githubusercontent.com/69644136/140364619-89faeb2f-e145-4e79-8ec1-148cab9a58ea.jpeg)


# Scripts
The scripts included in this repo are test revisions only meant for device development purposes and not intended for production deployment. The most recent script under the 'Node' folder is used for position tracking. The GPS recieves NMEA sentences and converts them to usable latitude, longitude, and speed information. The microcontroller then checks to see if the position between previous point, and current point have a change of greater than 2 meters. If such a change exists, a packet is prepared with the format "battery_level, latitude, longitude, speed in mph, speed in mps" and transmitted through LoRaWAN. This is repeated every 5 seconds until no change in position is detected.
The ChirpstackCode.js is not a script in and of itself but it contains the Javascript codec that we wrote on the chirpstack server to decode the string payload recieved from the Node which is then passed through a Chirpstack integration to our frontend client, Grafana, where the values are parsed to make them viewable as location blips on a worldmap, along with other supporting information.

# Dependancies and Installation
The following dependancies need to be installed in for proper operation of the device, this device uses the Arduino IDE and other Adafruit Libraries for functioning properly. To properly install the dependancies, please follow the guides below.
* [Arduino IDE 1.8.13 or higher](https://www.arduino.cc/en/software)
* [Adafruit SAMD21 Board Dependancies](https://learn.adafruit.com/adafruit-feather-m0-basic-proto/setup)
* [Adafruit Feather M0 RFM95](https://learn.adafruit.com/adafruit-feather-m0-radio-with-lora-radio-module/using-the-rfm-9x-radio)
* [TinyGPSPlus](https://github.com/mikalhart/TinyGPSPlus) (Install through Arduino Library Manager)
* [Arduino LMIC](https://www.arduino.cc/reference/en/libraries/mcci-lorawan-lmic-library/) (Install through Arduino Library Manager)

Once these are installed, and the module has been built according to the schematic shown above, please flash your Adafruit Feather M0 RFM95 with the Position_Tracking_No-Serial.ino file found in the Node folder within this repository.

# Authors
Abishek Gomes, Joseph Wackowski

