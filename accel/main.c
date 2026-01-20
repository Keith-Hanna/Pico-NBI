#include <stdio.h>  
#include "pico/stdlib.h"

#include "../lib/nbi.h"

// Default accelerometer sensitivity such that values range between  +/-  2 g.
// Thus, readings of g range between about +/- 64.  (ie, +/- 1g)

const uint SDA = 6;	    // GPIO pin 6 (Pico pin 69 will be used as I2C SDA 

const uint DA = 0x68;	// I2C address to which an MPU6050 (with A0 grounded) responds
const uint XH = 0x3B;	// MPU6050 register address for the high-order byte of the X acceleration

int read_acc_X(uint chan) {
    byte data;
    options_t opts;

    data = (DA << 1) | WRITE;
    opts = START | NACK;
    nbi_send(chan, data, opts);

    opts = NACK;
    nbi_send(chan, XH, opts);

    data = (DA << 1) | READ;
    opts = START | NACK;
    nbi_send(chan, data, opts);

    data = 0xFF;
    opts = REPLY | NACK | STOP;
    nbi_send(chan, data, opts);

    nbi_receive(chan, &data);

    int8_t acc = (int8_t) data;
    return acc;
}


int main() { 
    stdio_init_all();

    uint chan = nbi_create_channel(SDA, STANDARD);  

    while (true) {
        uint t1 = time_us_32();
        int acc_X = read_acc_X(chan);
        uint t2 = time_us_32();

        printf("Acceleration: %5d     Time taken:  %u us\n", acc_X, (t2 - t1));

        sleep_ms(500);
    }
}
