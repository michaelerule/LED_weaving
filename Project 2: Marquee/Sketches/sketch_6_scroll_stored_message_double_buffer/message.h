/** Packed font data, message, and routines for scrolling to marquee display
 *  
 *  This file was prepared using `prepare_c_datastructures.py` in the folder
 *  `Project 2: Marquee/serial communication/`.
 *  
 *  It expects the following drwaing function to have been defined:
 *  void draw_column(uint8_t pixel_data)
 * 
 */


// routines to store/access data from flash (program) memory directly
#include <avr/pgmspace.h>

// fixed numeric type definitions (uint8_t etc.)
#include <stdint.h>

// if the screen width has not otherwise been defined, default it to 5x22
// (the size of a charlieplexed display using 11 control lines)
#ifndef NCOLS
#define NCOLS (22)
#endif
#ifndef NROWS
#define NROWS (5)
#endif

// Relative character offsets (data starts at meanWidth*index+charOffset)
#define NCHARS (111)
#define MEANWIDTH (3)
const PROGMEM int8_t charOffset[NCHARS+1]={0,0,0,0,0,0,0,0,0,0,0,0,0,2,4,4,4,4,4,4,4,4,4,6,6,6,6,6,6,6,6,6,5,5,5,3,2,2,0,2,2,2,2,2,2,2,1,1,1,3,3,3,2,0,0,0,0,0,0,0,0,0,0,-2,-4,-5,-7,-9,-10,-10,-11,-11,-13,-15,-15,-15,-15,-13,-11,-9,-5,-5,-3,-4,-5,-6,-7,-5,-5,-5,-5,-7,-8,-9,-9,-9,-9,-9,-4,1,1,0,0,0,-2,-4,-4,-4,-4,-4,-4,-4};

// Bit-packed character data (lowest order bits used first)
const PROGMEM uint8_t charData[]={30,9,30,31,21,10,31,17,17,31,17,14,31,21,17,31,5,1,31,17,25,31,4,31,21,29,21,24,17,31,31,4,27,31,16,16,31,2,4,2,31,31,2,4,8,31,31,17,31,31,5,7,15,9,31,31,5,26,23,21,29,1,31,1,31,16,31,15,16,15,15,16,12,16,15,27,4,27,7,28,7,25,21,19,26,22,30,31,18,28,30,18,18,30,18,31,14,26,22,31,5,22,18,30,31,2,28,26,16,26,31,4,26,31,30,2,30,2,28,30,2,28,30,18,30,30,10,14,14,10,30,30,2,2,22,18,26,31,20,30,16,30,14,16,14,14,16,28,16,14,18,12,18,22,24,14,26,22,31,25,21,18,17,21,10,7,4,31,23,21,9,14,21,29,1,1,31,31,21,31,7,5,31,31,17,31,16,24,0,0,26,10,4,4,4,14,4,10,10,1,21,3,23,3,21,14,21,17,14,17,31,31,31,6,9,18,9,6,14,19,21,12,20,2,21,31,21,8,6,21,11,4,26,21,12,2,1,2,10,31,10,31,10,31,17,17,31,14,17,17,14,14,17,13,9,14,3,0,3,24,4,3,3,4,24,31,4,10,10,4,4,31,17,17,31,4,14,14,14,4,21,4,31,0,18,8,8,18,0,31,31,0,10,16,16,10,0,31,11,25,13,27,13,15,25,9,14,26,10,30,31,13,18,13,29,20,29,29,16,29,26,16,26,30,19,26,22,19,30};

// Mapping from ascii character value to index into our character data
#define NONE (0xFF)
const PROGMEM uint8_t ASCIIMap[128]={255,8,74,75,76,95,96,255,255,255,255,97,255,255,98,99,100,101,102,103,105,106,107,108,109,110,255,255,255,255,255,255,64,71,87,81,78,79,77,72,84,85,73,68,63,67,62,88,61,52,53,54,55,56,57,58,59,60,66,65,91,69,92,70,86,0,1,2,3,4,5,6,7,104,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,82,89,83,80,255,255,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,93,90,94,255,255};

// Message to be displayed (non-ascii as custom escape codes)
#define NMESSAGE (676)
const PROGMEM uint8_t message[676]="\n"
  "Questions\n"
  "-1-\n"
  "WHY\n"
  "The forlorn voice inside me:\n"
  "\"Why?\"\n"
  "And nights and the morning and the noon\n"
  "\"Why?\"\n"
  "The question that eats away the sun\n"
  "The pain wrapping my skin\n"
  "A silent exile to the void\n"
  "\"Why?\"\n"
  "A susurrus\n"
  "From the funeral of answers\n"
  "\"Why? Why? Why?\"\n"
  "\n"
  "-2-\n"
  "ANATOCISMUS\n"
  "Why don\'t you have eyes\n"
  "When looking at me\n"
  "Why don\'t you have ears\n"
  "When I\'m talking\n"
  "Such a big mouth you have\n"
  "Is it to eat me?\n"
  "\n"
  "-3-\n"
  "ANSWER QUESTION\n"
  "Not knowing what to ask\n"
  "You ask\n"
  "Of the pain whose answer is\n"
  "Hidden in the question\n"
  "\n"
  "-4-\n"
  "QUESTION FREE\n"
  "There were no questions\n"
  "Void and pain\n"
  "Question free darkness\n"
  "Writhed in void\n"
  "\n"
  "DENOUMENT\n"
  "\n"
  "A rippling\n"
  "Radiance\n"
  "Knot within me\n"
  "Come loose\n"
  "Tears\n"
  "Running\n"
  "\n"
  "\x04\x04\x04\n"
  "\n"
  "\n";


/** Routines to retrieve next column of pixel data in the scrolling text.
 *  
 *  Better implementation:
 * 
 *  Rather than have a complex state machine that returns the columns of 
 *  the scrolling marquee data, we could use more conventional looping & 
 *  recursion to extract the screen data, and have this routine call a 
 *  "draw" function as it unpacks new data. This drawing function could 
 *  have an internal state that causes it to wait a certain amount of time 
 *  between columns for smooth scrolling. 
 */
 
/** Scrolling routine
 *  Unpacks message and font pixel data and sends it to screen
 */
#define GETCHAR(V,I) ((uint8_t)pgm_read_byte(&V[(I)]))
#define GETINT8(V,I) (( int8_t)pgm_read_byte(&V[(I)]))
void scroll_message() {

  for (int messageidx=0; messageidx<NMESSAGE; messageidx++) {
        
    // Get next character from the message string
    uint8_t character = GETCHAR(message,messageidx); 
        
    // Special characters: these correspond to control actions and not
    // printed display characters. 
    // Presently, only the newline action is defined. 
    // We set it to scroll the text off screen.
    if (character=='\n') {
      for (int i=0; i<NROWS; i++)
        draw_column(0x00);
      continue;
    }
        
    // Remap char to an index into the custom font data
    // The setup script shouldn't allow undefined characters, 
    // but skip them if we happen to encounter them anyway.
    uint8_t chi = GETCHAR(ASCIIMap,character);
    if (chi==NONE) continue;
        
    // Draw the pixel data
    int stop = GETINT8(charOffset,chi+1)+MEANWIDTH*(chi+1);
    for (int i=GETINT8(charOffset,chi  )+MEANWIDTH*chi; i<stop; i++)
      draw_column(GETCHAR(charData,i)); 
        
    // Draw a blank column between characters
    draw_column(0x00);
  }
}

