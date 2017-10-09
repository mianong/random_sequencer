//www.elegoo.com
//2016.12.12


/*******Interrupt-based Rotary Encoder Sketch*******
by Simon Merrett, based on insight from Oleg Mazurov, Nick Gammon, rt, Steve Spence
*/

#include "Rotary.h"

// Rotary encoder is wired with the common to ground and the two
// outputs to pins 2 and 3.
Rotary rotary = Rotary(2, 3);
// Counter that will be incremented or decremented by rotation.
int rotaryCounter = 0;

int counterDirection;
// rotate is called anytime the rotary inputs change state.
void rotate() {
  unsigned char result = rotary.process();
  if (result == DIR_CW) {
    rotaryCounter++;
    counterDirection=1;
  //  Serial.println(rotaryCounter);
  } else if (result == DIR_CCW) {
    rotaryCounter--;
    counterDirection=-1;
  //  Serial.println(rotaryCounter);
  }
}

void setupEncoder() {
  attachInterrupt(0, rotate, CHANGE);
  attachInterrupt(1, rotate, CHANGE);
  
}


