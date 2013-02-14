#define TRIGGER_PIN  9  // Arduino pin tied to trigger pin on ping sensor.
#define ECHO_PIN     9  // Arduino pin tied to echo pin on ping sensor.
#define MAX_DISTANCE 200 // Maximum distance we want to ping for (in centimeters).

unsigned long pingTimer = 1;     // Holds the next ping time.

// Flag indicating we have data + timestamp and data
int haveDistance = 0;
int theDistance;
long tsDistance;


