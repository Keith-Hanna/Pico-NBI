#include <stdio.h>  
#include "pico/stdlib.h"

#include "../lib/nbi.h"

static const uint SDA = 6;      // There is a PCF8674 connected to this GPIO
static const uint DA = 0x20;    // The I2C address to which a PCF8675 responds

int main() { 
    stdio_init_all();

    uint chan = nbi_create_channel(SDA, STANDARD);  

    const options_t  opts1 = START | NACK;
    const byte addr_R = (DA << 1) | READ;
    nbi_send(chan, addr_R, opts1);
   
    const options_t opts2 = STOP | REPLY | NACK;
    byte data = 0xFF;
    nbi_send(chan, data, opts2);

    nbi_receive(chan, &data);

    for (uint i = 0; i < 8; i++) {
        printf("Switch %u is %s\n", i, (data & 0x01) ? "ON" : "OFF");
        data = data >> 1;
    }
}

