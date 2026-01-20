#include  "content.h"        

#include <stdio.h>              // For the "vsprintf" function 
#include <string.h>        
#include "pico/platform.h"      // For the "RAM function" macro

#include "font-6x8.h"           // The small font
#include "Font-32-24-tr.h"      // The big   font

#define SPACE 0x20              // The index of the first printable ASCII character

#define WIDTH 128               // Screen width and height (pixels)     -- SHOULD come from display.c
#define HEIGHT 64

#define MAX_LINES_BIG 2         // Max number of lines of text that can be displayed for BIG font
#define MAX_LINE_LENGTH_BIG 5   // Max length of each line for BIG font

#define MAX_LINES_SMALL 8       // Likewise for the SMALL font
#define MAX_LINE_LENGTH_SMALL 21

#define BUFFER_LINES  MAX_LINES_SMALL
#define BUFFER_LINE_LENGTH MAX_LINE_LENGTH_SMALL

static char lines[BUFFER_LINES][BUFFER_LINE_LENGTH + 1];    // (Including space for null terminator)

static font_size_t  font_size;

void content_init(font_size_t fs) {       
    font_size = fs;

    // Clear the array by inserting a string terminator in each line
    for (uint i = 0; i < BUFFER_LINES; i++) {
        lines[i][0] = 0;        
    }          
}


// Note that this function simply stores the strings in a buffer. It is not influenced
// by considerations of font size.
// The decision as to which strings (and which prefix of those strings) to display and which 
// font to use is made on the fly if/when the information is accessed by the relevant scanning function.

void display_set(uint line_num, const char *fmt, ...) {
    if (line_num >= BUFFER_LINES) panic("Display:  Line number out of range\n");

    char ss[BUFFER_LINE_LENGTH + 1];    // Allowing for terminator
    va_list args;                       // Standard macro for dealing with variadic functions
    va_start(args, fmt);                // -- ditto --
    // Apply the formatting operation, yielding a string, ss, no longer than BUFFER_LINE_LENGTH
    vsnprintf(ss, BUFFER_LINE_LENGTH, fmt, args);   
    va_end(args);

    strcpy(lines[line_num], ss);
}


static volatile bool valid;     // True when scanning within a string, false beyond it

static byte small_font(uint i) {
    const uint font_width = 6;
    byte slice;
    uint row = i / WIDTH;                        
    uint slice_in_row = i % WIDTH;                
    uint col = slice_in_row / font_width;           
    if (col == 0) valid = true;                     // Marks the start of the string
    char ch = lines[row][col];                      // ASCI character code 
    if (ch == 0) valid = false;                     // Marks the end of the string
    if (valid) {
        uint slice_in_ch = slice_in_row % font_width;   
        uint ch1 = ch - SPACE;                      // Offset wrt first printable character
        uint char_index = ch1 * font_width;         // Index of first slice of the character
        slice = font68[char_index + slice_in_ch];   // The byte slice to be displayed
    } else {
        slice = 0;                                  // Display nothing after end of string
    }
    
    return slice;                
}


static byte big_font(uint i) {                      // i in 0 .. 1023
    const uint N_SUBROWS = 4;                       // Glyph size is 24 x 32
    const uint N_SLICES = 24;
    const uint N_CHARS = WIDTH / N_SLICES;          // Number of chars / line
    const uint MAX_SLICES = N_CHARS * N_SLICES;
    const uint GLYPH_SIZE = N_SLICES * N_SUBROWS;   // Number of bytes / glyph

    uint row = i / WIDTH;                   // row in frame
    uint j   = i % WIDTH;                   // slice in the frame
    uint line   = row / N_SUBROWS;          
    uint subrow = row % N_SUBROWS;          
    uint col = j / N_SLICES;            
    if (col == 0) valid = true;             // Marks the start of the string
    uint k   = j % N_SLICES;            
    char ch = lines[line][col];             // the ASCII character
    if (ch == 0) valid = false;
    byte slice;
    if (valid) {
        uint font_base_index = (ch - SPACE) * GLYPH_SIZE;
        uint font_index = font_base_index + (subrow * N_SLICES) + k;
        slice = font_32_24[font_index];
    } else {
        slice = 0;
    }
    
    return slice;
}


byte get_slice(uint i) {
    byte slice = (font_size == SMALL) ? small_font(i) : big_font(i);
    return slice;
}


uint display_get_n_rows() {
    return (font_size == SMALL) ? MAX_LINES_SMALL : MAX_LINES_BIG;
}

uint display_get_n_cols() {
    return (font_size == SMALL) ? MAX_LINE_LENGTH_SMALL : MAX_LINE_LENGTH_BIG;
}
