// DS18B20 sensor connection check code .To see if its working or not...

#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 4  // Data wire is connected to GPIO 4


OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress sensorAddress; // Variable to hold the sensor's unique ID

void setup() {
  Serial.begin(115200);
  delay(1000); 
  
  Serial.println("--- DS18B20 Connection Check ---");
  sensors.begin();

  // Count how many sensors are on the wire
  int deviceCount = sensors.getDeviceCount();
  
  Serial.print("Searching for sensors...");
  Serial.print("Found ");
  Serial.print(deviceCount);
  Serial.println(" sensor(s).");

  if (deviceCount > 0) {
    if (sensors.getAddress(sensorAddress, 0)) {
      Serial.println("SUCCESS: Sensor found and responding!");
      Serial.print("Sensor Address: ");
      printAddress(sensorAddress);
      Serial.println();
    } else {
      Serial.println("ERROR: Found a device but could not retrieve address.");
    }
  } else {
    Serial.println("FAILED: No DS18B20 detected.");
    Serial.println("Check: 1. Power (3.3V), 2. Ground, 3. Data pin (GPIO 4), 4. Pull-up resistor (4.7k).");
  }
}

void loop() {
  // We only need to run the check once in Setup. 
  // If it works there, you're good to go!
}

// Helper function to print the sensor address in Hexadecimal
void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++) {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}


       

