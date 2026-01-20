#include <stdio.h>   
#include <math.h>
#include "pico/stdlib.h"
#include "pico/rand.h"

#include "../lib/nbi.h"
#include "../lib/SSD1306/SSD1306.h"


int main() {    
    stdio_init_all();
    printf("Program  starting\n");

    // Setup an I2C channel
    const uint SDA = 10;            // There is an SSD1306 on this GPIO
    uint chan = nbi_create_channel(SDA,  FAST);     
     
    // Setup the display to run on the this I2C channel and to use a small font
    display_init(chan, SMALL);   

    display_set(0, "Estimating Pi");

    // Now set a simple benchmark process going (running in the foreground), with
    // a display of the data it produces (an estimate of the value of pi) being continuously
    // updated on the SSD1306 by an interrupt-driven process.
    const uint N_THROWS = 1000 * 1000;
    const double RADIUS = UINT64_MAX;
    volatile uint throws = 0, hits = 0;
    char buf[30];

    uint t1 = time_us_32();

    for (uint i = 0; i < N_THROWS; i++) {
        double x = (double) get_rand_64();
        double y = (double) get_rand_64();

        double r = sqrt(x * x + y * y);
        if (r <= RADIUS) hits++;
        throws++;
   
        float pi_estim = 4.0 * (float)hits / (float)throws;
        
        display_set(3, "Iterations: %u", throws);
        display_set(5, "Estimate: %6.5f", pi_estim);

        display();          // *** Comment out this line for benchmark comparison ***
    }

    uint t2 = time_us_32();

    float elapsed_time_s = (float)(t2 - t1) * 1.0e-6;
    display_set(7, "Time taken: %5.1f s", elapsed_time_s);

    display();

    while (true) ;      // Need to prevent the main program from terminating too
                        // soon since interrupts may still be active
}
