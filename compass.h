// Compass
int HMC6352 = 0x21;

unsigned long headingTimer;
unsigned int headingSpeed = 100;

void printHeading(Hmc6352 compass) {
  compass.wake();
  float heading = compass.getHeading()+0.5;
  compass.sleep();
  long tsHeading = millis();
  Serial.print("H ");
  Serial.print(heading);
  Serial.print(" ");
  Serial.println(tsHeading);
}
  
