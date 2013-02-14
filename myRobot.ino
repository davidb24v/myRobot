
// Ultrasonic sensor on pin 10
#include <NewPing.h>
#include <Wire.h>
#include <hmc6352.h>
#include <MemoryFree.h>

#include <SoftwareSerial.h>
SoftwareSerial LEFT(6,8);
SoftwareSerial RIGHT(5,7);
char theMotor = '\0';
int motorSpeed = 0;
int leftSpeed = 0;
int rightSpeed = 0;
long lastMotorUpdate = 0;

#include "motors.h"

// Compass calibration mode
int ccMode = 0;
int ccStart;

#include "compass.h"
#include "distance.h"

float heading;
long tsHeading;

int count = 0;

long lastContact = 0;
long contactInterval = 5000;

// Setup ultrasonic sensor
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

//set up hmc6352/
Hmc6352 hmc6352;

// Command received from Raspberry Pi
String command;
int nchars = 0;

// Polling interval for heading and distance
int Speed = 500;

// Values for stepper motor
int stepperSpeed = 750;  // now in microseconds
#define STEPPER_DIR_PIN 13
#define STEPPER_STEP_PIN 12
int cwPos, ccwPos, centrePos;
int pos = 0;
int stopPin = 11;
#include "myStepper.h"

#include "blinkM.h"

void setup() {
  
  blinkM_off();
  
  Serial.begin(115200);
  
  // setup motors
  LEFT.begin(19200);
  RIGHT.begin(19200);
  
  LEFT.write("A");
  RIGHT.write("A");
  
}

// the loop routine runs over and over again forever:
void loop() {
  char ch;

  if ( theMotor != '\0' && millis()-lastMotorUpdate > 200 ) {
    set_motor_speed('L',leftSpeed);
    set_motor_speed('R',rightSpeed);
    lastMotorUpdate = millis();
  }

  if ( ccMode ) {
    if ( millis()-ccStart > 20001 ) {
      // Send "E" to finish "user calibration"
      Wire.beginTransmission(0x30);
      Wire.write(0x45);
      Wire.endTransmission();
      // STOP MOTORS
      ccMode = 0;
    }
  }
  
  if ( haveDistance ) {
    Serial.print(theDistance);
    Serial.print(" ");
    Serial.print(pos);
    Serial.print(" ");
    Serial.println(tsDistance);
    haveDistance = 0;
  }
  
  if (Serial.available() > 0) {
    ch = Serial.read();
    if (String(ch) == "\n") {
      process_command();
      command = "";
      nchars = 0;
    } else {
      command += ch;
      nchars++;
      if ( nchars > 10 ) {
        command = "";
        nchars = 0;
      }
    }
  }
}

void motorCmd(char motor) {
  int dir, value;

  if ( command.charAt(1) == '-' ) dir = -1;
  else dir = 1;
  
  value = 0;
  for (int n=2; n < 5; n++) {
    value *= 10;
    value += command.charAt(n)-'0';
  }
  
  value *= dir;
  value = max(-255,min(value,255));
  
  theMotor = motor;
  motorSpeed = value;
  setSpeed(motor,value);
}
  
void process_command() {
  if (command.charAt(0) == 'L' && nchars==5) { 
    motorCmd('L');
    leftSpeed = motorSpeed;
  }  else if (command.charAt(0) == 'R' && nchars==5) {
    motorCmd('R');
    rightSpeed = motorSpeed;
  }  else if (command.charAt(0) == 'B' && nchars==5) { 
    motorCmd('B');
    leftSpeed = motorSpeed;
    rightSpeed = motorSpeed;
  } else if (command == "CS") {
    calibrateStepper();
  } else if (command == "free") {
    Serial.println(freeRam());
  }  else if (command == "SP") {
    int newPos = readInt();
    setPos(newPos);
  }  else if (command == "led") {
    int r = readInt();
    int g = readInt();
    int b = readInt();
    blinkM_set_rgb_colour(r,g,b);
  }  else if (command == "ms") {
    Serial.println(millis());
  } else if ( command == "h" | command == "H" ) {
    hmc6352.wake();
    heading = hmc6352.getHeading();
    hmc6352.sleep();
    tsHeading = millis();
    Serial.print(heading);
    Serial.print(" ");
    Serial.println(tsHeading);
  } else if ( command == "p" ) {
    sonar.ping_timer(echoCheck); // Send out the ping
  } else if ( command == "P" ) {
    int newPos = readInt();
    setPos(newPos);
    sonar.ping_timer(echoCheck); // Send out the ping
  } else if ( command == "idle" ) {
    blinkM_idle();
  } else if ( command == "off" ) {
    blinkM_off();
  } else if ( command == "amber" ) {
    blinkM_orange_flash();
    Serial.println("arduino amber command");
  }
}

int readInt() {
  command = "";
  nchars = 0;
  char ch;
  for(;;) {
    if ( Serial.available() > 0 ) {
      ch = Serial.read();
      if ( ch == '\n' ) {
        break;
      }
      command += ch;
      nchars++;
    }
  }
  int value = 0;
  int start = 0;
  int sign = 1;
  if ( command[0] == '-' ) {
    start = 1;
    sign = -1;
  }
  for (int i=start; i < nchars; i++) {
    value *= 10;
    value += command[i]-'0';
  }
  value *= sign;
  command = "";
  nchars = 0;
  return value;
}

void setSpeed(char motor, int speed) {
  set_motor_speed(motor,speed);
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

int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}
