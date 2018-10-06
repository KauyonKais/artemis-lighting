
#include <DMXSerial.h>


const byte WhitePin =    3;
const byte RedPin =      5;
byte LightPin;

const byte smallMod = 1;
const byte bigMod = 2;
const byte stepTime = 40;

byte lightLevel;
byte lowLevel;
byte dmxBuffer[DMXSERIAL_MAX + 1];
unsigned long systime = 0;


void setup () {
  DMXSerial.init(DMXReceiver);
  randomSeed(analogRead(0));

  // enable pwm outputs
  pinMode(WhitePin,   OUTPUT); // sets the digital pin as output
  pinMode(RedPin, OUTPUT);
  LightPin = WhitePin;
}


void loop() {
  systime = millis();
  //lightLevel = DMXSerial.read(1);
  lightLevel = 255;
  lowLevel = lightLevel / 3;
  if (DMXSerial.read(2) != 1) {
    inNebula();//TODO
  }
  byte loc = DMXSerial.read(4);
  byte hit = DMXSerial.read(3);
  if (loc != 0) {
    gotHit(hit, loc);
  }
  byte dmg = DMXSerial.read(6);
  if (dmg >= 1 && LightPin == WhitePin) { //TODO Test this code!
    analogWrite(LightPin, 0);
    LightPin = RedPin;
  } else if (dmg <= 1 && LightPin == RedPin) {
    analogWrite(LightPin, 0);
    LightPin = WhitePin;
  }
  analogWrite(LightPin, lightLevel);
}


void testHit(byte a, byte b) {
  if (b == 1) {
    locHit(0, 0); //halbe aus
  }
  else {
    onlyHit(0, 0); //blinken
  }
}

void locHit(byte a, byte b) {
  analogWrite(LightPin, 0);
  delay(1500);
}

void onlyHit(byte a, byte b) {
  analogWrite(LightPin, 0);
  delay(250);
  analogWrite(LightPin, 255);
  delay(250);
  analogWrite(LightPin, 0);
  delay(250);
}

void inNebula() {

}

void gotHit(byte weapon, byte location) {
  if (location == 2) {
    switch (weapon) {
      case 1:
        big(1, true);
        break;
      case 2:
        big(1.5, true);
        break;
      case 3:
        big(2, true);
        break;
      default:
        break;
    }
  } else {
    switch (weapon) {
      case 1:
        small(lowLevel);
        break;
      case 2:
        small(lowLevel / 1.5);
        break;
      case 3:
        small(lowLevel / 2);
        break;
      default:
        break;
    }
  }
}

void small(byte low) {
  byte rand = random(4);
  switch (rand) {
    case 0:
      patOne(low, smallMod);
      break;
    case 1:
      patTwo(low, smallMod);
      break;
    case 2:
      patThree(low, smallMod);
      break;
    case 3:
      patFour(low, smallMod);
      break;
    default:
      break;
  }
}

void big(byte mod, boolean again) {
  byte rand = random(4);
  switch (rand) {
    case 0:
      patOne(10, bigMod * mod);
      break;
    case 1:
      patTwo(10, bigMod * mod);
      break;
    case 2:
      patThree(10, bigMod * mod);
      break;
    case 3:
      patFour(10, bigMod * mod);
      break;
    default:
      break;
  }
  if (again) {
    big(mod, false);
  }
}

void patOne(byte low, byte intens) {
  analogWrite(LightPin, low);
  delay(stepTime * intens);
  analogWrite(LightPin, lightLevel);
  delay(2 * stepTime * intens);
  analogWrite(LightPin, low);
  delay(2 * stepTime * intens);
  analogWrite(LightPin, lightLevel);
  delay(stepTime * intens);
  analogWrite(LightPin, low);
  delay(2 * stepTime * intens);
}

void patTwo(byte low, byte intens) {
  analogWrite(LightPin, low);
  delay(4 * stepTime * intens);
  analogWrite(LightPin, lightLevel);
  delay(stepTime * intens);
  analogWrite(LightPin, low);
  delay(stepTime * intens);
  analogWrite(LightPin, lightLevel);
  delay(stepTime * intens);
  analogWrite(LightPin, low);
  delay(stepTime * intens);
}

void patThree(byte low, byte intens) {
  analogWrite(LightPin, low);
  delay(3 * stepTime * intens);
  analogWrite(LightPin, lightLevel);
  delay(stepTime * intens);
  analogWrite(LightPin, low);
  delay(stepTime * intens);
  analogWrite(LightPin, lightLevel);
  delay(stepTime * intens);
  analogWrite(LightPin, low);
  delay(stepTime * intens);
}

void patFour(byte low, byte intens) {
  analogWrite(LightPin, low);
  delay(stepTime * intens);
  analogWrite(LightPin, lightLevel);
  delay(stepTime * intens);
  analogWrite(LightPin, low);
  delay(stepTime * intens);
  analogWrite(LightPin, lightLevel);
  delay(stepTime * intens);
  analogWrite(LightPin, low);
  delay(stepTime * intens);
  analogWrite(LightPin, lightLevel);
  delay(stepTime * intens);
  analogWrite(LightPin, low);
  delay(2 * stepTime * intens);
}
