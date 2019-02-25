/*  Debugging script for charlieplexing LED configuration.

    Use this script to test turning all lights on/off, and to set or determine 
    the mapping between Arduino pin numbers and control lines for the LED 
    display. 

    This is designed for the AtMega168/328-based boards and will not work on 
    other Arduino platforms, since it uses port mappings to more efficiently 
    control the LEDs.

    This implementation uses a timer interrupt to scane the LEDs one at a time.

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

// Include support for timer interrupts
#include <avr/interrupt.h>

// delay between turning lghts on/off in ms
#define DELAY 500

// We can control NLINES*(NLINES-1) LEDs using NLINES pins. Here, we develop a
// mapping between the LED control lines and the physical layout of the
// scrolling marquee, so we define columns/rows in terms ot the marquee shape. 
// NLINES should be odd.
#define NLINES 11
#define NROWS ((NLINES-1)/2)
#define NCOLS (((NLINES-1)*NLINES)/NROWS)

// Map from LED grid line number to arduino pin
const uint8_t lineToPin[NLINES] {11,12,2,4,6,8,10,9,7,5,3,};

// Volatile state variables for timer interrupt routine
volatile uint16_t display_data[NLINES];
volatile int      line1 = 0;
volatile int      line2 = 0;

/** Timer ineterrupt to scan the display
 */
SIGNAL(TIMER2_COMPA_vect) 
{
  // everything to INPUT mode
  DDRB  = DDRC  = DDRD  = 0;

  // Increment loop counters
  do if (++line2>=NLINES) {
      line2=0;
      line1=(line1+1)%NLINES;
  } while (line1==line2);

  // Show ONE pin
  if ((display_data[line1]>>line2)&1) {
    int p1 = lineToPin[line1];
    int p2 = lineToPin[line2];
    digitalWrite(p1,1);
    digitalWrite(p2,0);
    pinMode(p1,1);
    pinMode(p2,1);
  }
}

void setup() {
  // Set everything LOW and INPUT mode
  DDRB  = DDRC  = DDRD  = 0;
  PORTB = PORTC = PORTD = 0;
  
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
  TCCR2B=(TCCR2B&0b11111000)|0b001;
  // Set the output compare A bit for timer 2
  TIMSK2 |= 0b010;
  // Set timer period
  OCR2A = 1;
  // Allow interrupts
  sei();
}

/* Convert between display coordinates and LED grid 
 * coordinates.
 */
inline void displayToGrid(int row, int col, int *i, int *j) {
  // rows is now physical rows (display height)
  // cols is now physics columns (display width/length)

  //col = (col+NCOLS-row)%NCOLS;
  //row=row*2+1-(col&1);
  col/=2;
  
  if (row<col) row++;
  *i = col;//lineToPin[col];
  *j = row;//lineToPin[row];
}

/* Set given pixel to "on"
 */
void set_pixel(int row, int col) {
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

void loop() {
  for (int col=0; col<NCOLS; col++) {
    for (int row=0; row<NROWS; row++) {
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


 
