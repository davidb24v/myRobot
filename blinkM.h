// blinkM clone
const byte blinkM = 0x06;

#include <Wire.h>

/*
const byte blinkM_set_rgb = 0x6e;
const byte blinkM_fade_to_rgb = 0x63;
const byte blinkM_play_script = 0x70;
const byte blinkM_stop_script = 0x6f;
const byte blinkM_set_fade_speed = 0x66;
const byte blinkM_set_time_adjust = 0x74;
const byte blinkM_write_script_line = 0x57;
const byte blinkM_set_script_length_and_repeats = 0x4c;#
*/

// format of light script lines: duration, command, arg1,arg2,arg3
typedef struct _blinkm_script_line {
  uint8_t dur;
  uint8_t cmd[4];    // cmd,arg1,arg2,arg3
} blinkm_script_line;

void blinkM_stop_script() {
  Wire.beginTransmission(blinkM);
  Wire.write('o');
  Wire.endTransmission();
}


static void BlinkM_writeScriptLine(byte addr, byte script_id, 
                                   byte pos, byte dur,
                                   byte cmd, byte arg1, byte arg2, byte arg3) {
  Wire.beginTransmission(addr);
  Wire.write('W');
  Wire.write(script_id);
  Wire.write(pos);
  Wire.write(dur);
  Wire.write(cmd);
  Wire.write(arg1);
  Wire.write(arg2);
  Wire.write(arg3);
  Wire.endTransmission();

}

static void BlinkM_setScriptLengthReps(byte addr, byte script_id, 
                                       byte len, byte reps) {
  Wire.beginTransmission(addr);
  Wire.write('L');
  Wire.write(script_id);
  Wire.write(len);
  Wire.write(reps);
  Wire.endTransmission();
}

static void BlinkM_writeScript(byte addr, byte script_id, 
                               byte len, byte reps,
                               blinkm_script_line* lines) {
  blinkM_stop_script();
  for(byte i=0; i < len; i++) {
    blinkm_script_line l = lines[i];
    BlinkM_writeScriptLine( addr, script_id, i, l.dur,
                            l.cmd[0], l.cmd[1], l.cmd[2], l.cmd[3]);
    delay(20); // must wait for EEPROM to be programmed
  }
  BlinkM_setScriptLengthReps(addr, script_id, len, reps);
}



void blinkM_set_rgb_colour(byte r, byte g, byte b) {
  Wire.beginTransmission(blinkM);
  Wire.write('n');
  Wire.write(r);
  Wire.write(g);
  Wire.write(b);
  Wire.endTransmission();
}

void blinkM_fade_to_rgb(byte r, byte g, byte b) {
  Wire.beginTransmission(blinkM);
  Wire.write('c');
  Wire.write(r);
  Wire.write(g);
  Wire.write(b);
  Wire.endTransmission();
}

void blinkM_play_script(byte id, byte num_repeats) {
  Wire.beginTransmission(blinkM);
  Wire.write('p');
  Wire.write(id);
  Wire.write(num_repeats);
  Wire.write((byte) 0);
  Wire.endTransmission();
}

void blinkM_set_fade_speed(byte fadeSpeed) {
  Wire.beginTransmission(blinkM);
  Wire.write('f');
  Wire.write(fadeSpeed);
  Wire.endTransmission();
}

void blinkM_set_time_adjust(byte adjust) {
  Wire.beginTransmission(blinkM);
  Wire.write('t');
  Wire.write(adjust);
  Wire.endTransmission();
}

//  Orange flashing light
blinkm_script_line orange_flash[] = {
  {  1,  {'f',   255,  00,  00}},  // set fade speed
  { 5, {'c', 0xff,0x80,0x00}}, 
  { 75, {'c', 0x00,0x00,0x00}},  // off
};
int orange_flash_len = 3;  // number of script lines above

void blinkM_orange_flash() {
  BlinkM_writeScript( blinkM, 0, orange_flash_len, 0, orange_flash);
  blinkM_play_script(0,0);
}

//  Red/Blue flashing light
blinkm_script_line redblue_flash[] = {
  {  3,  {'f',   255,  00,  00}},  // set fade speed
  { 20, {'c', 0xff,0x00,0x00}}, 
  { 20, {'c', 0x00,0x00,0xff}}, 
};
int redblue_flash_len = 3;  // number of script lines above

void blinkM_redblue_flash() {
  BlinkM_writeScript( blinkM, 0, redblue_flash_len, 0, redblue_flash);
  blinkM_play_script(0,0);
}

//  Red fade light
blinkm_script_line red_flash[] = {
  {  1,  {'f',   5,  00,  00}},  // set fade speed
  { 5, {'c', 0xff,0x00,0x00}}, 
  { 25, {'c', 0x00,0x00,0x00}},  // off
};
int red_flash_len = 3;  // number of script lines above

void blinkM_red_flash() {
  BlinkM_writeScript( blinkM, 0, red_flash_len, 0, red_flash);
  blinkM_play_script(0,0);
}


//  Green light
blinkm_script_line idle_fade[] = {
  {  1,  {'f',   5,  00,  00}},  // set fade speed
  {100, {'c', 0x00,0x80,0x00}}, 
  {100, {'c', 0x00,0x00,0x00}},  // off
};
int idle_fade_len = 3;  // number of script lines above

void blinkM_idle() {
  BlinkM_writeScript( blinkM, 0, idle_fade_len, 0, idle_fade);
  blinkM_play_script(0,0);
}

//  "off"
blinkm_script_line off_fade[] = {
  {  1,  {'f',   1,  00,  00}},  // set fade speed
  { 10, {'c', 0x40,0x40,0x40}}, 
  {100, {'c', 0x00,0x00,0x00}},  // off
};
int off_fade_len = 3;  // number of script lines above

void blinkM_off () {
  BlinkM_writeScript( blinkM, 0, off_fade_len, 0, off_fade);
  blinkM_play_script(0,0);
}
