/**
 * This sketch extends Example 3 to scan the display
 * from within a timer interrupt handler. 
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

uint32_t display_buffer[NPINS]; 

// Write a 1-bit pixel to display memory
void setPixel(int i, int j, int v) {
  // (add code to convert pixel to display coordinates here)
  if (v) display_buffer[pinmap[i]] |=  ((uint32_t)1)<<pinmap[j];
  else   display_buffer[pinmap[i]] &=~(((uint32_t)1)<<pinmap[j]);
}

// Read a 1-bit pixel from display memory
int getPixel(int i, int j) {
  // (add code to convert pixel to display coordinates here)
  return (display_buffer[pinmap[i]]>>pinmap[j])&1;
}

// Scan one row of the display
volatile int scan_line = 0;
void scan_display() {
  uint32_t anode = ((uint32_t)1)<<scan_line;
  uint32_t mask  = anode | display_buffer[scan_line];
  DDR_LED (0    );
  PORT_LED(anode);
  DDR_LED (mask );
  if (++scan_line>=NPINS) scan_line=0;
}

// Interrupt handler for scanning the display
SIGNAL(TIMER2_OVF_vect) {
  // We want to update 18 rows at 400 Hz
  // I used the following python code to calculate
  // the reset value, using a prescaler of 32.
  //
  // >>> CLOCKRATE = 16e6 # 16 MHz system clock
  // >>> NLINES    = 18   # 18 LED control lines
  // >>> RATE      = 400  # Hz; Display scan rate
  // >>> PRESCALE  = 32   # Timer prescaler
  // >>> TIMERMAX  = 256  # 256 if 8-bit, 65536 if 16-bit timer  
  // >>> trigger_every = (CLOCKRATE/PRESCALE)/(NLINES*RATE)
  // >>> reset_to      = int(TIMERMAX-trigger_every+0.5)
  // >>> print('Reset the 8-bit timer to %d'%reset_to)
  TCNT2 = 187; // Reset timer; 
  scan_display();
}

void setup_diplay() {
  // Initialize the display memory
  for (int line=0; line<NPINS; line++) 
    display_buffer[line]=0;

  // Set up Timer2 interrupts

  // Timer/counter 2 control register A
  // Set to 0 to use the default mode.
  TCCR2A = 0;

  // Timer/counter 2 control register B
  // The first 3 bits control the prescaler
  // (i.e. clock divisor for timer tics)
  // 0:off  2:8   4:64   6:256
  // 1:1    3:32  5:128  7:1024
  TCCR2A = 3;
  
  // Enable the Timer 2 overflow interrupt
  // Timer/Counter2 Interrupt Mask Register
  TIMSK2 = 1;
}
 
void setup() {
  setup_diplay();
}

void loop() {
  // The main loop is now free for implementing program logic
  // For example we can randomly flip some LEDs
  setPixel(random(NPINS),random(NPINS),random(2));
  delay(5);
}
