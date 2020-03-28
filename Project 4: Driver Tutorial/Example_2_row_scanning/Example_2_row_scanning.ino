/**
 *  - Set up the electronic circuit, with appropriate series resistors 
 *    to protect the LEDs *and* *microcontroller* if something goes wrong.
 *  - For common-anode row/column scanning:
 *    - Set all pins to INPUT mode to avoid spuriously lighting LEDs
 *    - Set the anode pin to HIGH and all others to LOW
 *    - Switch to OUTPUT mode the anode, and any cathods from this row 
 *      that you wish to light up.
 */

#define NPINS 18
const int pinmap[NPINS] = {
  10,6,18,15,12,4,5,8,7,16,17,19,14,9,11,13,3,2
};

void setup() {
  // put your setup code here, to run once:
}

void loop() {
  // put your main code here, to run repeatedly:
  for (int i=0; i<NPINS; i++) 
  {
    int anode = pinmap[i];

    // Turn eveything off
    for (int line=0; line<NPINS; line++)
      pinMode(pinmap[line],INPUT);

    // Set cathodes to LOW and anode to HIGH
    for (int line=0; line<NPINS; line++)
      digitalWrite(pinmap[line],LOW );
    digitalWrite(anode,HIGH);

    // Turn on Anode, and any cathodes we want to light-up
    pinMode(anode,OUTPUT);
    for (int line=0; line<NPINS; line++)
      pinMode(pinmap[line],OUTPUT);

    // Uncomment the next line to scan too quickly for
    // the human eye to see!
    delay(900);
    delayMicroseconds(200);
  }
}
