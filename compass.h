// Compass
int HMC6352 = 0x21;

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
  
