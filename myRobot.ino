
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

// We'll flash an LED as well
unsigned long LEDon, LEDoff;


int LEDpin = 13;

int count = 0;

long lastContact = 0;
long contactInterval = 5000;

// Setup ultrasonic sensor
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

//set up hmc6352
Hmc6352 hmc6352;

// the setup routine runs once when you press reset:
void setup() {
  
  Serial.begin(115200);
  
  setup_motors();
  
  pingTimer = millis(); // Initiate distance sensor
  
  headingTimer = millis(); // Initiate compass
  
  // Configure Temperature sensor
  // Start up the library
  sensors.begin();
  // set the resolution to 10 bit (good enough?)
  sensors.setResolution(Thermometer, 10);
  TemperatureTimer = millis()+TemperatureSpeed;
  
  // Configure the MCP23017
  setup_MCP23017();

  // Switch on LED in 3 seconds
  pinMode(LEDpin,OUTPUT);
  digitalWrite(LEDpin,LOW);
  LEDon = millis()+3000;
  LEDoff = LEDon+15;

  // Start measuring voltage
  voltageTimer = millis()+voltageSpeed;
}

// the loop routine runs over and over again forever:
void loop() {
  byte b1, b2, b3, b4;
  
  if ( millis() >= LEDon ) {
    digitalWrite(LEDpin,HIGH);
    LEDon += 1000;
  }
  
  if ( millis() >= LEDoff ) {
    digitalWrite(LEDpin,LOW);
    LEDoff = LEDon+15;
  }
  
  if ( haveDistance ) {
    haveDistance = 0;
    Serial.print("D ");
    Serial.print(theDistance);
    Serial.print(" ");
    Serial.println(tsDistance);
  }
  
  if ( millis() >= pingTimer) {   // pingSpeed milliseconds since last ping, do another ping.
    pingTimer += pingSpeed;      // Set the next ping time.
    sonar.ping_timer(echoCheck); // Send out the ping
  }

  if ( millis() >= headingTimer ) {
    printHeading(hmc6352);
    headingTimer += headingSpeed;
  }
  
  if ( millis() >= TemperatureTimer ) {
    printTemperature();
    TemperatureTimer += TemperatureSpeed;
  }
  
  if ( millis() >= voltageTimer ) {
    printVoltage();
    voltageTimer = tsVoltage + voltageSpeed;
  }
    

  // Check if any motors need stopping
  if ( StopMotors > 0 ) {
    if ( millis() >= StopMotors ) {
      StopMotors = 0;
      bstop();
    }
  }
  if ( StopLeftMotor > 0 ) {
    if ( millis() >= StopLeftMotor ) {
      StopLeftMotor = 0;
      lstop();
    }
  }
  if ( StopRightMotor > 0 ) {
    if ( millis() >= StopRightMotor ) {
      StopRightMotor = 0;
      rstop();
    }
  }
  
  if (Serial.available() > 0) {
    lastContact = millis();
    b1 = Serial.read();
    switch (b1) {
      case 98:  // "b" = both
        b2 = Serial.read();
        if (b2 == 102)  {  //forward
          bforwards();
        } else if (b2 == 114) {  //reverse
          breverse();
        } else if (b2 == 115) {   //stop
          bstop();
        } else if (b2 == 98) {   //brake
          bbrake();
        } else {
          // bp    //pwm
          b3 = Serial.read();
          if (b3 == 112) {  //increment
            bplus();
          } else if (b3 == 109) {  //decrement
            bminus();
          } else {  //make equal
            // bpeq
            b4 = Serial.read();
            bpeq();
          }
        }
        break;
        
      case 108:  //left
        b2 = Serial.read();
        if (b2 == 102) {  //forward
          lforwards();
        } else if (b2 == 114) {  //reverse
          lreverse();
        } else if (b2 == 115) {  //stop
          lstop();
        } else if (b2 == 98) {  //brake
          lbrake();
        } else if (b2 == 82) {  //rotate to left
          rotateCCW();
        } else {  //pwm
          b3 = Serial.read();
          if (b3 == 112) {
            lplus();
          } else {
            lminus();
          }
        }
        break;
        
      case 114:
        b2 = Serial.read();
        if (b2 == 102) {
          rforwards();
        } else if (b2 == 114) {
          rreverse();
        } else if (b2 == 115) {
          rstop();
        } else if (b2 == 98) {
          rbrake();
        } else if (b2 == 82) {
          rotateCW();
        } else {  //pwm
          b3 = Serial.read();
          if (b3 == 112) {
            rplus();
          } else {
            rminus();
          }
        }
        break;
      }
    }
        
  // turn off if haven't heard from pi for a while...
  if ( millis()-lastContact > contactInterval) {
    lastContact = millis();
    bstop();
  }
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

