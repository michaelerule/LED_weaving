/**
 *  This sketch extends Example 2, row-column scanning,
 *  to read data from a display buffer. 
 */

#define NPINS 18
const int pinmap[NPINS] = {
  10,6,18,15,12,4,5,8,7,16,17,19,14,9,11,13,3,2
};

uint32_t display_buffer[NPINS]; 

void setup() {
  // Initialize the display memory
  for (int line=0; line<NPINS; line++) 
    display_buffer[line]=0;

  // Draw a test pattern
  for (int i=0; i<NPINS; i++) 
    for (int j=0; j<NPINS; j++)
      if (i!=j && ((i>>2)&1)==((j>>2)&1))
        display_buffer[i] |= ((uint32_t)1)<<j;
}

void loop() {
  // scan based on the information in the buffer
  for (int i=0; i<NPINS; i++) 
  {
    // Turn eveything off
    for (int line=0; line<NPINS; line++)
      pinMode(pinmap[line],INPUT);

    // Set cathodes to LOW and anode to HIGH
    int anode = pinmap[i];
    for (int line=0; line<NPINS; line++)
      digitalWrite(pinmap[line],LOW );
    digitalWrite(anode,HIGH);

    // Turn on Anode, and any cathodes we want to light-up
    pinMode(anode,OUTPUT);
    for (int line=0; line<NPINS; line++)
      if (display_buffer[i]>>line&1)
        pinMode(pinmap[line],OUTPUT);
    
    delayMicroseconds(200);
  }
}
