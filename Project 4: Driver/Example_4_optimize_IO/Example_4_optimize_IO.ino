/**
 *  This sketch extends Example 2, row-column scanning,
 *  to read data from a display buffer. 
 */

#define NPINS 18
const int pinmap[NPINS] = {
  8,4,16,13,10,2,3,6,5,14,15,17,12,7,9,11,1,0
};

// Set the PORT (HIGH/LOW) status of all LED control lines
inline void PORT_LED(uint32_t states) {
  PORTD = (PORTD & 0b00000011) | ((states & 0b111111)<<2);
  states >>= 6;
  PORTB = states & 0b111111;
  states >>= 6;
  PORTC = states & 0b111111;
}

// Set the DDR (INPUT/OUTPUT) status of all LED control lines
inline void DDR_LED(uint32_t states) {
  DDRD = (DDRD & 0b00000011) | ((states & 0b111111)<<2);
  states >>= 6;
  DDRB = states & 0b111111;
  states >>= 6;
  DDRC = states & 0b111111;
}

uint32_t display_buffer[NPINS]; 

// Write a 1-bit pixel to display memory
void setPixel(int i, int j, int value) {
  // (add code to convert pixel to display coordinates here)
  if (value)
    display_buffer[pinmap[i]] |=  ((uint32_t)1)<<pinmap[j];
  else
    display_buffer[pinmap[i]] &=~(((uint32_t)1)<<pinmap[j]);
}

// Read a 1-bit pixel from display memory
int getPixel(int i, int j) {
  // (add code to convert pixel to display coordinates here)
  return (display_buffer[pinmap[i]]>>pinmap[j])&1;
}

void setup() {
  // Initialize the display memory
  for (int line=0; line<NPINS; line++) 
    display_buffer[line]=0;

  // Draw a test pattern
  for (int i=0; i<NPINS; i++) 
    for (int j=0; j<NPINS; j++)
      if (i!=j && ((i>>2)&1)==((j>>2)&1))
        setPixel(i,j,1);
}

void loop() {
  // scan based on the information in the buffer
  for (int i=0; i<NPINS; i++) 
  {
    // Turn eveything off
    DDR_LED(0);
    
    // Set cathodes to LOW and anode to HIGH
    uint32_t anode_mask = ((uint32_t)1)<<i;
    PORT_LED(anode_mask);

    // Turn on those LEDs which are on
    DDR_LED(anode_mask | display_buffer[i]);    
    
    delayMicroseconds(200);
  }
}
