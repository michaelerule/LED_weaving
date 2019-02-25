/*
    This is designed for the AtMega168/328-based boards and will not work on 
    other Arduino platforms, since it uses port mappings to more efficiently 
    control the LEDs.

    We'll extend the "bulk scan" example to display serial data sent from the 
    computer. 
    
    First, as a test, write a routine to shift the display data down 
    one row. For this, we'll do something simple, and just take the lowest NROWS 
    bits from the serial byte and pipe it to the screen, shifting all other 
    lines down one. 
    
    Scanning charlieplexed LEDs "in parallel": 
    
    We scan ALL lights at once, per row. This causes some issues since rows 
    with many lights on will dim, but it is much faster and brigter overall.
    This is technically out of spec for the arduino, as it can over-current
    the I/O lines if too many LEDs are on. In practice, with the diagonally-
    multiplexed marquee, more than three LEDs per row are seldom on, and this 
    method works fairly well. 
  
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

        Programmer pinout, 10 pin:
                
        3 vcc  +-+   MOSI 2
               + +    
               + +]  RST  4 
               + +   SCK  5 
        1 gnd  +-+   MISO 6
*/

#include <avr/interrupt.h>

#define DELAY  25
#define NLINES 11
#define NCOLS ((NLINES-1)/2)
#define NROWS (((NLINES-1)*NLINES)/NCOLS)

// Map from LED grid line number to arduino pin
// This needs to be a contiguous block of pins
#define PIN_OFFSET 2
const uint8_t lineToPin[NLINES] {10,0,2,4,6,8,7,5,3,1,9};

// Volatile variables for timer interrupt routine
#define disp_t volatile uint16_t
disp_t buffer1[NLINES];
disp_t buffer2[NLINES];
disp_t *display_data = &buffer1[0];
volatile int line1 = 0;
volatile int line2 = 0;

// Trying to light 'dead' LEDs causes other LEDs to light up in a charlieplexing 
// configuration. So, don't set these ones to on! (see setup code)
disp_t bad_LEDs[NLINES];

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

/* Timer interrupt handler: scans the display
 */
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
  // (Don't try to light up dead LEDs though)
  uint32_t pin_states = 1<<line1;
  uint32_t io_mode = pin_states | (display_data[line1] & ~bad_LEDs[line1]);
  set_ddr(io_mode<<PIN_OFFSET);
  set_port(pin_states<<PIN_OFFSET);
}

/* The Charlieplexing scanning grid does not line up 
 * nicely with the display grid, so we convert display
 * locations into charlieplexing locations. 
 * 
 * I wrote this function with trial and error, so it 
 * might not be the  most efficient implementation.
 *  
 */
inline void displayToGrid(int row, int col, int *i, int *j) {

  // Messed up one LED!
  
  if (row==19 && col==0) {
    row=8;
    col=4;
  }
  else if (row==8 && col==4) {
    row=19;
    col=0;
  }
  
  // flip it upside down
  row = NROWS-1-row;
  col = NCOLS-1-col;
  
  col = col*2+1-(row&1);
  row = (row/2-col/2-(col&1)+NLINES)%NLINES;
  col = (row+col-1+NLINES)%(NLINES-1);
  if (col>=row) col++;
  *i = lineToPin[row];
  *j = lineToPin[col];
}

/* Set given pixel to "on"
 */
inline void set_pixel(disp_t *buff, int row, int col) {
  int i,j;
  displayToGrid(row,col,&i,&j);
  buff[i]|=1<<j;
}

/* Set given pixel to "off"
 */
inline void clear_pixel(disp_t *buff, int row, int col) {
  int i,j;
  displayToGrid(row,col,&i,&j);
  buff[i]&=~(1<<j);
}

/* Write pixel to specified value
 */
inline void write_pixel(disp_t *buff, int row, int col, uint8_t px) {
  if (px) set_pixel  (buff,row,col);
  else    clear_pixel(buff,row,col);
}

/* Retrieve pixel state
 */
inline uint8_t get_pixel(disp_t *buff, int row, int col) {
  int i,j;
  displayToGrid(row,col,&i,&j);
  return (buff[i]>>j)&1;
}

/* Shift the display right one pixel
 *  
 */
void shift_right(uint8_t data) {
  disp_t *buff = &display_data[0];
  for (int row=1; row<NROWS; row++)
    for (int col=0; col<NCOLS; col++)
      write_pixel(buff,row-1,col,get_pixel(buff,row,col));
  // last row zeroed out
  for (int col=0; col<NCOLS; col++)
    write_pixel(buff,NROWS-1,col,(data>>col)&1);
}

/* Shift the display left one pixel
 * Implement this one with double buffering now 
 */
void shift_left(uint8_t data) {
  disp_t *buff1 = &display_data[0];
  // whichever buffer isn't being used for the display
  disp_t *buff2 = &buffer1[0]==&display_data[0]? &buffer2[0] : &buffer1[0];
  for (int row=NROWS-1; row>=1; row--)
    for (int col=0; col<NCOLS; col++)
      write_pixel(buff2,row,col,get_pixel(buff1,row-1,col));

  // last row zeroed out
  for (int col=0; col<NCOLS; col++)
    write_pixel(buff2,0,col,(data>>col)&1);

  // Now flip the buffers
  display_data = buff2;
}

void set_timer_2_interrupt() {
  // Arduino environment might be already running interrupts
  // Temporarily disable interrupts as we configure
  cli();
  // Can't use timer 0 since arduino uses it!!
  // Use the TIMER2_OVF_vect
  // Set clock divisor (small 3 bits of TCCR2B)
  // Possible values are (0 through 7): 1 8 32 64 128 256 1024
  TCCR2B=(TCCR2B&0b11111000)|0b010;
  // Set the output compare A bit for timer 2
  TIMSK2 |= 0b010;
  // Set timer period
  OCR2A = 1;
  // Allow interrupts
  sei();
}

/* Arduino setup function: 
 * called once when the sketch starts up
 */
void setup() {
  // Set everything LOW and INPUT mode
  set_ddr(0);
  set_port(0);

  // Zero display buffer
  for (int line=0; line<NLINES; line++)
    display_data[line]=bad_LEDs[line]=0;

  // Some LEDs are bad, disable them them here
  set_pixel(&bad_LEDs[0],19,0);
  set_pixel(&bad_LEDs[0],8,4);

  // Start timer interrupt to scan display
  set_timer_2_interrupt();
}


/** Drawing routine
 *  This function scrolls the display left one pixel, and adds new pixel
 *  data on the right. It waits at least `scan_interval_ms` between calls 
 *  to smooth the scrolling effect (this is a blocking wait).
 */
volatile unsigned long previousMillis   = 0;
const    unsigned long scan_interval_ms = 80;
void draw_column(uint8_t pixel_data) {

  // Wait until next scan interval 
  while (millis()-previousMillis <= scan_interval_ms);
  previousMillis = millis(); 
    
  // Draw pixel data to display
  shift_left(pixel_data);
}


#include "message.h"

void loop() {
  //display_check();
  scroll_message();
}

void display_check() {
  for (int col=0; col<NCOLS; col++) {
    for (int row=0; row<NROWS; row++) {
      set_pixel(&display_data[0],row,col);
      delay(DELAY);
    }
  }
  for (int col=0; col<NCOLS; col++) {
    for (int row=0; row<NROWS; row++) {
      //clear_pixel(&display_data[0],row,col); 
      //delay(DELAY);
    }
  }
}
