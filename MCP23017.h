
void setup_MCP23017() {
  // Configure the MCP23017
  setup_MCP23017();
  Wire.beginTransmission(0x20);
  Wire.write(0x12);
  Wire.write(0x20); // use table 1.4 addressing
  Wire.endTransmission();
  delay(50);
  Wire.beginTransmission(0x20);
  Wire.write(0x00); // IODIRA register
  Wire.write(0x00); // set all of bank A to outputs
  Wire.write(0x00); // set all of bank B to outputs
  Wire.endTransmission();
}
