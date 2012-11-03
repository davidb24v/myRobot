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
#define INIT_PWM 150
byte PWMlr=INIT_PWM, PWMlf=INIT_PWM, PWMrr = INIT_PWM, PWMrf=INIT_PWM;

// Current values for each motor
byte LEFT = 0;
byte RIGHT = 0;

// times to stop motors at (0 otherwise)
long StopMotors = 0;
long StopLeftMotor = 0;
long StopRightMotor = 0;

void setup_motors() {
  // Configure the 4 x PWM pins
  pinMode(PWM_lr, OUTPUT);
  pinMode(PWM_lf, OUTPUT);
  pinMode(PWM_rr, OUTPUT);
  pinMode(PWM_rf, OUTPUT);
  analogWrite(PWM_lr, PWMlr);
  analogWrite(PWM_lf, PWMlf);
  analogWrite(PWM_rr, PWMrr);
  analogWrite(PWM_rf, PWMrr);
}

void writeLeft() {
  Wire.beginTransmission(0x20);
  Wire.write(0x12);
  Wire.write(LEFT);
  Wire.endTransmission();
}

void writeRight() {
  Wire.beginTransmission(0x20);
  Wire.write(0x13);
  Wire.write(RIGHT);
  Wire.endTransmission();
}  

void writeBoth() {
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

void beq() {
   int newval = (PWMlr + PWMlf + PWMrr + PWMrf)/4;
   PWMlr = PWMlf = PWMrr = PWMrf = newval;
}

void lset() {
  byte b;
  b = Serial.read();
  PWMlr = PWMlf = (int) b;
}
void rset() {
  byte b;
  b = Serial.read();
  PWMrr = PWMrf = (int) b;
}
void bset() {
  byte b;
  b = Serial.read();
  PWMrr = PWMrf = PWMlr = PWMlf = (int) b;
}

