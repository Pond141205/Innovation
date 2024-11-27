// Pin set for esp8266 board



// For Temperature sensor
#include <OneWire.h>
#include <DallasTemperature.h>
// For LoRa
#include <SPI.h>
#include <LoRa.h>
#define BAND 920E6
#define SS 15 // chip select pin
#define RST 5 // Reset pin
#define DIO0 4 // Interrupt pin 


// Pin definitions
#define ONE_WIRE_BUS 4 // DS18B20 data pin (GPIO4, D2)
#define RELAY_PELTIER 13 // Relay for Fan (GPIO12, D6)
#define RELAY_FAN 12 // Realy for Peltier (GPIO13, D7)

// Thresold temperature
#define TEMP_HIGH_THRESOLD 29.0 // Temperature to turn on Peltier and Fan
#define TEMP_LOW_THRESOLD 25.0 // Temperature to turn on Peltier and Fan




// Setup a oneWire and DallasTemperature instance
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Addresses of the DS18B20 sensors
DeviceAddress sensor1, sensor2;

// Initialize status string
String Temperature_sensor1_status = "Active";
String Temperature_sensor2_status = "Active";
String Fan_status = "OFF";
String Peltier_status = "OFF";
String LoRa_status = "Not Initialized";
int LoRa_Skip_Message = 0;

// For LoRa packet counter
int readingID = 0;
int counter = 0;
String LoRaMessage = "";
const String Headcode = ""; /////////// dont for got to put headcode

// Temperature Data
float Temperature ;

// Timing variables
unsigned long tempCheckPreviousMillis = 0;
unsigned long sendDataPreviousMillis = 0;
const unsigned long tempCheckInterval = 10000; // 10 seconds
const unsigned long sendDataInterval = 900000; // 15 minutes (900,000 ms)


// Flags
bool tempThresholdExceeded = false;
bool testMode = false; // Test mode Flag



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



void setup_Tem_sensor() {
  Serial.println("Initializing DS18B20 sensors...");
  // Start the DS18B20 sensor library
  sensors.begin();

  // Locate devices on the bus
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" devices.");

  // Assign sensor addresses
  if (!sensors.getAddress(sensor1, 0)) {
    Serial.println("Unable to find address for Sensor 1");
    Temperature_sensor1_status = "Disconnected";
  }
  if (!sensors.getAddress(sensor2, 1)) {
    Serial.println("Unable to find address for Sensor 2");
    Temperature_sensor2_status = "Disconnected";
  }

  // Set resolution to 9-bit, 10-bit, 11-bit, or 12-bit (higher is more accurate but slower)
  sensors.setResolution(sensor1, 12);
  sensors.setResolution(sensor2, 12);  
}


void sendReadings(String warning = "") {
  if (LoRa_status == "Initialized") {
    LoRaMessage = String(Headcode) + "/" + String(readingID) + "$" + String(Temperature) + "#" + Temperature_sensor1_status + "&" +
                  Temperature_sensor2_status + "%" + Peltier_status + "=" + Fan_status;
    if (warning != "") {
      LoRaMessage += " WARNING: " + warning;
    }
    // LoRa.beginPacket();
    // LoRa.print(LoRaMessage);
    // LoRa.endPacket();
    Serial.println(LoRaMessage);
    readingID++;
  } else {
    LoRa_Skip_Message += 1;
  }
}




void getTemp() {

  // Request temperature
  sensors.requestTemperatures();

  // Read Temperature from sensor
  float temp1 = sensors.getTempC(sensor1);
  float temp2 = sensors.getTempC(sensor2);

 
  if (temp1 == DEVICE_DISCONNECTED_C && temp2 != DEVICE_DISCONNECTED_C)  {
    Temperature_sensor1_status = "Disconnected";
    Temperature_sensor2_status = "Active";
    Temperature = temp2;
  } else if (temp2 == DEVICE_DISCONNECTED_C && temp1 != DEVICE_DISCONNECTED_C) {
    Temperature_sensor2_status = "Disconnected";
    Temperature_sensor1_status = "Active";
    Temperature = temp1;
  } else if (temp1 == DEVICE_DISCONNECTED_C && temp2 == DEVICE_DISCONNECTED_C) {
    Temperature_sensor1_status = "Disconnnected";
    Temperature_sensor2_status = "Disconneected";
    Temperature = -999;
  } else {
    Temperature_sensor1_status = "Active";
    Temperature_sensor2_status = "Active";
    Temperature = (temp1 + temp2)/2;
  }
  Serial.println("Temp: " + String(Temperature));

}



void Temp_control() {
  if (Temperature >= TEMP_HIGH_THRESOLD) {
    digitalWrite(RELAY_PELTIER, HIGH);  // Turn on Peltier
    digitalWrite(RELAY_FAN, HIGH);      // Turn on Fan
    Peltier_status = "ON";
    Fan_status = "ON";
    
    // Send an immediate alert only once when the threshold is exceeded
    if (!tempThresholdExceeded) {
      tempThresholdExceeded = true;
      sendReadings("High Temperature!"); // Immediate alert
    }
  } else if (Temperature <= TEMP_LOW_THRESOLD) {
    digitalWrite(RELAY_PELTIER, LOW);   // Turn off Peltier
    digitalWrite(RELAY_FAN, LOW);       // Turn off Fan
    Peltier_status = "OFF";
    Fan_status = "OFF";
    
    // Reset the flag to allow for a new alert if threshold is exceeded again
    tempThresholdExceeded = false;
  }
}


void debugStatus() {
  Serial.println("==== DEBUG STATUS ====");
  Serial.println("LoRa Status: " + LoRa_status);
  Serial.println("LoRa skip message count: " + String(LoRa_Skip_Message));
  Serial.println("Temperature Sensor 1 Status: " + Temperature_sensor1_status);
  Serial.println("Temperature Sensor 2 Status: " + Temperature_sensor2_status);
  Serial.println("Temperature: " + String(Temperature));
  Serial.println("Peltier Status: " + Peltier_status);
  Serial.println("Fan Status: " + Fan_status);
  Serial.println("Last readingID: " + String(readingID));
  Serial.println("======================");
}




void setup() {
  Serial.begin(115200);
  startLoRa(); // Start LoRa
  
  setup_Tem_sensor(); // Temperature sensor setup

  // Initialize relay pins
  pinMode(RELAY_PELTIER, OUTPUT);
  pinMode(RELAY_FAN, OUTPUT);

  // Ensure relays are off at startup
  digitalWrite(RELAY_PELTIER, LOW);
  digitalWrite(RELAY_FAN, LOW);

  getTemp();
  Temp_control();
  sendReadings();
  delay(2000);

}

void loop() {
  unsigned long currentMillis = millis();

  // Handle test mode
  if (testMode) {
    // Wait for commands in test mode
    if (Serial.available()) {
      String input = Serial.readStringUntil('\n');
      input.trim(); // Remove leading and trailing whitespace

      if (input.equalsIgnoreCase("fan on")) {
        digitalWrite(RELAY_FAN, HIGH); // Turn on Fan (active LOW)
        Serial.println("Fan turned ON");
      } else if (input.equalsIgnoreCase("fan off")) {
        digitalWrite(RELAY_FAN, LOW); // Turn off Fan (active LOW)
        Serial.println("Fan turned OFF");
      } else if (input.equalsIgnoreCase("peltier on")) {
        digitalWrite(RELAY_PELTIER, HIGH); // Turn on Peltier (active LOW)
        Serial.println("Peltier turned ON");
      } else if (input.equalsIgnoreCase("peltier off")) {
        digitalWrite(RELAY_PELTIER, LOW); // Turn off Peltier (active LOW)
        Serial.println("Peltier turned OFF");
      } else if (input.equalsIgnoreCase("exit")) {
        testMode = false; // Exit test mode
        Serial.println("Exiting Test Mode.");
        // Ensure relays are off after exiting
        digitalWrite(RELAY_FAN, LOW);
        digitalWrite(RELAY_PELTIER, LOW);
      } else {
        Serial.println("Unknown command. Use 'fan on', 'fan off', 'peltier on', 'peltier off', or 'exit'.");
      }
    }
    return; // Exit loop early to prevent other logic from running
  }

  // Normal operation logic
  if (currentMillis - tempCheckPreviousMillis >= tempCheckInterval) {
    tempCheckPreviousMillis = currentMillis;
    getTemp();          // Get the temperature reading
    Temp_control();     // Adjust relays based on temperature
  }

  if (currentMillis - sendDataPreviousMillis >= sendDataInterval) {
    sendDataPreviousMillis = currentMillis;
    sendReadings();     // Send temperature data via LoRa
  }

  // Check for input to enter test mode
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim(); // Remove leading and trailing whitespace

    if (input.equalsIgnoreCase("test mode")) {
      testMode = true; // Enter test mode
      Serial.println("Entering Test Mode. Type 'exit' to leave.");
    }
  }
}
