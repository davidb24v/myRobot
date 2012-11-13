
void setup_MCP23017() {
  // Configure the MCP23017
  Wire.begin();

  // Initialise motor driver IC
  Wire.beginTransmission(0x20);
  Wire.write(0x00); // IODIRA register
  Wire.write(0x00); // set all of bank A to outputs
  Wire.write(0x00); // set all of bank B to outputs
  Wire.endTransmission();
  
  // Initialise stepper driver IC
  Wire.beginTransmission(0x21);
  Wire.write(0x00); // IODIRA register
  Wire.write(0x00); // set all of bank A to outputs
  Wire.write(0x01); // set all of bank B to output, 0 as input
  Wire.endTransmission();
  
}
