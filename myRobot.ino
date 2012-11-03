
// Ultrasonic sensor on pin 10
#include <NewPing.h>
#include <Wire.h>
#include <hmc6352.h>

// Voltage measurement
unsigned long voltageTimer;
unsigned int voltageSpeed = 2500;
long tsVoltage;
// Motor voltage
#define VOLTS A3

// Compass
int HMC6352 = 0x21;
unsigned long headingTimer;
unsigned int headingSpeed = 100;

// We'll flash an LED as well
unsigned long LEDon, LEDoff;

#define TRIGGER_PIN  12  // Arduino pin tied to trigger pin on ping sensor.
#define ECHO_PIN     12  // Arduino pin tied to echo pin on ping sensor.
#define MAX_DISTANCE 200 // Maximum distance we want to ping for (in centimeters).

unsigned int pingSpeed = 100; // How frequently are we going to send out a ping (ms)
unsigned long pingTimer;     // Holds the next ping time.

// Flag indicating we have data + timestamp and data
int haveDistance = 0;
int theDistance;
long tsDistance;

// Flag indicating we have data + timestamp and data
int haveHeading = 0;
byte theHeading;
long tsHeading;

// Dallas 1-wire temperature sensor
#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into pin 3 on the Arduino
#define ONE_WIRE_BUS 3

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

// Assign the addresses of your 1-Wire temp sensors.
// See the tutorial on how to obtain these addresses:
// http://www.hacktronics.com/Tutorials/arduino-1-wire-address-finder.html

DeviceAddress Thermometer = { 0x28, 0x85, 0xC8, 0xAA, 0x03, 0x00, 0x00, 0x55 };

// Flags relating to temperature data
int haveTemperature = 0;
float Temperature;
long tsTemperature;
long TemperatureTimer;
int TemperatureSpeed = 10011;

int LEDpin = 13;

// Motor funtions
// Left
#define STBY 1
#define lAIN1 4
#define lAIN2 2
#define lBIN1 16
#define lBIN2 8
// Right
#define rAIN1 2
#define rAIN2 4
#define rBIN1 8
#define rBIN2 16

// Arduino pin numbers for motor PWM
#define PWM_lr 6    // (L)eft (R)ear etc.
#define PWM_lf 9
#define PWM_rr 10
#define PWM_rf 11

// PWM values
byte PWMlr=200, PWMlf=200, PWMrr = 200, PWMrf=200;

// Current values for each motor
byte LEFT = 0;
byte RIGHT = 0;

int count = 0;

long lastContact = 0;
long contactInterval = 5000;

// times to stop motors at (0 otherwise)
long StopMotors = 0;
long StopLeftMotor = 0;
long StopRightMotor = 0;

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

//set up hmc6352
Hmc6352 hmc6352;

// the setup routine runs once when you press reset:
void setup() {
  
  Serial.begin(115200);
  
  // Configure the 4 x PWM pins
  pinMode(PWM_lr, OUTPUT);
  pinMode(PWM_lf, OUTPUT);
  pinMode(PWM_rr, OUTPUT);
  pinMode(PWM_rf, OUTPUT);
  analogWrite(PWM_lr, PWMlr);
  analogWrite(PWM_lf, PWMlf);
  analogWrite(PWM_rr, PWMrr);
  analogWrite(PWM_rf, PWMrr);

  pingTimer = millis(); // Initiate distance sensor
  
  headingTimer = millis(); // Initiate compass
  
  // Configure Temperature sensor
  // Start up the library
  sensors.begin();
  // set the resolution to 10 bit (good enough?)
  sensors.setResolution(Thermometer, 10);
  TemperatureTimer = millis()+TemperatureSpeed;
  
  // Configure the MCP23017
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
  
  if ( haveHeading ) {
    haveHeading = 0;
    Serial.print("H ");
    Serial.print(theHeading);
    Serial.print(" ");
    Serial.println(tsHeading);
  }
  
  if ( haveTemperature ) {
    haveTemperature = 0;
    Serial.print("T ");
    Serial.print(Temperature);
    Serial.print(" ");
    Serial.println(tsTemperature);
  }  
  
  if ( millis() >= pingTimer) {   // pingSpeed milliseconds since last ping, do another ping.
    pingTimer += pingSpeed;      // Set the next ping time.
    sonar.ping_timer(echoCheck); // Send out the ping
  }

  if ( millis() >= headingTimer ) {
    hmc6352.wake();
    float a = hmc6352.getHeading()+0.5;
    hmc6352.sleep();
    theHeading = (int) a;
    haveHeading = 1;
    tsHeading = millis();
    headingTimer = tsHeading + headingSpeed;
  }
  
  if ( millis() >= TemperatureTimer ) {
    sensors.requestTemperatures();
    Temperature = sensors.getTempC(Thermometer);
    if (Temperature != -127.00) {
      haveTemperature = 1;
      tsTemperature = millis();
      TemperatureTimer = tsTemperature+TemperatureSpeed;
    }
  }
  
  if ( millis() >= voltageTimer ) {
    float volts = (float) analogRead(VOLTS);
    volts = (5.0*volts/1024.0)*10.5/4.73;
    Serial.print("V ");
    Serial.print(volts);
    tsVoltage = millis();
    Serial.print(" ");
    Serial.println(tsVoltage);
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

void writeLeft() {
//Serial.print("writeLeft ");
//Serial.println(LEFT, DEC);
  Wire.beginTransmission(0x20);
  Wire.write(0x12);
  Wire.write(LEFT);
  Wire.endTransmission();
}

void writeRight() {
//Serial.print("writeRight ");
//Serial.println(RIGHT, DEC);
  Wire.beginTransmission(0x20);
  Wire.write(0x13);
  Wire.write(RIGHT);
  Wire.endTransmission();
}  

void writeBoth() {
//Serial.print("writeBoth LEFT=");
//Serial.print(LEFT, DEC);
//Serial.print(" RIGHT=");
//Serial.println(RIGHT, DEC);
  Wire.beginTransmission(0x20);
  Wire.write(0x12);
  Wire.write(LEFT);
  Wire.write(RIGHT);
  Wire.endTransmission();
}  

void lforwards() {
  // Set STBY, AIN1 and BIN1 - both wheels forwards
  LEFT = STBY | lAIN1 | lBIN1;
  writeLeft();
}

void lreverse() {
  // Set STBY, AIN2 and BIN2 - both wheels reverse
  LEFT = STBY | lAIN2 | lBIN2;
  writeLeft();
}

void lstop() {
  // Clear all and power off the driver board
  LEFT = 0;
  writeLeft();
}

void lbrake() {
  // Set all motor bits to on
  LEFT = STBY | lAIN1 | lAIN2 | lBIN1 | lBIN2;
  writeLeft();
  StopLeftMotor = millis() + PWMlr + PWMlf;
}

void rforwards() {
  // Set STBY, AIN1 and BIN1 - both wheels forwards
  RIGHT = STBY | rAIN1 | rBIN1;
  writeRight();
}

void rreverse() {
  // Set STBY, AIN2 and BIN2 - both wheels reverse
  RIGHT = STBY | rAIN2 | rBIN2;
  writeRight();
}

void rstop() {
  // Clear all and power down
  RIGHT = 0;
  writeRight();
}

void rbrake() {
  // Set all motor bits to on
  RIGHT = STBY | rAIN1 | rAIN2 | rBIN1 | rBIN2;
  writeRight();
  StopRightMotor = millis() + PWMrr + PWMrf;
}

void bforwards() {
  // Set STBY, AIN1 and BIN1 - both wheels forwards
  LEFT = STBY | lAIN1 | lBIN1;
  RIGHT = STBY | rAIN1 | rBIN1;
  writeBoth();
}

void breverse() {
  // Set STBY, AIN2 and BIN2 - both wheels reverse
  LEFT = STBY | lAIN2 | lBIN2;
  RIGHT = STBY | rAIN2 | rBIN2;
  writeBoth();
}

void bstop() {
  // Clear all and power down
  LEFT = RIGHT = 0;
  writeBoth();
}

void bbrake() {
  // Set all motor bits to on
  LEFT = STBY | lAIN1 | lAIN2 | lBIN1 | lBIN2;
  RIGHT = STBY | rAIN1 | rAIN2 | rBIN1 | rBIN2;
  writeBoth();
  StopMotors = millis() + PWMlr + PWMlf + PWMrr + PWMrf;
}

void rotateCW() {
  LEFT = STBY | lAIN1 | lBIN1;
  RIGHT = STBY | rAIN2 | rBIN2;
  writeBoth();
}

void rotateCCW() {
  LEFT = STBY | lAIN2 | lBIN2;
  RIGHT = STBY | rAIN1 | rBIN1;
  writeBoth();
}
  
// PWM values
//byte PWMlr=50, PWMlf=50, PWMrr = 50, PWMrf=50;
void lplus() {
  PWMlr += 5;
  PWMlf += 5;
  if ( PWMlr > 255 ) {
    PWMlr = 255;
  }
  if ( PWMlf > 255 ) {
    PWMlf = 255;
  }
}
void rplus() {
  PWMrr += 5;
  PWMrf += 5;
  if ( PWMrr > 255 ) {
    PWMrr = 255;
  }
  if ( PWMrf > 255 ) {
    PWMrf = 255;
  }
}
void lminus() {
  PWMlr -= 5;
  PWMlf -= 5;
  if ( PWMlr < 0 ) {
    PWMlr = 0;
  }
  if ( PWMlf < 0 ) {
    PWMlf = 0;
  }
}
void rminus() {
  PWMrr -= 5;
  PWMrf -= 5;
  if ( PWMrr < 0 ) {
    PWMrr = 0;
  }
  if ( PWMrf < 0 ) {
    PWMrf = 0;
  }
}

void bminus() {
  lminus(); rminus();
}
void bplus() {
  lplus(); rplus();
}

void bpeq() {
   int newval = (PWMlr + PWMlf + PWMrr + PWMrf)/4;
   PWMlr = PWMlf = PWMrr = PWMrf = newval;
}
