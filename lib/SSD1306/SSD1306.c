#include  "SSD1306.h"         

#include <stdio.h>                // For the "vsprintf" function 
#include "hardware/pio.h"
#include "pico/platform.h"        // For the "RAM function" macro

#include "../nbi.h"
#include "content.h"


#define WIDTH 128                // Screen width and height (pixels)
#define HEIGHT 64

#define FRAME_SIZE (WIDTH * HEIGHT / 8)    // Number of bytes to paint a frame 

#define MONITOR 12              // Use GP12 for scope monitor


const uint I2C_ADDRESS =  0x3C;                            // The default I2C address of SSD1306 and SSD1309 devices
const uint ADDR_W = (I2C_ADDRESS << 1) | WRITE;

// Incantation to initialise an SSD1306 or SSD1309 
static byte magic_1[] = {
    0xAE,   0x00,   0x10,   0x40,    0x20,   0x00,   0x81,   0xFF,
    0xA6,   0xA8,   0x3F,   0xD3,   0x00,   0xD5,   0x80,   0xD9,
    0x22,   0xDA,   0x12,   0xDB,   0x40,   0x8D,   0x14,   0xA4,
    0xAF
};

static void interrupt_handler();

static irq_handler_t the_action_function = NULL;

// The 'another' flag indicates that a further scan is required after the ongoing
// one completes.
// The 'busy' flag indicates that the system is in the process of displaying a frame.
volatile static bool another;
volatile static bool busy;

void display_set_action_function(irq_handler_t action_function) {
    the_action_function = action_function;
}


// The number of the I2C channel that will be used by the SSD1306
static uint the_cn, the_pio_indx, the_sm;
static PIO the_pio;


static void cmd(byte b) {
    nbi_send(the_cn, ADDR_W, START | NACK);
    nbi_send(the_cn, 0x00, NACK);        
    nbi_send(the_cn, b, STOP | NACK);    
}


void display_init(uint cn, font_size_t font_size) {
    the_cn       = cn;
    the_pio_indx = cn / 4;
    the_sm       = cn % 4;

    the_pio = pio_get_instance(the_pio_indx);

    content_init(font_size);
        
    // Initialise the SSD1306 
    for (int i = 0; i < sizeof(magic_1); i++) {
        cmd(magic_1[i]);        
    }

    another = false;
    busy = false;

    // Associate the interrupt handler with this I2C channel
    TX_FIFO_set_irq_handler(the_cn, interrupt_handler);

    // Set up a GPIO pin for monitoring interrupt duration
    gpio_init(MONITOR);
    gpio_set_dir(MONITOR, GPIO_OUT);
    gpio_put(MONITOR, 0);
}


const uint cmds[] =    {0x22, 0x00, 0x07,        // Setup page start and end addresses (8 rows)
                        0x21, 0x00, 0x7F};        // Setup Col  start and end addresses (128 cols)


// --- The state machine --------------------------------------------

typedef enum {S0, S1, S2, S3, S4, T0, T1, T2, T3, U0, U1, U2} state_t;

static volatile state_t state;                
static volatile uint i;            // Frame index, typically running from 0 to 1023 (8 rows x 128 slices/row)

void __not_in_flash_func(interrupt_handler)() {       // LOAD THIS FUNCTION INTO RAM 
    gpio_put(MONITOR, 1);                             // For monitoring interrupt duration

    switch (state) { 

        case S0:
            i = 0;
            nbi_send(the_cn, ADDR_W, START | NACK);
            state = T1;
            break;

        case S1:
            nbi_send(the_cn, ADDR_W, START | NACK);
            state = S2; 
            break;

        case S2:
            i = 0;
            nbi_send(the_cn, 0x40, NACK);
            state = U0;
            break;

        case S3:
            if (another) {            // Initiate the pending scan
                state = S0;
                another = false;
            } else {
                enable_tx_fifo_interrupts(the_cn, false);        // Disable TX FIFO interrupts    
                busy = false;
                state = S4;                                    
                if (the_action_function != NULL) {
                    the_action_function();                  // CALLBACK
                }
            } 
            break;

        case S4:                // This state should never be encountered (since interrupts should have been disabled)
            panic("Display: stop at S4");


        case T1:
            nbi_send(the_cn, 0x00, NACK);
            state = T2;
            break;

        case T2:
            nbi_send(the_cn, cmds[i], STOP | NACK);
            i++;
            state = (i < 6) ? T3 : S1;
            break;                    

        case T3: 
            nbi_send(the_cn, ADDR_W, START | NACK);
            state = T1;
            break;

        case U0:
            byte slice;
            slice = get_slice(i);
            
            bool stop = (i == (FRAME_SIZE - 1));
            uint x = stop ? STOP : 0;
            nbi_send(the_cn, slice, x | NACK);
            i++;
            state = (i == FRAME_SIZE) ? S3 : U0;
            break; 

        default:
            panic("Bad state in switch");
    }

    gpio_put(MONITOR, 0);  
}


bool display_try() {
    if (state == S0 || state == S4) {
        state = S0;
        enable_tx_fifo_interrupts(the_cn, true);      
        return true;
    } else {
        return false;
    }
}


void display() {     
    if (busy) {
        another = true;
        return;
    } else {
        state = S0;
        busy = true;
        enable_tx_fifo_interrupts(the_cn, true);
    }
}

