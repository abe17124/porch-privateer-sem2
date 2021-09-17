#include <SPI.h>
#include <TinyLoRa.h>

//Test Code to transmit packets to chirpstack every 30 seconds


// Chirpstack Network Session Key, big endian
uint8_t NwkSkey[16] = {0x00, 0x08, 0x7d, 0xc0, 0x6a, 0xd3, 0xe9, 0xc9, 0x1b, 0x6a, 0x8b, 0x71, 0x60, 0x07, 0x12, 0x68};

// Chirpstack Application Session Key, big endian
uint8_t AppSkey[16] = {0x2a, 0x36, 0x67, 0xb8, 0xcd, 0xa0, 0xf2, 0xf6, 0x38, 0x21, 0xb3, 0x08, 0x40, 0xb3, 0x28, 0x5e};

// Chirpstack Device Address, also big endian
uint8_t DevAddr[4] = {0x04, 0x05, 0x06, 0x07};

/************************** Example Begins Here ***********************************/
// Chirpstack Data Packet
unsigned char sendPacket[11] = {"hello LoRa"};
const unsigned int delayInterval = 30;

// Pins for RFM95 Chip
TinyLoRa lora = TinyLoRa(3, 8, 4);

void setup()
{
  delay(2000);
  Serial.begin(9600);
  while (! Serial);
  
  // Set LED to Output
  pinMode(LED_BUILTIN, OUTPUT);
  
  // Initialize LoRa
  Serial.print("Initializing LoRa Chip");
  lora.setChannel(MULTI); //Send to all available channels
  lora.setDatarate(SF7BW125); //Set US908-928, Spreading Factor 125
  if(!lora.begin())
  {
    Serial.println("Failed to initialize Chip");
    Serial.println("Possible Frame Counter Issue, Reactivate device");
    while(true);
  }
}

void loop()
{
  Serial.println("Sending LoRa Packet to CS");
  lora.sendData(sendPacket, sizeof(sendPacket), lora.frameCounter);
  Serial.print("Frame Counter: ");Serial.println(lora.frameCounter); //Print Frame Counter
  lora.frameCounter++;

  // blink LED to indicate packet sent
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
  
  Serial.println("Queuing next Packet...");
  delay(delayInterval * 1000);
}
