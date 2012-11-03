#define TRIGGER_PIN  12  // Arduino pin tied to trigger pin on ping sensor.
#define ECHO_PIN     12  // Arduino pin tied to echo pin on ping sensor.
#define MAX_DISTANCE 200 // Maximum distance we want to ping for (in centimeters).

unsigned int pingSpeed = 100; // How frequently are we going to send out a ping (ms)
unsigned long pingTimer = 0;     // Holds the next ping time.

// Flag indicating we have data + timestamp and data
int haveDistance = 0;
int theDistance;
long tsDistance;

void printDistance() {
  Serial.print("D ");
  Serial.print(theDistance);
  Serial.print(" ");
  Serial.println(tsDistance);
}
