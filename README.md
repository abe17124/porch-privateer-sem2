# Porch Pirate IoT LoRaWAN Device Semester 2

# Description
Porch Pirate IoT Device for Senior Design Project Semester 2 @ Wichita State University. This product allows the Wichita Police Department to track stolen assets/packages and retrieve them. This device is an IoT node that can be placed directly onto critical assets/packages to track the status of the package and alert interested parties through a frontend application (Grafana or equivalent) for both iPhone and Android 10+.

The Porch Pirate IoT node actively tracks its own position and alerts LEO’s when its position has changed 2m relative to its position when it was armed. This hardware node can be placed inside packages or affixed to critical city assets such as city bikes, and construction equipment to monitor their location. Once the hardware node detects a position change, it packages its current latitude, longitude, and speed information into an end-to-end encrypted payload that is sent to gateways via LoRaWAN. The information is then decoded and displayed onto a frontend application where LEO’s can track down the package as it actively moves and transmits. 

![WhatsApp Image 2021-11-04 at 10 41 32 AM](https://user-images.githubusercontent.com/69644136/140364619-89faeb2f-e145-4e79-8ec1-148cab9a58ea.jpeg)


# Parts List 

* Adafruit Feather M0 with RFM95 LoRa Radio 
* 3D Printed housing (See Node Folder)
* GPS Module GT-U7
* uFL SMT Antenna Connector
* 900MHz Antenna
* 1500 mAh Li Poly battery
* 30x70mm Protoboard (only 1 needed)
* [Bill of Materials](https://docs.google.com/document/d/1Mle4k8iCFTbPMJ-krMxWjjYFfz29U2Eb0DUpucMsKYo/edit?usp=sharing)


# Operation and Maintenance
Operation has been extremely simplified for ease of use. Once you have identified your target location outdoors, simply turn on the node (Slide switch shown below moved to the right) while stationary, for proper signal sync of the GPS module. Once turned on, the GPS will have a steady light (standby mode), and when the signal has synced, the GPS led will flash every second, this process will take a few seconds. At this point, the node is armed and ready to detect changes in position relative to the last position it recorded. Turning off the node (Slide switch moved to the left), will disarm and stop all connectivity/transmission. To charge the battery housed inside the enclosure, connect a microusb into the jack on the Feather board shown below and plug it into a phone charger. This will turn on a yellow light to indicate charging, which turns off when the battery is fully charged.

![image](https://user-images.githubusercontent.com/69644136/144472920-b0437cbb-2195-4f99-aee3-5bdd4c6373fc.png)
![image](https://user-images.githubusercontent.com/69644136/144473018-1fa3bedc-c600-41dd-ab34-2eea91c76244.png)


# Schematic
![image](https://user-images.githubusercontent.com/69644136/142782587-fe75c587-a839-469c-b799-a6234c22e914.png)


# Scripts
The scripts included in this repo are test revisions only meant for device development purposes and not intended for production deployment. The most recent script under the 'Node' folder is used for position tracking. The GPS module receives NMEA sentences and converts them to usable latitude, longitude, and speed information. The microcontroller then checks to see if the position between previous point, and current point have a change of greater than 2 meters. If such a change exists, a packet is prepared with the format "battery_level, latitude, longitude, speed in mph, speed in mps”' and transmitted through LoRaWAN. This is repeated every 5 seconds until no change in position is detected.
The ChirpstackCode.js is not a script in and of itself but it contains the Javascript codec that we wrote on the chirpstack server to decode the string payload received from the Node which is then passed through a Chirpstack integration to our frontend client, Grafana, where the values are parsed to make them viewable as location blips on a world map, along with other supporting information.


# Dependencies 
The following dependencies need to be installed for proper operation of the device, this device uses the Arduino IDE and other Adafruit Libraries for functioning properly. To properly install the dependencies, please follow the guides below, and install them from top to bottom order.
* [Arduino IDE 1.8.13 or higher](https://www.arduino.cc/en/software)
* [Adafruit SAMD21 Board Dependencies](https://learn.adafruit.com/adafruit-feather-m0-basic-proto/setup)
* [Adafruit Feather M0 RFM95](https://learn.adafruit.com/adafruit-feather-m0-radio-with-lora-radio-module/using-the-rfm-9x-radio)
* [TinyGPSPlus](https://github.com/mikalhart/TinyGPSPlus) (Install through Arduino Library Manager)
* [Arduino LMIC](https://www.arduino.cc/reference/en/libraries/mcci-lorawan-lmic-library/) (Install through Arduino Library Manager)

# Chirpstack Setup and Installation
It is required to generate a new device on the Chirpstack on the GoCreate LoRaWAN server to properly initiate authentication between the Node, Gateway, and Server. Please follow each step from top to bottom order.
[Install VPN and Connect to it, Steps 1-11](https://www.wichita.edu/services/its/ITSApplicationsTraining/VPNWIndows.php)

![image](https://user-images.githubusercontent.com/69644136/144474993-d7f68b32-6299-4faf-8b9b-d8cfa39446e9.png)

Once connected, log on to the Chirpstack application server on http://10.50.208.17:8080/#/login Username and password is located "Server Credentials" document under server
![image](https://user-images.githubusercontent.com/69644136/144475121-7eb44393-576d-46d4-9cea-524b74cb6b61.png)

Navigate to the dropdown menu listed "Gateway"and select "PorchPrivateersOrg". This is the organization under which all our nodes, and Grafana exist.
![image](https://user-images.githubusercontent.com/69644136/144475420-9ffa4ceb-f1aa-471e-ad17-1563a99f7e20.png)

Select "Applications", this will bring up the primary application where all nodes and integrations lie. Then select "PorchPrivateersApplication"
![image](https://user-images.githubusercontent.com/69644136/144475700-04a17219-a99d-43d7-8a33-93b4715ccc82.png)

Once selected, you will see the nodes registered under this application. Select "Create"
![image](https://user-images.githubusercontent.com/69644136/144475941-764726f7-f8a7-4a7d-883e-799edb09a720.png)

Give your device a desired name and description, click the circular arrow highlighted. This generates a unique Device EUI. Now click the "MSB" button, this will change to "LSB" and reverse the order of your Device EUI. Note this value down and keep it secret. Finally, click the "Device-profile" dropdown and select "PorchPrivateersOTAADeviceProfile" and select "Create Device" at the bottom right of the screen.
![image](https://user-images.githubusercontent.com/69644136/144476305-c77a2ef7-7e36-46e7-9db9-fdd1df92fb59.png)

Now you will be taken to a panel that says "Keys OTAA" as shown below, select the circular arrow highlighted, and notate this value down. **There is no need to click MSB in this case**. Click "Set Device Keys" in the bottom right. Your device is now successfully created, we have to now change the values we recorded into hexadecimal form, and add them to our script.
![image](https://user-images.githubusercontent.com/69644136/144476906-6861fdc6-9dd1-4159-8140-081558a03e22.png)

Reorder your values and convert them to hexadecimal with the following steps, and keep note of your new values.
![image](https://user-images.githubusercontent.com/69644136/144477919-faa72112-b0f1-497a-ba6e-d16d5ba96bb7.png)

Open the Position_Tracking_No-Serial.ino script and ensure your Feather M0 is plugged into your computer and recognized. Copy and paste your "DeviceEUI" key replacing the existing value, your final result will look like this (except with your own unique keys):
![image](https://user-images.githubusercontent.com/69644136/144478220-12a01c2c-8d99-45c9-8156-66415bbdfbda.png)

Copy and paste your application key replacing the existing value, your final result will look like this (except with your own unique keys):
![image](https://user-images.githubusercontent.com/69644136/144478354-dfe7dbfd-7ec7-481a-9310-2eaa5c64f45d.png)



Once all of these steps have been properly executed, please flash your Adafruit Feather M0 RFM95 with the Position_Tracking_No-Serial.ino file found in the Node folder within this repository. Your node is now ready for use. Follow the instructions given in "Operation and Maintenance" and you will see LoRaWAN Frames and Device Data showing up when in range with the LoRaWAN Gateway.

# Authors
Abishek Gomes, Joseph Wackowski


