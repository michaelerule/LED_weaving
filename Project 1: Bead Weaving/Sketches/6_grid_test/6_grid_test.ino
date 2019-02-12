/*
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

#define NLINES 6
#define SCANRATE 130

// Map from LED grid line number to arduino pin
const uint8_t lineToPin[NLINES] {2,6,10,12,8,4};

volatile uint8_t display_data[NLINES];

// Volatile variables for timer interrupt routine
volatile int line1 = 0;
volatile int line2 = 0;

//SIGNAL(TIMER2_OVF_vect) {
SIGNAL(TIMER2_COMPA_vect) 
{
  // Clear previous state
  //int p1 = lineToPin[line1];
  //int p2 = lineToPin[line2];
  //pinMode(p1,0);
  //pinMode(p2,0);

  // Hasty blit: everything LOW and INPUT mode
  DDRB  = DDRC  = DDRD  = 0;
  //PORTB = PORTC = PORTD = 0;

  // Increment loop counters
  line2++;
  if (line2>=NLINES) {
    line2=0;
    line1++;
    if (line1>=NLINES)
      line1=0;
  }
  if (line1==line2) return;
  
  int p1 = lineToPin[line1];
  int p2 = lineToPin[line2];

  // Show ONE pin
  if ((display_data[line1]>>line2)&1) {
    digitalWrite(p1,1);
    digitalWrite(p2,0);
    pinMode(p1,1);
    pinMode(p2,1);
  }
}

// Commands to set all three ports at the same time
// Note that this only makes sense on the 168/368 based boards
// (it will not work on the Leonardo and other 32U boards)
void set_ddr(uint32_t b){
  DDRD=b&0xff; b>>=8;
  DDRB=b&0xff; b>>=8;
  DDRC=b&0xff;
}
void set_port(uint32_t b){
  PORTD=b&0xff; b>>=8; 
  PORTB=b&0xff; b>>=8; 
  PORTC=b&0xff;
}

void setup() {
  // Hasty blit: everything LOW and INPUT mode
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
  TCCR2B=(TCCR2B&0b11111000)|0b011;

  // Set the output compare A bit for timer 2
  TIMSK2 |= 0b010;

  // Set timer period
  OCR2A = 80;
  
  // Allow interrupts
  sei();
}

#define NROWS 6
#define NCOLS 5

void set_pixel(int row, int col) {
  if (col%2==1) 
    row = (row-1)%NROWS;
  row = (row+NROWS-col/2)%NROWS;
  col = (row+col+NCOLS)%NCOLS;
  int i = row;
  int j = col<row? col : col+1;
  display_data[i]|=0b1<<j;
}

void clear_pixel(int row, int col) {
  if (col%2==1) 
    row = (row-1)%NROWS;
  row = (row+NROWS-col/2)%NROWS;
  col = (row+col+NCOLS)%NCOLS;
  int i = row;
  int j = col<row? col : col+1;
  display_data[i]&=~(1<<j);
}

void loop() {

  
  for (int iter=0; iter<3; iter++) {
  for (int row=0; row<NROWS; row++) {
    for (int col=0; col<NCOLS; col++) {
      set_pixel(row,col);
      delay(10);
    }
  }
  for (int col=0; col<NCOLS; col++) {
    for (int row=0; row<NROWS; row++) {
      clear_pixel(row,col); 
      delay(10);
    }
  }
  }
  
  for (int iter=0; iter<5; iter++) {
    for (int row=0; row<NROWS+2; row++) {
      for (int col=0; col<NCOLS; col++) {
        if (row<NROWS) set_pixel(row,col);
        delay(10);
        clear_pixel((row-2)%NROWS,col); 
      }
    }
    delay(100);
  }
  
  for (int row=NROWS-1; row>=0; row--) {
    for (int col=0; col<NCOLS; col++) {
      set_pixel(row,col);
      delay(15);
      clear_pixel((row+1)%NROWS,col); 
    }
  }
  for (int col=0; col<NCOLS; col++) {
    delay(15);
    clear_pixel(0,col); 
  }
  delay(100);
}
