// This is a lightly edited extract drawn from
// the "pio_i2c_bus_scan" example in the Pico SDK set of PIO examples. 

#include "nbi_pio_driver.h"
#include "nbi.pio.h"   

#include "hardware/pio.h"
#include <stdio.h>          

#define CLOCK_FREQ (150 * 1000 * 1000)          // For Pico 2  (150 MHz)
#define CLOCK_CYCLES_PER_BIT 32         

uint offset;

// Boilerplace code for configuring the given pio/sm.  Returns the PIO program offset.
uint configure_sm(uint pio_index, uint sm, uint i2c_bit_rate, uint sda) {
    const uint scl = sda + 1; 

    printf("About to configure SM %u of PIO %u\n", sm, pio_index);
    PIO pio = pio_get_instance(pio_index);

    // Load the PIO program (required for each bank of 4 SMs)
    if (sm == 0) {
        // Load the PIO program named "transfer". Note the auto-generated name formed
        // by appending the string "_program"  to the name of the PIO program.
        offset = pio_add_program(pio, &transfer_program);
        printf("Program loaded into PIO %u with offset = %u\n", pio_index, offset);
        if (offset < 0) {
            panic("Out of PIO program space!");
        }
    }

    // Get the default SM configuration
    // (Again, note the auto-generated name, this time "transfer_program_get_default_config") 
    pio_sm_config c = transfer_program_get_default_config(offset); 
    
    // IO mapping
    sm_config_set_out_pins(&c, sda, 1);
    sm_config_set_set_pins(&c, sda, 1);
    sm_config_set_in_pins(&c, sda);
    sm_config_set_jmp_pin(&c, sda);

    sm_config_set_sideset_pins(&c, scl);

    // OSR configuration:  shift left, and no autopull
    sm_config_set_out_shift(&c, false, false, 32);

    // ISR configuration: shift left, and no autopush
    sm_config_set_in_shift(&c, false, false, 32);

    // Set the PIO clock rate
    float clk_div = (float)CLOCK_FREQ / (i2c_bit_rate * CLOCK_CYCLES_PER_BIT);
    printf("I2C bus freq set to %d kHz with PIO clock divider set to %f\n", i2c_bit_rate / 1000, clk_div);
    sm_config_set_clkdiv(&c, clk_div);

    // Configure the GPIO pull-ups
    gpio_pull_up(sda);
    gpio_pull_up(scl);

    // Configure the pins so that a 0 pulls the line low while a 1 doesn't affect it
    uint32_t sda_scl= (1u << sda) | (1u << scl);
    pio_sm_set_pins_with_mask(pio, sm, sda_scl, sda_scl);       // Set selected pins to high (to avoid glitches)
    pio_sm_set_pindirs_with_mask(pio, sm, sda_scl, sda_scl);    // Set selected pindirs to OUTPUT

    pio_gpio_init(pio, sda);
    gpio_set_oeover(sda, GPIO_OVERRIDE_INVERT);
    pio_gpio_init(pio, scl);
    gpio_set_oeover(scl, GPIO_OVERRIDE_INVERT);

    // Set the outputs of SDA and SCL so that, when enabled,
    // they will pull their bus low.
    pio_sm_set_pins_with_mask(pio, sm, 0, sda_scl);

    // Configure the SM
    pio_sm_init(pio, sm, offset, &c);
    // and set it going
    pio_sm_set_enabled(pio, sm, true);
    printf("SM %u of PIO %u has now been configured and enabled\n", sm, pio_index);

    return offset;
} 



