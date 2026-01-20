#include "nbi.h"  
 
#include <stdio.h>    
#include "hardware/pio.h"

#include "nbi.pio.h"

#define NMAX_CHNLS 12               // Assumes a Pico 2 

typedef struct {
    uint sda;
    speed_t speed;
    uint pio;
    uint sm;
    int tx_fifo_irq;
    int rx_fifo_irq;
} channel_t;

static char * speed2str(speed_t speed);

static channel_t chnls[NMAX_CHNLS];

uint configure_sm(uint pio_index, uint sm, uint i2c_bit_rate, uint sda);

static const int i2c_bit_rates[] = {100000, 400000, 1000000};  // Corresponding to {STANDARD, FAST, FAST_PLUS}
static char * i2c_speed_names[] = {" STANDARD", "     FAST", "FAST PLUS"};

// The number of the first unallocated channel  (channels range from  0 to NMAX_CHNLS - 1)
static uint next_chnl_num = 0;

uint nbi_create_channel(uint sda, speed_t speed) {
    uint chnl_num = next_chnl_num;
    next_chnl_num++;

    if (next_chnl_num > NMAX_CHNLS) {                     
        panic("No more channels can be allocated");
    }

    // Allocate SMs in succession for each new channel
    uint pio_index = chnl_num / 4;
    uint sm        = chnl_num % 4;
    PIO pio = pio_get_instance(pio_index);

    pio_sm_claim(pio, sm);

    uint i2c_bit_rate = i2c_bit_rates[speed];
    configure_sm(pio_index, sm, i2c_bit_rate, sda);

    channel_t * chnl_ptr = &chnls[chnl_num];
    chnl_ptr->sda = sda;
    chnl_ptr->speed = speed;
    chnl_ptr->pio = pio_index;
    chnl_ptr->sm = sm;
    chnl_ptr->tx_fifo_irq = -1;         // Marked as unallocated
    chnl_ptr->rx_fifo_irq = -1;

    return chnl_num;
}


static char * speed2str(speed_t speed) {
    return i2c_speed_names[speed];
}


char * rsp2str(bool resp) {
    return (resp == ACK) ? "ACK" : "NACK";
}


void nbi_print_channel_info() {
    printf("\nChannel     SDA       Speed     kHz  PIO   SM  TX_IRQ  RX_IRQ\n");
    for (uint cn= 0; cn < next_chnl_num; cn++) {
        channel_t chnl = chnls[cn];
        speed_t speed = chnl.speed;
        char * s = speed2str(speed);
        uint kHz = i2c_bit_rates[speed] / 1000;
        printf("     %2u      %2u   %9s    %4u    %1u    %1u",
            cn, chnl.sda, s, kHz, chnl.pio, chnl.sm);
        int irq_tx = chnl.tx_fifo_irq,  irq_rx = chnl.rx_fifo_irq;
        if (irq_tx >= 0) printf("      %2d", irq_tx); else printf("       -");
        if (irq_rx >= 0) printf("      %2d", irq_rx); else printf("       -");
        printf("\n");
    }
    printf("\n");
}



void nbi_send(uint channel, byte data, options_t options) {
    uint pio_index = channel / 4;
    uint sm        = channel % 4;
    PIO pio        = pio_get_instance(pio_index);

    uint record = ((data << 3) | options) << (32 - 12);

    // Push it onto the TX FIFO (blocking until FIFO is not full)
    pio_sm_put_blocking(pio, sm, record); 
}



bool nbi_receive(uint channel, byte * data_ptr) {
    uint pio_index = channel / 4;
    uint sm        = channel % 4;
    PIO pio        = pio_get_instance(pio_index);

    // Pull (blocking) the topmost  word from the RX FIFO
    uint record = pio_sm_get_blocking(pio, sm);

    // Extract the 'data' component of the record and update the argument
    *data_ptr = (byte) ((record >> 1) & 0xFF);

    bool resp = (~ record) & 0x1;   
    return resp;
}


static int  irq_TX_FIFO = -1;               // Will always be given the value 15 when allocate

// Specify an interrupt handler for TX FIFO interrupts for the given channel.

// NOTE: This is a _skeletal_ implementation of this function. It allows TX FIFO interrupt
// handling for at most a single channel, and this channel number must be in the range 0 to 3.
void TX_FIFO_set_irq_handler(uint chnl, irq_handler_t  handler) {

    if (irq_TX_FIFO >= 0) panic("Only a single TX FIFO interrupt handler can presently be defined");

    uint pio_index = chnl / 4;
    uint sm        = chnl % 4;
    if (pio_index != 0) panic("Interrupt handling is presently only available for channels 0 to 3");  

    PIO pio = pio_get_instance(pio_index);
  
    // Determine the IRQ that will be used for TX FIFO interrupts for PIO 0
    // (Use PIO IRQ index = 0 for TX FIFO interrupts)
    const uint irq_index = 0;
    irq_TX_FIFO = pio_get_irq_num(pio, irq_index);      // Always has the value 15
    irq_set_exclusive_handler(irq_TX_FIFO, handler);

    // Enable TX FIFO interrupt handling
    irq_set_enabled(irq_TX_FIFO, true);

    chnls[chnl].tx_fifo_irq = irq_TX_FIFO;
}


// Enable or disable TX FIFO NOT FULL interrupts on the given channel (assuming an interrupt
// handler has already been defined for this channel)
void enable_tx_fifo_interrupts(uint chnl, bool enable)  {
    if (irq_TX_FIFO < 0) panic("No interrupt handler has been defined for TX FIFO interrupts");

    PIO pio = pio_get_instance(chnl / 4);
    uint sm =                  chnl % 4;

    pio_interrupt_source_t source = pio_get_tx_fifo_not_full_interrupt_source(sm);
    pio_set_irq0_source_enabled(pio, source, enable);
}

static int  irq_RX_FIFO = -1;

// Specify an interrupt handler for RX FIFO interrupts for the given channel.
// NOTE: This is a _skeletal_ implementation of this function. It allows RX FIFO interrupt
// handling for at most a single channel, and this channel number must be in the range 0 to 3.
void RX_FIFO_set_irq_handler(uint chnl, irq_handler_t  handler) {
    if (irq_RX_FIFO >= 0) panic("Only a single RX FIFO interrupt handler can presently be defined");

    uint pio_index = chnl / 4;
    uint sm        = chnl % 4;
    if (pio_index != 0) panic("Interrupt handling is presently only available for channels 0 to 3");

    PIO pio = pio_get_instance(pio_index);

    // Determine the IRQ that will be used for RX FIFO interrupts for PIO 0
    // (Use PIO IRQ index = 1 for RX FIFO interrupts)
    const uint irq_index = 1;
    irq_RX_FIFO = pio_get_irq_num(pio, irq_index);
    irq_set_exclusive_handler(irq_RX_FIFO, handler);

    // Enable RX FIFO interrupt handling
    irq_set_enabled(irq_RX_FIFO, true);

    chnls[chnl].rx_fifo_irq = irq_RX_FIFO;
}


// Enable or disable RX FIFO NOT EMPTY interrupts on the given channel (assuming an interrupt
// handler has already been defined for this channel)
void enable_rx_fifo_interrupts(uint chnl, bool enable)  {
    if (irq_RX_FIFO < 0) panic("No interrupt handler has been defined for RX FIFO interrupts");

    PIO pio = pio_get_instance(chnl / 4);
    uint sm =                  chnl % 4;

    pio_interrupt_source_t source = pio_get_rx_fifo_not_empty_interrupt_source(sm);
    pio_set_irq1_source_enabled(pio, source, enable);
}

