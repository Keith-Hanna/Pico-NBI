#include <stdio.h>  
#include "pico/stdlib.h"

#include "../lib/nbi.h"

static const uint SDA = 6;      

int main() { 
    stdio_init_all();

    uint chan = nbi_create_channel(SDA, FAST_PLUS);  

    const uint ADDR_START = 0x08,
               ADDR_END   = 0x77;
    const options_t  opts = START | STOP | NACK | REPLY;

    byte data;    // A placeholder (not used)

    for (uint a = ADDR_START; a <= ADDR_END; a++) {
        const byte addr_R = (a << 1) | READ;
        nbi_send(chan, addr_R, opts);       
        bool resp = nbi_receive(chan, &data);
        if (resp) printf("Device found at address 0x%02X\n", a);
    }
}
