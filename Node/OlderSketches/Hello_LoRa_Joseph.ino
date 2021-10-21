#include <SPI.h>
#include <TinyLoRa.h>

//Test Code to transmit packets to chirpstack every 30 seconds


// Chirpstack Network Session Key, big endian
uint8_t NwkSkey[16] = {0xd0, 0xc3, 0x32, 0xb6, 0x94, 0xe1, 0x9d, 0xb3, 0x52, 0xe4, 0x6c, 0xba, 0xa5, 0x83, 0x1e, 0xe1};
//d0 c3 32 b6 94 e1 9d b3 52 e4 6c ba a5 83 1e e1
// Chirpstack Application Session Key, big endian
uint8_t AppSkey[16] = {0xa1, 0x73, 0x00, 0xca, 0x24, 0x38, 0x48, 0xb2, 0x4f, 0xaf, 0xfb, 0xf0, 0x5a, 0x14, 0x5c, 0xe3};
//a1 73 00 ca 24 38 48 b2 4f af fb f0 5a 14 5c e3
// Chirpstack Device Address, also big endian
uint8_t DevAddr[4] = {0x05, 0x06, 0x07, 0x08};

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
  lora.setDatarate(SF7BW125); //Set US908-928, Spreading Factor 7, BW125
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
