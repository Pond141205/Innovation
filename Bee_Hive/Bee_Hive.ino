#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is connected to digital pin 2
#define ONE_WIRE_BUS 2

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass oneWire reference to DallasTemperature library
DallasTemperature sensors(&oneWire);

// Addresses of the DS18B20 sensors
DeviceAddress sensor1, sensor2;


void setup_Tem_sensor() {
  Serial.println("DS18B20 Sensor Test.");
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

void getTemp() {

  // Request temperature
  sensor.requestTemperatures();

  // Initialize status string
  String Temperature_sensor1_status = "Active";
  String Temperature_sensor2_status = "Active";

  // Read Temperature from sensor
  float temp1 = sensors.getTempC(sensor1);
  float temp2 = sensors.getTempC(sensor2);

  float Temperature ; 
  if (temp1 == DEVICE_DISCONNECTED_C && temp2 != DEVICE_DISCONNECTED_C)  {
    Temperature_sensor1_status = "Disconnected";
    Temperature_sensor2_status = "Active"
    Temperature = temp2;
  } else if (temp2 == DEVICE_DISCONNECTED_C && temp1 != DEVICE_DISCONNECTED_C) {
    Temperature_sensor2_status = "Disconnected";
    Temperature_sensor1_status = "Active"
    Temperature = temp1;
  } else if (temp1 == DEVICE_DISCONNECTED_C && temp2 == DEVICE_DISCONNECTED_C) {
    Temperature_sensor1_status = "Disconnnected";
    Temperature_sensor2_status = "Disconneected";
    Temperature = -999;
  } else {
    Temperature = (temp1 + temp2)/2;
  }

  // For debugging
  Serial.print("Sensor 1 Status: ");
  Serial.println(Temperature_sensor1_status);
  Serial.print("Sensor 2 Status: ");
  Serial.println(Temperature_sensor2_status);
  Serial.print("Temperature: ");
  if (Temperature == -999) {
    Serial.println("Error");
  } else {
    Serial.print(Temperature);
    Serial.println(" °C");
  }

}

  











void setup() {
  Serial.begin(9600);
  
  setup_Tem_sensor(); // Temperature sensor setup

}

void loop() {

  // Wait 2 seconds before the next reading
  delay(2000);
  getTemp();
}
