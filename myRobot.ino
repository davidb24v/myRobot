// I2C device class (I2Cdev) demonstration Arduino sketch for MPU6050 class using DMP (MotionApps v2.0)
// 6/21/2012 by Jeff Rowberg <jeff@rowberg.net>
// Updates should (hopefully) always be available at https://github.com/jrowberg/i2cdevlib
//
// Changelog:
//     2012-06-21 - added note about Arduino 1.0.1 + Leonardo compatibility error
//     2012-06-20 - improved FIFO overflow handling and simplified read process
//     2012-06-19 - completely rearranged DMP initialization code and simplification
//     2012-06-13 - pull gyro and accel data from FIFO packet instead of reading directly
//     2012-06-09 - fix broken FIFO read sequence and change interrupt detection to RISING
//     2012-06-05 - add gravity-compensated initial reference frame acceleration output
//                - add 3D math helper file to DMP6 example sketch
//                - add Euler output and Yaw/Pitch/Roll output formats
//     2012-06-04 - remove accel offset clearing for better results (thanks Sungon Lee)
//     2012-06-01 - fixed gyro sensitivity to be 2000 deg/sec instead of 250
//     2012-05-30 - basic DMP initialization working

/* ============================================
I2Cdev device library code is placed under the MIT license
Copyright (c) 2012 Jeff Rowberg

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
===============================================
*/

// Arduino Wire library is required if I2Cdev I2CDEV_ARDUINO_WIRE implementation
// is used in I2Cdev.h
#include <Wire.h>

// I2Cdev and MPU6050 must be installed as libraries, or else the .cpp/.h files
// for both classes must be in the include path of your project
#include <I2Cdev.h>
#include <MemoryFree.h>

#include "MPU6050_6Axis_MotionApps20.h"

// Ultrasonic sensor on pin 10
#include <NewPing.h>
#include <MemoryFree.h>

#include <SendOnlySoftwareSerial.h>
SendOnlySoftwareSerial LEFT(8);
SendOnlySoftwareSerial RIGHT(7);
char theMotor = '\0';
int motorSpeed = 0;
int leftSpeed = 0;
int rightSpeed = 0;

#include "motors.h"

float heading;
long tsHeading;

// Setup ultrasonic sensor
#define TRIGGER_PIN  9  // Arduino pin tied to trigger pin on ping sensor.
#define ECHO_PIN     9  // Arduino pin tied to echo pin on ping sensor.
#define MAX_DISTANCE 200 // Maximum distance we want to ping for (in centimeters).
#define SONAR_NUM 5
byte currentSensor = 0;

unsigned long pingTimer = 1;     // Holds the next ping time.

// Flag indicating we have data + timestamp and data
int haveDistance = 0;
int theDistance;
long tsDistance;
NewPing sonar[SONAR_NUM] = {
  NewPing(TRIGGER_PIN+0, ECHO_PIN+0, MAX_DISTANCE),
  NewPing(TRIGGER_PIN+1, ECHO_PIN+1, MAX_DISTANCE),
  NewPing(TRIGGER_PIN+2, ECHO_PIN+2, MAX_DISTANCE),
  NewPing(TRIGGER_PIN+3, ECHO_PIN+3, MAX_DISTANCE),
  NewPing(TRIGGER_PIN+4, ECHO_PIN+4, MAX_DISTANCE)
};

// Command received from Raspberry Pi
String command;
int nchars = 0;

#include "blinkM.h"

int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

// class default I2C address is 0x68
// specific I2C addresses may be passed as a parameter here
// AD0 low = 0x68 (default for SparkFun breakout and InvenSense evaluation board)
// AD0 high = 0x69
MPU6050 mpu;

/* =========================================================================
   NOTE: In addition to connection 3.3v, GND, SDA, and SCL, this sketch
   depends on the MPU-6050's INT pin being connected to the Arduino's
   external interrupt #0 pin. On the Arduino Uno and Mega 2560, this is
   digital I/O pin 2.
 * ========================================================================= */

/* =========================================================================
   NOTE: Arduino v1.0.1 with the Leonardo board generates a compile error
   when using Serial.write(buf, len). The Teapot output uses this method.
   The solution requires a modification to the Arduino USBAPI.h file, which
   is fortunately simple, but annoying. This will be fixed in the next IDE
   release. For more info, see these links:

   http://arduino.cc/forum/index.php/topic,109987.0.html
   http://code.google.com/p/arduino/issues/detail?id=958
 * ========================================================================= */


#define LED_PIN 3 // (Arduino is 13, Teensy is 11, Teensy++ is 6)
bool blinkState = false;

// MPU control/status vars
bool dmpReady = false;  // set true if DMP init was successful
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer

// orientation/motion vars
Quaternion q;           // [w, x, y, z]         quaternion container
VectorFloat gravity;    // [x, y, z]            gravity vector
float euler[3];         // [psi, theta, phi]    Euler angle container
float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

float lastAngle = 0.0;
float yaw = 0.0;
bool settled = false;
int count = 0;

// packet structure for InvenSense teapot demo
uint8_t teapotPacket[14] = { '$', 0x02, 0,0, 0,0, 0,0, 0,0, 0x00, 0x00, '\r', '\n' };



// ================================================================
// ===               INTERRUPT DETECTION ROUTINE                ===
// ================================================================

volatile bool mpuInterrupt = false;     // indicates whether MPU interrupt pin has gone high
void dmpDataReady() {
    mpuInterrupt = true;
}



void setSpeed(char motor, int speed) {
  set_motor_speed(motor,speed);
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

void echoCheck() { // Timer2 interrupt calls this function every 24uS where you can check the ping status.
  // Don't do anything here!
  if (sonar[currentSensor].check_timer()) { // This is how you check to see if the ping was received.
    // Here's where you can add code.
    tsDistance = millis();
    theDistance = (sonar[currentSensor].ping_result / US_ROUNDTRIP_CM);
    haveDistance = 1;
  }
  // Don't do anything here!
}

  
void process_command() {
  digitalWrite(LED_PIN,LOW);
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
  } else if (command == "free") {
    Serial.println(freeRam());
  }  else if (command == "led") {
    int r = readInt();
    int g = readInt();
    int b = readInt();
    blinkM_set_rgb_colour(r,g,b);
  }  else if (command == "ms") {
    Serial.println(millis());
  } else if ( command == "p" | command == "P" ) {
    byte sensor = readInt();
    if ( sensor != currentSensor ) sonar[currentSensor].timer_stop();
    currentSensor = sensor;
    sonar[currentSensor].ping_timer(echoCheck);
  } else if ( command == "yaw" ) {
    Serial.println(yaw);
    yaw = 0.0;
  } else if ( command == "idle" ) {
    blinkM_idle();
  } else if ( command == "off" ) {
    blinkM_off();
  } else if ( command == "amber" ) {
    blinkM_orange_flash();
  } else if ( command == "red" ) {
    blinkM_red_flash();
  } else if ( command == "redblue" ) {
    blinkM_redblue_flash();
  }
}
// ================================================================
// ===                      INITIAL SETUP                       ===
// ================================================================

void setup() {
    // join I2C bus (I2Cdev library doesn't do this automatically)
    Wire.begin();

    // initialize serial communication
    // (115200 chosen because it is required for Teapot Demo output, but it's
    // really up to you depending on your project)
    Serial.begin(115200);
    while (!Serial); // wait for Leonardo enumeration, others continue immediately

    blinkM_off();
  
    // setup motors
    LEFT.begin(19200);
    RIGHT.begin(19200);
  
    LEFT.write("A");
    RIGHT.write("A");
    
    // NOTE: 8MHz or slower host processors, like the Teensy @ 3.3v or Ardunio
    // Pro Mini running at 3.3v, cannot handle this baud rate reliably due to
    // the baud timing being too misaligned with processor ticks. You must use
    // 38400 or slower in these cases, or use some kind of external separate
    // crystal solution for the UART timer.

    // initialize device
    mpu.initialize();

    // verify connection
    if ( !mpu.testConnection() ) {
      Serial.println(F("MPU6050 connection failed"));
    }

    // wait for ready
    delay(1000);

    // load and configure the DMP
    devStatus = mpu.dmpInitialize();
    
    // make sure it worked (returns 0 if so)
    if (devStatus == 0) {
        // turn on the DMP, now that it's ready
        mpu.setDMPEnabled(true);

        // enable Arduino interrupt detection
        attachInterrupt(0, dmpDataReady, RISING);
        mpuIntStatus = mpu.getIntStatus();

        // set our DMP Ready flag so the main loop() function knows it's okay to use it
        dmpReady = true;

        // get expected DMP packet size for later comparison
        packetSize = mpu.dmpGetFIFOPacketSize();
    } else {
        // ERROR!
        // 1 = initial memory load failed
        // 2 = DMP configuration updates failed
        // (if it's going to break, usually the code will be 1)
        Serial.print(F("DMP Initialization failed (code "));
        Serial.print(devStatus);
        Serial.println(F(")"));
    }

    // configure LED for output
    pinMode(LED_PIN, OUTPUT);
}



// ================================================================
// ===                    MAIN PROGRAM LOOP                     ===
// ================================================================

void loop() {
    // if programming failed, don't try to do anything
    if (!dmpReady) return;

    // wait for MPU interrupt or extra packet(s) available
    while (!mpuInterrupt && fifoCount < packetSize) {

      char ch;

      if ( haveDistance ) {
        Serial.print(theDistance);
        Serial.print(F(" "));
        Serial.println(tsDistance);
        haveDistance = 0;
      }
    
      if (Serial.available() > 0) {
        digitalWrite(LED_PIN,LOW);
        ch = Serial.read();
        if (String(ch) == "\n") {
          delay(50);
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

    // reset interrupt flag and get INT_STATUS byte
    mpuInterrupt = false;
    mpuIntStatus = mpu.getIntStatus();

    // get current FIFO count
    fifoCount = mpu.getFIFOCount();

    // check for overflow (this should never happen unless our code is too inefficient)
    if ((mpuIntStatus & 0x10) || fifoCount == 1024) {
        // reset so we can continue cleanly
        mpu.resetFIFO();
        Serial.println(F("FIFO overflow!"));

    // otherwise, check for DMP data ready interrupt (this should happen frequently)
    } else if (mpuIntStatus & 0x02) {
        // wait for correct available data length, should be a VERY short wait
        while (fifoCount < packetSize) fifoCount = mpu.getFIFOCount();

        // read a packet from FIFO
        mpu.getFIFOBytes(fifoBuffer, packetSize);
        
        // track FIFO count here in case there is > 1 packet available
        // (this lets us immediately read more without waiting for an interrupt)
        fifoCount -= packetSize;

        // display yaw
        mpu.dmpGetQuaternion(&q, fifoBuffer);
        mpu.dmpGetGravity(&gravity, &q);
        mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
        ypr[0] *= 180/M_PI;
        if ( ypr[0] < 0 ) ypr[0]+=360.0;
        if ( settled ) {
          float delta = fabs(lastAngle-ypr[0]);
          if ( delta > 0.01 ) {
            // Funny things happen around 0 and 360
            // If we get a large positive or negative delta then fix it
            delta = lastAngle-ypr[0];
            if ( delta > 180.0 ) {
              delta = 360.0-delta;
            }
            if ( delta < -180.0 ) {
              delta += 360.0;
            }
            yaw += delta;
          }
          lastAngle = ypr[0];
        } else {
          if ( fabs(lastAngle-ypr[0]) <= 0.01 ) {
            count++;
            if (count == 3) {
              settled = true;
              Serial.println("READY");
            }
          } else {
            count = 0;
          }
          lastAngle = ypr[0];
        }

        // blink LED to indicate activity
        blinkState = !blinkState;
        digitalWrite(LED_PIN, blinkState);
    }
}
