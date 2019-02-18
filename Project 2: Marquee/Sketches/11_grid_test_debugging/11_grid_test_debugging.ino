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

#define NLINES 11

// Map from LED grid line number to arduino pin
const uint8_t lineToPin[NLINES] {2,4,5,7,6,3,11,12,10,9,8,};

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

// Volatile variables for timer interrupt routine
volatile uint16_t display_data[NLINES];
volatile int      line1 = 0;
volatile int      line2 = 0;

SIGNAL(TIMER2_COMPA_vect) 
{
  // everything to INPUT mode
  DDRB  = DDRC  = DDRD  = 0;

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

void setup() {
  // Set everything LOW and INPUT mode
  DDRB  = DDRC  = DDRD  = 0;
  PORTB = PORTC = PORTD = 0;
  
  // Zero display buffer
  for (int line=0; line<NLINES; line++)
    display_data[line]=0;
    
  // Display buffer all on
  for (int line=0; line<NLINES; line++)
    display_data[line]=~0;

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

  /*
  while (1)
  for (int i1=0;i1<NLINES;i1++) {
    i1=0;
    int p1=lineToPin[i1];
    digitalWrite(p1,LOW);
    pinMode(p1,OUTPUT);
    for (int iter=0; iter<100; iter++) {
    for (int i2=0;i2<NLINES;i2++) {
      int p2=lineToPin[i2];
      if (p1==p2) continue;
      digitalWrite(p2,HIGH);
      pinMode(p2,OUTPUT);
      delay(1);
      pinMode(p2,INPUT);
      digitalWrite(p2,LOW);
    }
    }
    pinMode(p1,INPUT);
  }
  */
}

#define NROWS NLINES
#define NCOLS (NLINES-1)

void set_pixel(int row, int col) {
  //if (col%2==1) 
  //  row = (row-1)%NROWS;
  //row = (row+NROWS-col/2)%NROWS;
  //col = (row+col+NCOLS)%NCOLS;
  int i = row;
  int j = col<row? col : col+1;
  display_data[i]|=0b1<<j;
}

void clear_pixel(int row, int col) {
  //if (col%2==1) 
  //  row = (row-1)%NROWS;
  //row = (row+NROWS-col/2)%NROWS;
  //col = (row+col+NCOLS)%NCOLS;
  int i = row;
  int j = col<row? col : col+1;
  display_data[i]&=~(1<<j);
}

void loop() {
  /*
  for (int row=0; row<NROWS; row++) {
    for (int col=0; col<NCOLS; col++) {
      set_pixel(row,col);
      delay(15);
    }
  }
  for (int col=0; col<NCOLS; col++) {
    for (int row=0; row<NROWS; row++) {
      clear_pixel(row,col); 
      delay(15);
    }
  }
  */
}
