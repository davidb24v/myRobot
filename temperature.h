// Data wire is plugged into pin 3 on the Arduino
#define ONE_WIRE_BUS 3

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

// Assign the addresses of your 1-Wire temp sensors.
// See the tutorial on how to obtain these addresses:
// http://www.hacktronics.com/Tutorials/arduino-1-wire-address-finder.html

DeviceAddress Thermometer = { 0x28, 0x85, 0xC8, 0xAA, 0x03, 0x00, 0x00, 0x55 };

// Flags relating to temperature data
long TemperatureTimer = 0;
int TemperatureSpeed = 1000;

void printTemperature() {
  sensors.requestTemperatures();
  float Temperature = sensors.getTempC(Thermometer);
  Serial.print("T ");
  Serial.print(Temperature);
  Serial.print(" ");
  Serial.println(millis());
}
