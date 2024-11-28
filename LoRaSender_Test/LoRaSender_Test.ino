// For LoRa
#include <SPI.h>
#include <LoRa.h>
#define BAND 920E6
#define SS 15 // chip select pin
#define RST 5 // Reset pin
#define DIO0 4 // Interrupt pin 


int counter = 0;
int readingID = 0;
String LoRa_status = "Not Initialized";


void startLoRa() {
  
  LoRa.setPins(SS,RST,DIO0);
  while (!LoRa.begin(BAND) && counter < 10) {
    Serial.print(".");
    counter++;
    delay(500);
  }
  if (counter == 10){
    // Increment readingID on every new reading
    readingID++;
    Serial.println("LoRa Initialization Failed!");
    LoRa_status = "Initialization Failed";
    return;
  }
  Serial.println("LoRa Initialization successful.");
  LoRa_status = "Initialized";
  delay(500);

  // Set LoRa module parameters
  LoRa.setTxPower(14); // Set transmission power (dBm)
  LoRa.setSpreadingFactor(7); // Set spreading factor 
  LoRa.setSignalBandwidth(500E3); // Set bandwidth
  LoRa.setCodingRate4(5); // Set coding rate
  LoRa.setPreambleLength(8); // Set preamble length

 delay(2000);
}







void setup() {
  Serial.begin(9600);
  while (!Serial);

  startLoRa();
}

void loop() {
  Serial.print("Sending packet: ");
  Serial.println(counter);

  // send packet
  LoRa.beginPacket();
  LoRa.print("hello ");
  LoRa.print(counter);
  LoRa.endPacket(true);

  counter++;

  delay(5000);
}
