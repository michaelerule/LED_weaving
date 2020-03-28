/**
 *  - Build Charlieplexed display hardware
 *  - Hook up all lines with appropriate current-limiting resistors
 *  - Set the constant NPINS to the number of lines
 *  - Place the Arduino pin numbers used in the `pinmap` array
 *  - Upload this sketch and confirm that all lights work
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
  int j=0;
  for (int j=0; j<NPINS; j++) 
  for (int i=0; i<NPINS; i++)
  {
    if (i==j) continue;
    int anode   = pinmap[i];
    int cathode = pinmap[j];
    // Turn everything off
    for (int line=0; line<NPINS; line++)
      pinMode(pinmap[line],INPUT);
    // Turn on one anode-cathode pair
    digitalWrite(cathode,LOW );
    digitalWrite(anode  ,HIGH);
    pinMode(anode  ,OUTPUT);
    pinMode(cathode,OUTPUT);
    delayMicroseconds(500);
  }
}
