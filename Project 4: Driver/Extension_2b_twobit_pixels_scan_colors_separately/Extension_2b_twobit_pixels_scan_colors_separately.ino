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

// No. bits of brightness info per LED
// 2 is the only supported bit-depth
#define NBITS 2
#define SCANLEVELS ((1<<(NBITS))-1)

uint32_t buffer1[NPINS*NBITS];
uint32_t buffer2[NPINS*NBITS];
volatile uint32_t *display_buffer;
volatile uint32_t *drawing_buffer;

// FLip display and drawing buffers
void flipBuffers() {
  uint32_t *temp = display_buffer;
  display_buffer = drawing_buffer;
  drawing_buffer = temp;
}

// Write a N-bit pixel to display memory
void setPixel(uint32_t *buff, int i, int j, int value) {
  for (int bi=0; bi<NBITS; bi++) {
    int b = (value>>bi)&1;
    int c = bi*NPINS;
    if (b) buff[c+pinmap[i]] |=  ((uint32_t)1)<<pinmap[j];
    else   buff[c+pinmap[i]] &=~(((uint32_t)1)<<pinmap[j]);
  }
}

// Read a N-bit pixel from display memory
int getPixel(uint32_t *buff, int i, int j) {
  int value = 0;
  for (int bi=0; bi<NBITS; bi++) {
    int b = (value>>bi)&1;
    int c = bi*NPINS;
    value |= ((buff[c+pinmap[i]]>>pinmap[j])&1)<<bi;
  }
  return value;
}

// Interrupt handler for scanning the display
uint32_t      color_mask = 0b011001011100101010;
volatile  int colorstep  = 0;
volatile  int scanlevel  = 0;
volatile  int scanline   = 0;
const uint8_t scanlens[(1<<NBITS)-1] = {15,15,30};
SIGNAL(TIMER2_OVF_vect) {
  TCNT2 = 256-scanlens[scanlevel];
  uint32_t b1 = display_buffer[scanline];
  uint32_t b2 = display_buffer[NPINS+scanline];
  uint32_t px = 0;
  if (scanlevel==0) px = b1 | b2;
  if (scanlevel==1) px = b2;
  if (scanlevel==2) px = b1 & b2;
  px &= (colorstep? ~color_mask: color_mask);
  uint32_t anode = ((uint32_t)1)<<scanline;
  DDR_LED(0);
  PORT_LED(anode);
  DDR_LED (anode|px);
  if (++colorstep>1) {
    colorstep=0;   
    if (++scanline>=NPINS) {
      scanline=0;
      if (++scanlevel>=SCANLEVELS) 
        scanlevel=0;
    }
  }
}

void setup_diplay() {
  // Start with 1->display, 2->drawing.
  display_buffer = &buffer1[0];
  drawing_buffer = &buffer2[0];
  // Clear memory
  for (int i=0; i<NPINS*NBITS; i++)
    buffer1[i]=buffer2[i]=0;
  // Set up Timer2 interrupts
  TCCR2A = 0; // Normal timer mode
  TCCR2A = 3; // Prescale 3: 32 cycles/tick
  TIMSK2|= 1; // Use Timer 2 overflow interrupt
}
 
void setup() {
  setup_diplay();
  // Draw a test pattern
  int levels[9] = {0,0,1,1,1,2,2,3,3};
  for (int i=0; i<NPINS; i++) 
  for (int j=0; j<NPINS; j++)
    if (i!=j) setPixel(drawing_buffer,i,j,levels[j%9]);
  flipBuffers();
}

void loop() {
  
  // The main loop is now free for implementing program logic
  for (int i=0; i<NPINS; i++) 
  for (int j=0; j<NPINS; j++) {
    if (i==j) continue;
    int next_j = (j+1)%NPINS;
    if (next_j==i) next_j = (next_j+1)%NPINS;
    int p = getPixel(display_buffer,i,next_j);
    setPixel(drawing_buffer,i,j,p);
  }
  flipBuffers();
  delay(1000/20);
}
