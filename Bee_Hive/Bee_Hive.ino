// For Temperature sensor
#include <OneWire.h>
#include <DallasTemperature.h>
// For LoRa
#include <SPI.h>
#include <LoRa.h>
#define BAND 920E6
#define SS 10 // chip select pin
#define RST 9 // Reset pin
#define DIO0 2 // Interrupt pin (DIO0)


// Pin definitions
#define ONE_WIRE_BUS 2 // DS18B20 data pin
#define RELAY_PELTIER 8 // Relay for Peltier
#define RELAY_FAN 9 // Realy for Fan

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
unsigned long previousMillis = 0;
unsigned long interval = 900000; // Default delay 15 minutes

// Flags
bool tempThresholdExceeded = false;



void startLoRa() {
  int counter = 0;
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
  }
  if (!sensors.getAddress(sensor2, 1)) {
    Serial.println("Unable to find address for Sensor 2");
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
    LoRa.beginPacket();
    LoRa.print(LoRaMessage);
    LoRa.endPacket();
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
    Temperature = (temp1 + temp2)/2;
  }

}



void Temp_control() {
  if (Temperature >= TEMP_HIGH_THRESOLD) {
    digitalWrite(RELAY_PELTIER, HIGH);
    digitalWrite(RELAY_FAN, HIGH);
    interval = 2000;                  // realtime updates (2 second)
    Peltier_status = "ON";
    Fan_status = "ON";
    if (!tempThresholdExceeded) {
      tempThresholdExceeded = true;
      sendReadings("High Temperature!"); // Immediate alert
    }
  } else if (Temperature <= TEMP_LOW_THRESOLD) {
    digitalWrite(RELAY_PELTIER, LOW);
    digitalWrite(RELAY_FAN, LOW);
    interval = 900000;                // Back to 15 min updates
    Peltier_status = "OFF";
    Fan_status = "OFF";
    tempThresholdExceeded = false;
  }
}

void debugStatus() {
  Serial.println("==== DEBUG STATUS ====");
  Serial.println("LoRa Status: " + LoRa_status);
  Serial.println("LoRa skip message count: " + LoRa_Skip_Message);
  Serial.println("Temperature Sensor 1 Status: " + Temperature_sensor1_status);
  Serial.println("Temperature Sensor 2 Status: " + Temperature_sensor2_status);
  Serial.println("Temperature: " + String(Temperature));
  Serial.println("Peltier Status: " + Peltier_status);
  Serial.println("Fan Status: " + Fan_status);
  Serial.println("Last readingID: " + readingID);
  Serial.println("======================");
}




void setup() {
  Serial.begin(9600);
  startLoRa(); // Start LoRa
  
  setup_Tem_sensor(); // Temperature sensor setup

  // Initialize relay pins
  pinMode(RELAY_PELTIER, OUTPUT);
  pinMode(RELAY_FAN, OUTPUT);

  // Ensure relays are off at startup
  digitalWrite(RELAY_PELTIER, LOW);
  digitalWrite(RELAY_FAN, LOW);

}

void loop() {

  unsigned long currentMillis = millis();
  
  // Only check temperture at specified intervals
  if (currentMillis - previousMillis >= interval || tempThresholdExceeded) {
    previousMillis = currentMillis;
    getTemp();
    Temp_control();
    if (!tempThresholdExceeded) {
      sendReadings();
    }
  }

  // Check for input from the Serial Monitor
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim(); // Remove leading and trailing whitespace
    if (input.equalsIgnoreCase("status")) {
      debugStatus(); // Call debug function
    }
  }
}
