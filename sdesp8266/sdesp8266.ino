#include <SPI.h>
#include <SD.h>

#define CS_PIN 15  // GPIO15 (D8) for CS

void setup() {
  Serial.begin(115200);  // Initialize serial communication
  delay(1000);  // Give time for serial monitor to connect

  Serial.println("Initializing SD card...");

  // Initialize SD card using default SPI pins (GPIO13, GPIO12, GPIO14)
  if (!SD.begin(CS_PIN)) {
    delay(1500);
    Serial.println("SD card initialization failed.");
    return;
  }

  Serial.println("SD card initialized successfully.");

  // Create or open a file
  File file = SD.open("/test.txt", FILE_WRITE);
  if (file) {
    file.println("Hello, SD card!");
    file.close();  // Close the file
    Serial.println("Data written to file.");
  } else {
    Serial.println("Failed to open file.");
  }
}

void loop() {
  // Nothing needed here
}
