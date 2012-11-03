// Voltage measurement
unsigned long voltageTimer;
unsigned int voltageSpeed = 2500;
long tsVoltage;

// Motor voltage on analog pin 3 (via a resistive divider)
#define VOLTS A3

void printVoltage() {
  float volts = (float) analogRead(VOLTS);
  volts = (5.0*volts/1024.0)*10.5/4.73;
  Serial.print("V ");
  Serial.print(volts);
  tsVoltage = millis();
  Serial.print(" ");
  Serial.println(tsVoltage);
}
