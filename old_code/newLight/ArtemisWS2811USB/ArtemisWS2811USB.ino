#include <Adafruit_NeoPixel.h>
#include <QueueArray.h>

//Pins
#define RIGHTLIGHT       6
#define LEFTLIGHT        7
//Strip length (one side)
#define SIDE            50
//Tick in ms
#define TICK            10
#define WARPTICK        5
#define BLINKTICK       40
#define NEBSTEP         5
#define JUMPSTEP        10
#define SHIELDSRAN      200
#define JITTERTICK      1


//define colors
#define BRIGHTNESS      255
#define LOWLIGHT        (BRIGHTNESS/3)
#define DARKLIGHT       10
#define WARPLIGHT       50
#define BASECOLOR       255,  255,  160
#define EMERGENCY       255,  0,    0
#define SNEBCOLOR       70,  255,  120
#define FNEBCOLOR       70,  255,  70
#define SHIELD          200


Adafruit_NeoPixel rightStrip = Adafruit_NeoPixel(SIDE, RIGHTLIGHT, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel leftStrip = Adafruit_NeoPixel(SIDE, LEFTLIGHT, NEO_GRB + NEO_KHZ800);

unsigned long waittime;
byte input; //TODO
//Pixel arrays
uint32_t right[SIDE];
uint32_t left[SIDE];
//current main color
uint32_t basecolor;
byte brightness;

//DMX states
byte warpSpeed;
byte weapon;
byte jexec=0;
boolean shield;
boolean nebula;
boolean emergency;
boolean jumpinit;
boolean jumpexec;

char fastNebula = 0;
char slowNebula = 0;
char jump = 0;
boolean fNeb = true;
boolean sNeb = true;
boolean tJump = true;

QueueArray<uint8_t> blinkbuffer;
QueueArray<uint8_t> flashbuffer;
QueueArray<boolean> shieldbuffer;

const byte pn = 4; //number of patterns
const byte ps = 8; //length of patterns
boolean patterns[pn][ps] = {{false, true, true, false, false, true, false, false},
  {false, false, false, false, true, false, true, false},
  {false, false, false, true, false, true, false, false},
  {false, true, false, true, false, true, false, false}
};


void setup() {
  Serial.begin(9600);

  leftStrip.begin();
  rightStrip.begin();

  waittime = millis();
  basecolor = Pixel(BRIGHTNESS, BASECOLOR);
  //initPixels sets value AND color
  initPixels(basecolor);
}


void loop() {
  //read Info (Serial atm, DMX later)
  if (Serial.available()) {
    input = Serial.read();
  }
  //get input variables
  if (input == 's') {
    shield = !shield;
  }
  if (input == 'n') {
    nebula = !nebula;
  }
  if (input == 'a') {
    emergency = !emergency;
  }
  if (input == '0' | input == '1' | input == '2' | input == '3' | input == '4') {
    warpSpeed = input - 48;
  }
  if (input == 'j') {
    //newBlink(input==1);
    newBlink(true);
    weapon = 4;
  }
  if (input == 'k') {
    newBlink(true);
    weapon = 5;

  }
  if (input == 'l') {
    newBlink(true);
    weapon = 7;
  }
  if (input == 'u') {
    newBlink(false);
    weapon = 4;
  }
  if (input == 'i') {
    newBlink(false);
    weapon = 5;
  }
  if (input == 'o') {
    newBlink(false);
    weapon = 7;
  }
  if (input == 'y') {
    jumpinit = !jumpinit;
  }
  if(input=='x'){
    jumpexec=true;
  }


  //every TICK(10)ms
  if ((millis() - waittime) >= TICK) {
    //Serial.println(millis() - waittime);
    //functions working off color
    basecolor = Pixel(255, BASECOLOR);
    if (emergency) {
      basecolor = Pixel(255, EMERGENCY);
    } else if (nebula) {
      basecolor = Pixel(255, BASECOLOR);
      inNebula();
    }
    if (shield && !emergency && !nebula) {
      shields();
    }
    setStripColor(basecolor);
    //functions working off brightness


    blink();
    if(jumpexec){
      jumpExecuted();
    }
    if (jumpinit) {
      jumpInitiated();
    }
    warp();
    flash();
    show(); //send Pixels to Strips
    waittime = millis();
  }
  delay(5);//TODO needed for testing with Serial
}
//END OF LOOP

void show() {
  byte i;
  for (i = 0; i < SIDE; i++) {
    rightStrip.setPixelColor(i, calcPixel(brightness, right[i]));
    leftStrip.setPixelColor(i, calcPixel(brightness, left[i]));
  }
  rightStrip.show();
  leftStrip.show();
}

void initPixels(uint32_t c) {
  byte i;
  for (i = 0; i < SIDE; i++) {
    right[i] = c;
    left[i] = c;
  }
}

void setStripColor(uint32_t c) {
  byte i;
  for (i = 0; i < SIDE; i++) {
    right[i] = Pixel(pixelFlash(right[i]), c);
    left[i] = Pixel(pixelFlash(left[i]), c);
  }
}


//creates new warpflashes depending on warp speed every WARPTICK ticks
byte warptick;
void warp() {
  if (warptick > (WARPTICK * TICK) - (warpSpeed * TICK) && warpSpeed > 0) {
    newWarpFlash(random(warpSpeed) + 1);
    warptick = 0;
  } else {
    warptick++;
  }
}

//creates new flash
void newWarpFlash(byte intens) { //TODO create pattern
  byte i;
  for (i = 0; i < intens; i++) {
    flashbuffer.push(WARPLIGHT);
  }
}


void flash() {
  uint32_t tempFlash;
  byte i;
  //moves Flashes in Strip
  for (i = SIDE - 1; i > 0; i--) {
    if (pixelFlash(right[i - 1]) != pixelFlash(right[i])) {
      right[i] = right[i - 1];
      left[i] = left[i - 1];
    }
  }
  //loads flashbuffer[0] into Strip OR replaces [0] with basecolor
  if (!flashbuffer.isEmpty()) {
    tempFlash = flashbuffer.pop();
    right[0] = Pixel(tempFlash, right[0]);
    left[0] = Pixel(tempFlash, left[0]);
  } else {
    right[0] = basecolor;
    left[0] = basecolor;
  }
}

//gets Flash value byte of Pixel
uint8_t pixelFlash(uint32_t p) {
  return p >> 24;
}

//creates Pixel with flash value and rgb values
uint32_t Pixel(uint8_t flash, uint8_t r, uint8_t g, uint8_t b) {
  return ((uint32_t)flash << 24) | ((uint32_t)r << 16) | ((uint32_t)b <<  8) | g;
}

//creates Pixel with flash value and rgb values from color c
uint32_t Pixel(uint8_t flash, uint32_t c) {
  return ((uint32_t)flash << 24) | c << 8 >> 8;
}

//creates color with rgb values
uint32_t Color(uint8_t r, uint8_t g, uint8_t b) {
  return ((uint32_t)255 << 24) | ((uint32_t)r << 16) | ((uint32_t)b <<  8) | g;
}

//calculates RGB Pixel from light value, flash value of p and rgb of p
uint32_t calcPixel(byte value, uint32_t p) {
  uint32_t tempValue;
  tempValue = value;
  tempValue *= pixelFlash(p);
  return Color((tempValue * (byte)(p >> 16)) / 65025, (tempValue * (byte)p) / 65025, (tempValue * (byte)(p >> 8)) / 65025);
}

uint32_t mixColors(uint32_t c1, uint32_t c2) {
  uint16_t temp;
  byte c1r, c1g, c1b, c2r, c2g, c2b;
  c1r = c1 >> 16;
  c1g = c1;
  c1b = c1 >> 8;
  c2r = c2 >> 16;
  c2g = c2;
  c2b = c2 >> 8;
  temp = c1r;
  temp = (temp + c2r) / 2;
  c1r = (byte)temp;
  temp = c1g;
  temp = (temp + c2g) / 2;
  c1g = (byte)temp;
  temp = c1b;
  temp = (temp + c2b) / 2;
  c1b = (byte)temp;
  return ((uint32_t)255 << 24) | ((uint32_t)c1r << 16) | ((uint32_t)c1b <<  8) | c1g;
}

byte tick;
void blink() {
  //changes brightness every "weapon" tick
  if (!blinkbuffer.isEmpty()) {
    if (tick >= weapon) {
      brightness = blinkbuffer.pop();
      tick = 0;
    }
    else {
      tick++;
    }
  } else {
    brightness = BRIGHTNESS;
  }
}

//empties blinkbuffer, loads random new pattern depending on
//shields on/off
void newBlink(boolean shields) {
  while (!blinkbuffer.isEmpty()) {
    blinkbuffer.pop();
  }
  byte i, ran;
  if (shields) {
    ran = random(pn);
    for (i = 0; i < ps; i++) {
      if (patterns[ran][i]) {
        blinkbuffer.push(BRIGHTNESS);
      } else {
        blinkbuffer.push(LOWLIGHT);
      }
    }
  } else {
    ran = random(pn);
    byte j;
    for (j = 0; j < 2; j++) {
      for (i = 0; i < ps; i++) {
        if (patterns[ran][i]) {
          blinkbuffer.push(BRIGHTNESS);
        } else {
          blinkbuffer.push(DARKLIGHT);
        }
      }
    }
  }
}

void inNebula() {
  //calculate color of both nebula colors+basecolor
  basecolor = mixColors(basecolor, mixColors(calcPixel(sin((fastNebula - 127) / (float)255) * 255, Color(FNEBCOLOR)), calcPixel(sin((slowNebula - 127) / (float)255) * 255, Color(SNEBCOLOR))));
  //count up/down
  if (fNeb & fastNebula + NEBSTEP < 125) {
    fastNebula += NEBSTEP;
    fNeb = true;
  } else if (fastNebula - NEBSTEP > -125) {
    fastNebula -= NEBSTEP;
    fNeb = false;
  } else {
    fNeb = true;
  }
  //count up/down
  if (sNeb & slowNebula + 1 < 125) {
    slowNebula++;
    sNeb = true;
  } else if (slowNebula - 1 > -125) {
    slowNebula--;
    sNeb = false;
  } else {
    sNeb = true;
  }
}

//adds jitter if shields are up
void shields() {
  //clear buffer step by step
  if (!shieldbuffer.isEmpty()) {
    if (shieldbuffer.pop()) {
      basecolor = basecolor | (byte)SHIELD << 8;
    }
  } else {
    byte i, j;
    //if buffer is empty, randomly load new buffer
    if (random(SHIELDSRAN) == 1) {
      for (i = 0; i < ps; i++) {
        for (j = 0; j < JITTERTICK; j++) {
          shieldbuffer.push(patterns[random(pn)][i]);
        }
      }
    }
  }
}

void jumpInitiated() {
  if (tJump & jump + JUMPSTEP < 120) {
    jump += NEBSTEP;
    tJump = true;
  } else if (jump - JUMPSTEP > -120) {
    jump -= JUMPSTEP;
    tJump = false;
  } else {
    tJump = true;
  }
  flashbuffer.push(sin((jump - 127) / (float)255) * 255);
}

void jumpExecuted(){
  Serial.println(jexec);
  if(jexec < 100){
    brightness=0;
    jexec++;
  }else{
    weapon=2;
    newBlink(false);
    brightness=BRIGHTNESS;
    jexec=0;
    jumpexec=false;
  }
}

