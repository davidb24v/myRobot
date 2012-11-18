// Compass (This value is hard-wired in the hmc6352 library)
int HMC6352 = 0x30;

unsigned long headingTimer = 0;

void printHeading(Hmc6352 compass) {
  compass.wake();
  float heading = compass.getHeading();
  compass.sleep();
  long tsHeading = millis();
  Serial.print("H ");
  Serial.print(heading);
  Serial.print(" ");
  Serial.println(tsHeading);
}
  
