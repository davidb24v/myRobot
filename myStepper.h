
void step() {
  digitalWrite(STEPPER_STEP_PIN,HIGH);
  delayMicroseconds(stepperSpeed);
  digitalWrite(STEPPER_STEP_PIN,LOW);
  delayMicroseconds(stepperSpeed);
}

void cw () {
  digitalWrite(STEPPER_DIR_PIN,HIGH);
}


void ccw () {
  digitalWrite(STEPPER_DIR_PIN,LOW);
}

void calibrateStepper() {
  
  pinMode(STEPPER_DIR_PIN,OUTPUT);
  pinMode(STEPPER_STEP_PIN,OUTPUT);
  digitalWrite(STEPPER_DIR_PIN,LOW);
  digitalWrite(STEPPER_STEP_PIN,LOW);
  pinMode(stopPin,INPUT);

  pos = 0;
  // step clockwise until stopPin goes low
  cw();
  for(;;) {
    step();
    pos++;
    if ( digitalRead(stopPin) == 0 ) break;
  }
  cwPos = pos;
  
  // note time at which we start from fully clockwise
  long fullCW = millis();

  // back off 10 steps counter-clockwise  
  ccw();
  for(int j=1; j<10; j++) {
    step();
    pos--;
  }

  // continue stepping counter clockwise until stopPin goes low
  for(;;) {
    step();
    pos--;
    if ( digitalRead(stopPin) == 0 ) break;
  }
  ccwPos = pos;
  
  // note time at which we reach fully counter-clockwise
  long fullCCW = millis();

  // step to centre
  centrePos = (cwPos-ccwPos)/2;
  
  // force centre position to be zero
  cwPos = centrePos;
  ccwPos = -centrePos;
  centrePos = 0;

  cw();  
  for(int i=ccwPos; i <= 0; i++) {
    step();
  }
  pos=0;

  Serial.println(cwPos);
  Serial.println(fullCCW-fullCW);
}

void setPos(int to) {
  int diff = to-pos;
  if ( diff == 0 ) return;
  if ( diff > 0 ) {
    cw();
    for (int i=0; i < diff; i++) {
      step();
      pos++;
    }
  } else {
    ccw();
    diff *= -1;
    for (int i=0; i < diff; i++) {
      step();
      pos--;
    }
  }
}

