/*
  In this one, we'll scan ALL lights at once, per row.
  This causes some issues since rows with many lights on
  will be dim, but it is much faster and might be a bit
  brigter overall. 

  We assume that we're using one of the AtMega*8-based
  Arduinos, in which pins 0-7 map to portD. Pins 8-13 
  map to portB (6 pins)

  This project uses 11 control lines, reserving 0 and 1
  for serial I/O, so 13 in total. We don't need to worry 
  about mapping using PORTC, the analog I/O lines.   
    
    DDRx  : 1 = output, 0 = input
    PORTx : output buffer
    PINx  : digital input buffer ( writes set pullups )

    Arduino Uno Pin mapping
                          __ __
         !RESET     PC6 -|  U  |- PC5    19 (a5)
         0 (rx)     PD0 -|     |- PC4    18 (a4)
         1 (tx)     PD1 -|     |- PC3    17 (a3)
         2          PD2 -|     |- PC2    16 (a2)
         3 (pwm)    PD3 -|  m  |- PC1    15 (a1)
         4          PD4 -|  3  |- PC0    14 (a0)
                    VCC -|  2  |- GND
                    GND -|  8  |- AREF
         xtal       PB6 -|     |- AVCC
         xtal       PB7 -|     |- PB5    13 SCK  ( yello )
         5 (pwm)    PD5 -|     |- PB4    12 MISO ( green )
         6 (pwm)    PD6 -|     |- PB3    11 MOSI ( blue )
         7          PD7 -|     |- PB2    10
         8          PB0 -|_____|- PB1     9

        Programmer pinout, 6 pin:
                
        6 MISO +-+  VCC 3
        5 SCK  + + MOSI 2 
        4 RST  +-+  GND 1
        Programmer pinout, 6 pin:
                
        6 MISO +-+  
        5 SCK  + + 
        4 RST  +-+  
        VCC 3
        MOSI 2
        GND 1

        Programmer pinout, 10 pin:
                
        3 vcc  +-+   MOSI 2
               + +    
               + +]  RST  4 
               + +   SCK  5 
        1 gnd  +-+   MISO 6
*/

#include <avr/interrupt.h>

#define NLINES 11
#define SCANRATE 130

// Map from LED grid line number to arduino pin
// This needs to be a contiguous block of pins
#define PIN_OFFSET 2
const uint8_t lineToPin[NLINES] {0,2,3,5,4,1,9,10,8,7,6};

// Volatile variables for timer interrupt routine
volatile uint16_t display_data[NLINES];
volatile int line1 = 0;
volatile int line2 = 0;

// Commands to set all three ports at the same time
// Note that this only makes sense on the 168/368 based boards
// (it will not work on the Leonardo and other 32U boards)
inline void set_ddr(uint32_t b){
  DDRD=b&0xff; b>>=8;
  DDRB=b&0xff; b>>=8;
  DDRC=b&0xff;
}

inline void set_port(uint32_t b){
  PORTD=b&0xff; b>>=8; 
  PORTB=b&0xff; b>>=8; 
  PORTC=b&0xff;
}


SIGNAL(TIMER2_COMPA_vect) 
{
  // Set everything LOW and INPUT mode
  set_ddr(0);
  set_port(0);
  
  // Increment loop counters
  line1 = (line1+1)%NLINES ;
  
  // Show ALL pins
  // The currently active line will be high
  // All active LEDs on this line will be low
  uint32_t pin_states = 1<<line1;
  uint32_t io_mode = pin_states | display_data[line1];
  set_ddr(io_mode<<PIN_OFFSET);
  set_port(pin_states<<PIN_OFFSET);
}

void setup() {
  // Set everything LOW and INPUT mode
  set_ddr(0);
  set_port(0);

  // Zero display buffer
  for (int line=0; line<NLINES; line++)
    display_data[line]=0;

  // Arduino environment might be already running interrupts
  // Temporarily disable interrupts as we configure
  cli();

  // Can't use timer 0 since arduino uses it!!
  // Use the TIMER2_OVF_vect
  // Set clock divisor (small 3 bits of TCCR2B)
  // Possible values are (0 through 7): 1 8 32 64 128 256 1024
  TCCR2B=(TCCR2B&0b11111000)|0b011;

  // Set the output compare A bit for timer 2
  TIMSK2 |= 0b010;

  // Set timer period
  OCR2A = 1;
  
  // Allow interrupts
  sei();
}


// These work as long as NLINES is odd
#define NCOLS ((NLINES-1)/2)
#define NROWS (((NLINES-1)*NLINES)/NCOLS)

// Trying to light dead LEDs causes other LEDs
// to light up in a charlieplexing configuration
// So, don't set these ones!
volatile uint32_t bad_LEDs[NCOLS] {
  0b0000000100000000000000,
  0b0000000000000000000000,
  0b0000000000000000000000,
  0b0000000000000000000000,
  0b0000000000000001000100,
};

/* The Charlieplexing scanning grid does not line up 
 * nicely with the display grid, so we convert display
 * locations into charlieplexing locations. 
 * 
 * I wrote this function with trial and error, so it 
 * might not be the  most efficient implementation.
 *  
 */
inline void displayToGrid(int row, int col, int *i, int *j) {
  col = col*2+1-(row&1);
  row = (row/2-col/2-(col&1)+NLINES)%NLINES;
  col = (row+col-1+NLINES)%(NLINES-1);
  if (col>=row) col++;
  *i = lineToPin[row];
  *j = lineToPin[col];
}

/* Set given pixel to "on"
 *  
 */
void set_pixel(int row, int col) {
  if ((bad_LEDs[col]>>row)&1) return;
  int i,j;
  displayToGrid(row,col,&i,&j);
  display_data[i]|=1<<j;
}

/* Set given pixel to "off"
 */
void clear_pixel(int row, int col) {
  int i,j;
  displayToGrid(row,col,&i,&j);
  display_data[i]&=~(1<<j);
}

#define DELAY 4

void loop() {

  for (int iter=0; iter<3; iter++) {
    for (int row=0; row<NROWS; row++) {
      for (int col=0; col<NCOLS; col++) {
        set_pixel(row,col);
        delay(DELAY);
      }
    }
    for (int col=0; col<NCOLS; col++) {
      for (int row=0; row<NROWS; row++) {
        clear_pixel(row,col); 
        delay(DELAY);
      }
    }
  }

  for (int iter=0; iter<5; iter++) {
    for (int row=0; row<NROWS+2; row++) {
      for (int col=0; col<NCOLS; col++) {
        if (row<NROWS) 
          set_pixel(row,col);
        delay(DELAY);
        clear_pixel((row+NROWS-2)%NROWS,col); 
      }
    }
    delay(DELAY*5);
  }
  for (int row=NROWS-1; row>=0; row--) {
    for (int col=0; col<NCOLS; col++) {
      set_pixel(row,col);
      delay(DELAY);
      clear_pixel((row+1)%NROWS,col); 
    }
  }
  for (int col=0; col<NCOLS; col++) {
    delay(DELAY);
    clear_pixel(0,col); 
  }
  delay(DELAY*5);
  
}