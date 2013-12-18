// Motor functions

void set_motor_speed(char motor, int value) {
  byte chk, msb, lsb;
  msb = highByte(value);
  lsb = lowByte(value);
  chk = msb ^ lsb;
  
  if ( motor == 'L' ) {
    LEFT.write('*');
    LEFT.write(msb);
    LEFT.write(lsb);
    LEFT.write(chk);
  } else if ( motor == 'R' ) {
    RIGHT.write('*');
    RIGHT.write(msb);
    RIGHT.write(lsb);
    RIGHT.write(chk);
  } else {
    // Do both
    LEFT.write('*');
    RIGHT.write('*');
    LEFT.write(msb);
    RIGHT.write(msb);
    LEFT.write(lsb);
    RIGHT.write(lsb);
    LEFT.write(chk);
    RIGHT.write(chk);
  }
}  
