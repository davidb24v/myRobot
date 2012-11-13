
// Ultrasonic sensor on pin 10
#include <NewPing.h>
#include <Wire.h>
#include <hmc6352.h>
// Dallas 1-wire temperature sensor
#include <OneWire.h>
#include <DallasTemperature.h>

#include "motors.h"
#include "voltage.h"
#include "compass.h"
#include "distance.h"
#include "MCP23017.h"
#include "temperature.h"
#include "led.h"

int count = 0;

long lastContact = 0;
long contactInterval = 5000;

// Setup ultrasonic sensor
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

//set up hmc6352
Hmc6352 hmc6352;

// Command received from Raspberry Pi
String command;

// Polling interval for heading and distance
int Speed = 100;

// Values for stepper motor
int stepperSpeed = 1;
byte stepperValueA = 0;
byte stopPin = 0;
int cwPos, ccwPos, pos, centrePos;
int nSteps = 1;
#include "myStepper.h"

// the setup routine runs once when you press reset:
void setup() {
  
  Serial.begin(115200);
//  Serial.begin(9600);
  
  setup_motors();
  
  // Configure Temperature sensor
  // Start up the library
  sensors.begin();
  // set the resolution to 10 bit (good enough?)
  sensors.setResolution(Thermometer, 10);
  
  // Configure the MCP23017
  setup_MCP23017();

  // Switch on flashing LED 
  pinMode(LEDpin,OUTPUT);
  digitalWrite(LEDpin,LOW);
  LEDon = millis();
  LEDoff = LEDon+LEDontime;
  
  // Calibrate sensor position
  calibrateStepper();
  
}

// the loop routine runs over and over again forever:
void loop() {
  char ch;
  
  if ( haveDistance ) {
    printDistance();
    haveDistance = 0;
  }
  
  // pingSpeed milliseconds since last ping, do another ping.
  if ( pingTimer && millis() >= pingTimer) {
    sonar.ping_timer(echoCheck); // Send out the ping
    pingTimer += Speed;      // Set the next ping time.
  }

  if ( headingTimer && millis() >= headingTimer ) {
    printHeading(hmc6352);
    headingTimer += Speed;
  }
  
  if ( LEDon && millis() >= LEDon ) {
    digitalWrite(LEDpin,HIGH);
    LEDon += LEDinterval;
  }
  
  if ( LEDoff && millis() >= LEDoff ) {
    digitalWrite(LEDpin,LOW);
    LEDoff = LEDon+LEDontime;
  }
  
  // Check if any motors need stopping
  if ( StopMotors && millis() >= StopMotors ) {
    StopMotors = 0;
    bs();
  }
  if ( StopLeftMotor && millis() >= StopLeftMotor ) {
    StopLeftMotor = 0;
    ls();
  }
  if ( StopRightMotor > 0 && millis() >= StopRightMotor ) {
    StopRightMotor = 0;
    rs();
  }
  
  if (Serial.available() > 0) {
    lastContact = millis();
    ch = Serial.read();
    if (String(ch) == "\n") {
      process_command();
    } else {
      command += ch;
    }
  }
        
  // turn off if haven't heard from pi for a while...
/*  if ( millis()-lastContact > contactInterval) {
    lastContact = millis();
    bs();
    idle_mode();
  }
*/
}

void process_command() {
  if (command == "h") get_heading();             // query heading
  else if (command == "d") get_distance();       // query distance sensor
  else if (command == "v") get_voltage();        // query motor voltage
  else if (command == "t") get_temperature();    // query motor voltage
  else if (command == "s") scan_mode();       // Fast scanning distance/heading
  else if (command == "i") idle_mode();       // Idle mode
  else if (command == "lf") lf();  //left forwards
  else if (command == "rf") rf();  //right forwards
  else if (command == "bf") bf();  //both forwards
  else if (command == "lr") lr();    //
  else if (command == "rr") rr();    // reverse
  else if (command == "br") br();    //
  else if (command == "lb") lb();	     //
  else if (command == "rb") rb();	     // brake
  else if (command == "bb") bb();	     //
  else if (command == "ls") ls();	      //
  else if (command == "rs") rs();	      // stop
  else if (command == "bs") bs();	      //
  else if (command == "cw") rotateCW();    // rotate clockwise
  else if (command == "ccw") rotateCCW();  // rotate counter-clockwise
  else if (command == "pl") lp();	      // Add 5 to left PWM values
  else if (command == "pr") rp();	      //   to right
  else if (command == "pb") bp();	      //   to both
  else if (command == "ml") lm();        // subtract 5 from left PWM
  else if (command == "mr ") rm();        //   from right
  else if (command == "mb ") bm();        //   from both
  else if (command == "sl") lset();	       // set left PWM
  else if (command == "sr") rset();	       // set right
  else if (command == "sb") bset();	       // set both
  else if (command == "beq") beq();              // make left and right PWM equal
  else if (command == "cs") calibrateStepper();  // what it says on the tin

  command = "";
}

void get_heading() {
  printHeading(hmc6352);
}

void get_distance() {
  sonar.ping_timer(echoCheck);
}

void get_voltage() {
  printVoltage();
}

void get_temperature() {
  printTemperature();
}

void idle_mode() {
  LEDinterval = 1000;
  pingTimer = headingTimer = 0;
}

void scan_mode() {
  // Fast scanning, speed up the LED
  LEDinterval = 100;
  // Get heading as soon as possible
  headingTimer = millis();
  // Offset distance in time by half a "tick"
  pingTimer = headingTimer + Speed/2;
}

void echoCheck() { // Timer2 interrupt calls this function every 24uS where you can check the ping status.
  // Don't do anything here!
  if (sonar.check_timer()) { // This is how you check to see if the ping was received.
    // Here's where you can add code.
    tsDistance = millis();
    theDistance = (sonar.ping_result / US_ROUNDTRIP_CM);
    haveDistance = 1;
  }
  // Don't do anything here!
}


