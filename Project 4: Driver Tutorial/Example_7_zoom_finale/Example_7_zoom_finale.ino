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
  PORTD = (PORTD & 0b11) | ((states & 0b111111)<<2);
  states >>= 6;
  PORTB = states & 0b111111;
  states >>= 6;
  PORTC = states & 0b111111;
}

// Set the DDR (INPUT/OUTPUT) status of all LED control lines
inline void DDR_LED(uint32_t states) {
  DDRD = (DDRD & 0b11) | ((states & 0b111111)<<2);
  states >>= 6;
  DDRB = states & 0b111111;
  states >>= 6;
  DDRC = states & 0b111111;
}

uint32_t buffer1[NPINS];
uint32_t buffer2[NPINS];
uint32_t *display_buffer;
uint32_t *drawing_buffer;

// FLip display and drawing buffers
void flipBuffers() {
  uint32_t *temp = display_buffer;
  display_buffer = drawing_buffer;
  drawing_buffer = temp;
}

// Write a 1-bit pixel to display memory
void setPixel(uint32_t *buff, int i, int j, int v) {
  // (add code to convert pixel to display coordinates here)
  if (v) buff[pinmap[i]] |=  ((uint32_t)1)<<pinmap[j];
  else   buff[pinmap[i]] &=~(((uint32_t)1)<<pinmap[j]);
}

// Read a 1-bit pixel from display memory
int getPixel(uint32_t *buff, int i, int j) {
  // (add code to convert pixel to display coordinates here)
  return (buff[pinmap[i]]>>pinmap[j])&1;
}

// Interrupt handler for scanning the display
// I split my control lines into two groups
// One for high and one for low voltage LEDs
// (These lines got a bit scrabled in the electrical
// hookup, so I made this lookup table "mask" 
// to figure out which lines control which lights)
//  1 0 1  0  1  0 1 0 1 0  1  0  1  0 1 0  1 0
//  8,4,16,13,10,2,3,6,5,14,15,17,12,7,9,11,1,0
uint32_t color_mask = 0b011001011100101010;

// Scan high/low voltage LEDs during separate steps
volatile int colorstep = 0;
volatile int scan_line = 0;
void scan_display() {
  // Scan one row of the display
  uint32_t anode = ((uint32_t)1)<<scan_line;
  uint32_t mask = color_mask;
  if (colorstep) mask = ~mask;
  mask &= display_buffer[scan_line];
  // I added this line to avoid a dead pixel
  if (scan_line==17) mask &= ~(1<<9); 
  mask |= anode;
  DDR_LED (0    );
  PORT_LED(anode);
  DDR_LED (mask ); 
  if (++colorstep>1) {
    colorstep = 0;   
    if (++scan_line>=NPINS) scan_line=0;
  }
}
SIGNAL(TIMER2_OVF_vect) {
  TCNT2 = 222;
  scan_display();
}

void setup_diplay() {
  // Start with 1=display, 2=drawing
  display_buffer = &buffer1[0];
  drawing_buffer = &buffer2[0];
  // Clear the display memory
  for (int line=0; line<NPINS; line++)
    buffer1[line]=buffer2[line]=0;
  // Set up Timer2 interrupts
  TCCR2A = 0; // Timer 2 in default mode
  TCCR2A = 3; // Prescale 3: 32 cycles/tick
  TIMSK2 = 1; // Use Timer 2 overflow interrupt
}
 
void setup() {
  setup_diplay();
}

void loop() {
  // The main loop is now free for implementing program logic
  // For example we can randomly flip some LEDs

  // scroll patter 1 pixel
  for (int i=0; i<NPINS; i++) 
    for (int j=0; j<NPINS; j++) {
      if (i==j) continue;
      int next_j = (j+1)%NPINS;
      if (next_j==i) next_j++;
      int p = getPixel(display_buffer,i,next_j);
      setPixel(drawing_buffer,i,j,p);
    }

  // Modify one row randomly
  for (int i=0; i<NPINS; i++) {
    int j = (i+1)%NPINS;
    int p = getPixel(display_buffer,i,j);
    if (p) {
      if (!random(3)) p=0;
    } else {
      if (!random(15)) p=1;
    }
    setPixel(drawing_buffer,i,j,p);
  }
  flipBuffers();
  delay(1000/24);
}
