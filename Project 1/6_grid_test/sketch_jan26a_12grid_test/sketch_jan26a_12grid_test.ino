#define NPINS 20
#define MINPIN 2
#define MAXPIN 13
#define NLINES 12

// Map from LED grid line number to arduino pin
const uint8_t lineToPin[NLINES] {
  2,
  4,
  6,
  8,
  10,
  14,
  15,
  16,
  9,
  7,
  5,
  3,
};

uint16_t MASK_INVALID[NLINES] {
  0b100000000000,
  0b010000000000,
  0b001000000000,
  0b000100000000,
  0b000010000000,
  0b000001000000,
  0b000000100000,
  0b000000010000,
  0b000000001000,
  0b000000000100,
  0b000000000010,
  0b000000000001};

uint16_t MASK_CYAN[NLINES] {
  0b010101010100,
  0b101010101010,
  0b010101010101,
  0b101010101010,
  0b010101010101,
  0b101010101010,
  0b010101010101,
  0b101010101010,
  0b010101010101,
  0b101010101010,
  0b010101010101,
  0b101010101010};

uint16_t MASK_YELLOW[NLINES] {
  0b000000000000,
  0b000001010101,
  0b000000000000,
  0b010001010101,
  0b000000000000,
  0b010100010101,
  0b000000000000,
  0b010101000101,
  0b000000000000,
  0b010101010001,
  0b000000000000,
  0b010101010100};

uint16_t MASK_RED[NLINES] {
  0b001010101010,
  0b000100000000,
  0b100010101010,
  0b000000000000,
  0b101000101010,
  0b000000000000,
  0b101010011010,
  0b000000000000,
  0b101010100010,
  0b000000000000,
  0b101010101000,
  0b000000000000};

// Color bitmasks
#define NCOLORS 3
uint16_t *MASK_COLORS[NCOLORS] = {MASK_RED, MASK_YELLOW, MASK_CYAN};

void setup() {
  
  // set all lines to high impedence
  for (int pin=0; pin<NPINS; pin++) {
    pinMode(pin, INPUT);
    digitalWrite(pin, LOW);
  }

  // Hasty blit: everything LOW and INPUT mode
  DDRB  = DDRC  = DDRD  = 0;
  PORTB = PORTC = PORTD = 0;
}

// Map from LED grid line number to arduino pin
const uint8_t pinMapping[20] {
  0,
  1,
  2,
  3,
  4,
  5,
  6,
  7,
  8,
  9,
  10,
  11,
  12,
  13,
  16,
  17,
  18,
  19,
  20,
  21,
};

uint32_t maskToDDR(uint16_t mask) {
  uint32_t mapped = 0;
  for (int i=0; i<NLINES; i++) {
    if (mask & (1<<(11-i)))
        mapped |= 1<<pinMapping[lineToPin[i]];
  }
}

void set_ddr(uint32_t b){
  DDRD = b&0xff;
  b >>= 8;
  DDRB = b&0xff;
  b >>= 8;
  DDRC = b&0xff;
}

void set_port(uint32_t b){
  PORTD = b&0xff;
  b >>= 8;
  PORTB = b&0xff;
  b >>= 8;
  PORTC = b&0xff;
}

void loop() {

  // Charlieplexing exhaustive scan, row-wise
  // LEDs with lower forward voltage will "steal"
  // current from those with higher forward voltage
  /*
  for (int line1=0; line1<NLINES; line1++) {
    PORTB=PORTC=PORTD=0;
    digitalWrite(lineToPin[line1],HIGH);
    DDRB=DDRC=DDRD=~0;
    delayMicroseconds(1000);
    DDRB=DDRC=DDRD=0;
  }
  */
  
  /*
  // Charlieplexing exhaustive scan, all pins
  // One light at a time
  for (int line1=0; line1<NLINES; line1++) {
    for (int line2=0; line2<NLINES; line2++) {
      if (line1==line2) continue;
      int pin1 = lineToPin[line1];
      int pin2 = lineToPin[line2];
      
      // Ensure high-impedence first
      //pinMode(pin1, INPUT);
      //pinMode(pin2, INPUT);
      // Set output states
      digitalWrite(pin1, HIGH);
      digitalWrite(pin2, LOW);
      // Activate outputs
      pinMode(pin1, OUTPUT);
      pinMode(pin2, OUTPUT);
      // Very short, try not to burn the LEDs
      delayMicroseconds(100);
      //delay(0.1); 
      // Inactivate the outputs
      pinMode(pin1, INPUT);
      pinMode(pin2, INPUT);
      // Set both to low
      //digitalWrite(pin1, LOW);
      //digitalWrite(pin2, LOW);
    }
  }
  */

  /*
  // Scan colors separately
  for (int ic=0; ic<NCOLORS; ic++) {
    uint16_t *color_mask = MASK_COLORS[ic];
    for (int led1=0; led1<NLINES; led1++) {
      for (int led2=0; led2<NLINES; led2++) {
        // Cycle through lights
        // One color only
        if ((color_mask[led1]&(1<<(11-led2)))) {
          int pin1 = lineToPin[led1];
          int pin2 = lineToPin[led2];
          // Set output states
          digitalWrite(pin1, HIGH);
          digitalWrite(pin2, LOW);
          // Activate outputs
          pinMode(pin1, OUTPUT);
          pinMode(pin2, OUTPUT);
          // Very short, try not to burn the LEDs
          delayMicroseconds(175);
          // Inactivate the outputs
          pinMode(pin1, INPUT);
          pinMode(pin2, INPUT);
        }
      }
    }
  }
  */

  
  // Row scan colors separately
  for (int ic=0; ic<NCOLORS; ic++) {
    uint16_t *color_mask = MASK_COLORS[ic];
    for (int led1=0; led1<NLINES; led1++) {
        PORTB=PORTC=PORTD=0;
        digitalWrite(lineToPin[led1],HIGH);
        DDRB=DDRC=DDRD=~0;
        for (int i=0; i<NLINES; i++) {
          if ((color_mask[led1]&(1<<(11-i)))) {
            int pin = lineToPin[i];
            int bip = pinMapping[pin];
            if (pin<8)
              DDRD = ~0;//1<<(7-bip;
            else
              pinMode(pin,OUTPUT);
              //pinMode(pin,OUTPUT);
            
            //if (bip<16)
            //  DDRB |= 1<<(bip-8);
            //else 
            //  DDRC |= 1<<(bip-16);
          }
        }
        // Masked write
        //set_ddr(maskToDDR(color_mask[led1]));
        pinMode(lineToPin[led1],OUTPUT);
        delayMicroseconds(1000);
        DDRB=DDRC=DDRD=0;
    }
  }
  

  
  /*
  // Charlieplexing exhaustive scan, all pins
  // One row, one color at a time
  for (int icolor=0; icolor<1; icolor++) {
    
    // Only scan those LEDs that match this color
    uint16_t *color_mask = MASK_COLORS[icolor];
    
    // Scan all rows
    for (int row=0; row<NLINES; row++) {
      // Assume all pins in input mode and low
      
      // Set the current row to high
      digitalWrite(lineToPin[row], HIGH);
      
      // Get the color mask for this row
      uint16_t row_mask = color_mask[row];
      
      // For all other pins we need to figure out if they should be on or off
      for (int col=0; col<NLINES; col++) {
        if (col==row) continue;
        pinMode(lineToPin[col],(row_mask>>col)&1? OUTPUT: INPUT);
      }
      // Show result by toggling row pin to output
      digitalWrite(lineToPin[row], OUTPUT);
      
      // Hasty blit: everything OUTPUT
      //DDRB  = DDRC  = DDRD  = 0xff;

        
      // Very short, try not to burn the LEDs
      delayMicroseconds(10000);
      
      // Hasty blit: everything LOW and INPUT mode
      DDRB  = DDRC  = DDRD  = 0;
      PORTB = PORTC = PORTD = 0;
    }
  }*/
}
