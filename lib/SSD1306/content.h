#pragma once 

#include "pico/stdlib.h"

typedef enum {SMALL, BIG} font_size_t;

typedef uint8_t byte;


// Initialises the content provider
void content_init(font_size_t fs);


// Yields the height and width of the display (expressed in characters, not pixels)
// for the current font size.
uint display_get_n_rows();
uint display_get_n_cols();


// Displays, on the given line, the given string. (Lines are indexed from 0 upwards.)
// The format string behaves in a similar way to the way it does in the "printf" function. 
void display_set(uint line, const char *fmt, ...);


// Returns the byte to be painted at raster position i
byte get_slice(uint i);
