/*
I2C Slave Motor Controller - 2 x Pololu Baby Oragutan "B" boards
*/
 
#define F_CPU=20000000L

#include <OrangutanMotors.h>
#include <SoftwareSerial.h>

SoftwareSerial mySerial(7,4);

int baudRate = 19200;

const int led = 1;  // Red LED which we'll use to indicate receipt of a message
long ledOn = 0;

// buffer to hold serial data
// ['*',msb,lsb,crc]
byte data[4];
int nbytes = 0;

// the setup routine runs once when you press reset:
void setup() { 

  // Start with motors off (should be default)
  set_motors(0,0);

  // initialize the digital pin as an output.
  pinMode(led, OUTPUT);
  
  // open serial port
  mySerial.begin(baudRate);
  
  // start listening for commands (we only receive, never transmit)
  mySerial.listen();
  
}

void on() {
  digitalWrite(led,HIGH);
  ledOn = millis()+10;
}

void loop() {
  
  if ( mySerial.available() ) {
    byte b = mySerial.read();
    if ( nbytes == 0 && b != '*' ) {
      on();
      return;
    }
    data[nbytes] = b;
    nbytes++;
    if ( nbytes == 4 ) {
      // have complete message
      byte checkSum = data[1] ^ data[2];
      if ( checkSum == data[3] ) {
        int speed = 256*data[1]+data[2];
        set_motors(speed,speed);
      } else {
        on();
      }
      nbytes = 0;
    }
  }
  
  if ( ledOn && millis() > ledOn ) {
    digitalWrite(led,LOW);
    ledOn = 0;
  }

}


