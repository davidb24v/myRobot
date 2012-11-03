
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

// the setup routine runs once when you press reset:
void setup() {
  
  Serial.begin(115200);
  
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
    pingTimer += pingSpeed;      // Set the next ping time.
  }

  if ( headingTimer && millis() >= headingTimer ) {
    printHeading(hmc6352);
    headingTimer += headingSpeed;
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
    bstop();
  }
  if ( StopLeftMotor && millis() >= StopLeftMotor ) {
    StopLeftMotor = 0;
    lstop();
  }
  if ( StopRightMotor > 0 && millis() >= StopRightMotor ) {
    StopRightMotor = 0;
    rstop();
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
  if ( millis()-lastContact > contactInterval) {
    lastContact = millis();
    bstop();
    idle_mode();
  }
}

void process_command() {
  if (command == "H") get_heading();             // query heading
  else if (command == "D") get_distance();       // query distance sensor
  else if (command == "V") get_voltage();        // query motor voltage
  else if (command == "T") get_temperature();    // query motor voltage
  else if (command == "S") scan_mode();       // Fast scanning distance/heading
  else if (command == "I") idle_mode();       // Idle mode
  else if (command == "lf") lf();  //left forwards
  else if (command == "rf") rf();  //right forwards
  else if (command == "bf") bf();  //both forwards
  else if (command == "lr") lreverse();    //
  else if (command == "rr") rreverse();    // reverse
  else if (command == "br") breverse();    //
  else if (command == "lb") lbrake();	     //
  else if (command == "rb") rbrake();	     // brake
  else if (command == "bb") bbrake();	     //
  else if (command == "ls") lstop();	      //
  else if (command == "rs") rstop();	      // stop
  else if (command == "bs") bstop();	      //
  else if (command == "CW") rotateCW();    // rotate clockwise
  else if (command == "CCW") rotateCCW();  // rotate counter-clockwise
  else if (command == "lp") lplus();	      // Add 5 to left PWM values
  else if (command == "rp") rplus();	      //   to right
  else if (command == "bp") bplus();	      //   to both
  else if (command == "lm=") lminus();        // subtract 5 from left PWM
  else if (command == "rm ") rminus();        //   from right
  else if (command == "bm ") bminus();        //   from both
  else if (command == "ls") lset();	       // set left PWM
  else if (command == "rs") rset();	       // set right
  else if (command == "bs") bset();	       // set both
  else if (command == "beq") beq();              // make left and right PWM equal

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
  LEDinterval = 100;
  pingTimer = headingTimer = millis();
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


