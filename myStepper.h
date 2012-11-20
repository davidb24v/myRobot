// based on code from http://arduino.cc/forum/index.php/topic,85335.0.html
// by celem
//
// This Arduino example demonstrates bidirectional operation of a 
// 28BYJ-48, which is readily available on eBay, using a ULN2003 
// interface board to drive the stepper. The 28BYJ-48 motor is a 4-
// phase, 8-beat motor, geared down by a factor of 68. One bipolar 
// winding is on motor pins 1 & 3 and the other on motor pins 2 & 4. 
// Refer to the manufacturer's documentation of  Changzhou Fulling 
// Motor Co., Ltd., among others.  The step angle is 5.625/64 and the 
// operating Frequency is 100pps. Current draw is 92mA.
// speed when the potentiometer has been rotated fully clockwise.
//
// Modified use an MCP23017 to drive the stepper board
////////////////////////////////////////////////


//set pins to ULN2003 high in sequence from 1 to 4
//delay "stepperSpeed" between each pin setting (to determine speed)

void stepperWrite() {
  Wire.beginTransmission(0x21);
  Wire.write(0x12);
  Wire.write(stepperValueA);
  Wire.endTransmission();
}

void stepperOff() {
  // switch off
  stepperValueA &= 0x0F;
  stepperWrite();
}

void checkStop() {
  // Set address pointer to bank B
  Wire.beginTransmission(0x21);
  Wire.write(0x13);
  Wire.endTransmission();
  
  Wire.requestFrom(0x21, 1);
  stopPin = Wire.read() && 1;
}

void step(int b) {
  stepperValueA &= 0x0F;
  stepperValueA |= b;
  stepperWrite();
  delayMicroseconds(stepperSpeed);
}

void ccw (){
 int n;
 for( n=0; n < nSteps; n++) {
    step(0b00010000);
    step(0b00110000);
    step(0b00100000);
    step(0b01100000);
    step(0b01000000);
    step(0b11000000);
    step(0b10000000);
    step(0b10010000);
  }
  stepperOff();
}


void cw(){
 int n;
 for( n=0; n < nSteps; n++) {
   step(0b10000000);
   step(0b11000000);
   step(0b01000000);
   step(0b01100000);
   step(0b00100000);
   step(0b00110000);
   step(0b00010000);
   step(0b10010000);
  }
  stepperOff();
}

void doStep() {
  int p0, s0;

  // save number of steps
  s0 = nSteps;
  
  if (stepperDir > 0) {
    p0 = pos + nSteps;
    if ( p0 >= cwPos ) stepperDir = -1;
    if ( p0 > cwPos )  nSteps = cwPos-pos;
    cw();
    pos += nSteps;
  } else {
    p0 = pos - nSteps;
    if ( p0 <= ccwPos ) stepperDir = 1;
    if ( p0 < ccwPos ) nSteps = pos-ccwPos;
    ccw();
    pos -= nSteps;
  }
  
  // restore steps
  nSteps = s0;
}

void calibrateStepper() {
  pos = 0;
  nSteps=1;
  // step clockwise until stopPin goes low
  for(;;) {
    pos++;
    cw();
    checkStop();
    if ( stopPin == 0 ) break;
  }
  cwPos = pos;
  Serial.print("CW stop at pos = ");
  Serial.println(pos, DEC);
  
  for(;;) {
    pos--;
    ccw();
    checkStop();
    if ( stopPin == 0 ) break;
  }
  ccwPos = pos;
  Serial.print("CCW stop at pos = ");
  Serial.println(pos, DEC);
  
  // 
  
  // step to centre
  centrePos = (cwPos-ccwPos)/2;
  if ( centrePos % 2 == 1 ) centrePos--;
  
  // force centre position to be zero
  cwPos = centrePos;
  ccwPos = -centrePos;
  Serial.print("Width = ");
  Serial.println(centrePos, DEC);
  centrePos = 0;
  
  for(int i=ccwPos; i <= 0; i++) {
    cw();
  }
  pos=0;
  Serial.print("Centred at pos = ");
  Serial.println(pos, DEC);
}
