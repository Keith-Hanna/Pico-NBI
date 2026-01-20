#include "MPU6050.h"

#include <stdio.h>  
#include "pico/stdlib.h"

#include "../nbi.h"

#define N_VALUES 7
#define ASF (2 * 9.81 / INT16_MAX)
#define GSF (250.0 / INT16_MAX)    // Gyroscope Scale Factor for converting readings to degree/s [see p14]

const uint DA = 0x68;    // I2C address to which an MPU6050 (with A0 grounded) responds
const uint RA = 0x3B;    // MPU6050 register address for the high-order byte of the X acceleration
const byte ADDRESS_W = (DA << 1) | WRITE;
const byte ADDRESS_R = (DA << 1) | READ;

static char * names[] = {"aX", "aY", "aZ", "T ", "wX", "wY", "wZ"};

static uint the_chan;
static irq_handler_t the_action;
static byte high, low;
static volatile uint16_t data[N_VALUES];

static uint state;

void acc_gyro_read() {
    nbi_send(the_chan, ADDRESS_W, START | NACK);
    nbi_send(the_chan, RA, NACK);
    nbi_send(the_chan, ADDRESS_R, START | NACK);
    nbi_send(the_chan, 0xFF, ACK | REPLY);

    state = 0;
    enable_rx_fifo_interrupts(the_chan, true);
}


static void handler() {
    uint j = state / 2;     // aX, aY, aZ, T, wX, wY, wZ
    uint k = state % 2;     // High byte, Low byte

    if (k == 0) {
        nbi_receive(the_chan, &high);
    } else {
        nbi_receive(the_chan, &low);
        data[j] = (high << 8) | low;
    }

    if (j == (N_VALUES - 1)) {
        if (k == 0) {
            nbi_send(the_chan, 0xFF, NACK | REPLY | STOP);
            state++;
        } else {
            enable_rx_fifo_interrupts(the_chan, false);
            if (the_action != NULL) {
                the_action();
            }
        }
    } else {
        nbi_send(the_chan, 0xFF, ACK | REPLY);
        state++;
    }
}


void acc_gyro_init(uint chan) {
    the_chan = chan;

    the_action = NULL;
    RX_FIFO_set_irq_handler(the_chan, handler);
}


void acc_gyro_set_action(irq_handler_t action) {
    the_action = action;
}


float acc_gyro_get_aX() {
    float aX = ASF * (float) (int16_t) data[0];
    return aX;
}

float acc_gyro_get_aY() {
    float aY = ASF * (float) (int16_t) data[1];
    return aY;
}

float acc_gyro_get_aZ() {
    float aZ = ASF * (float) (int16_t) data[2];
    return aZ;
}

float acc_gyro_get_wZ() {
    float wZ = GSF * (float) (int16_t) data[6];
    return wZ;
}

void acc_gyro_print() {
    for (uint i = 0; i < N_VALUES; i++) {
        uint d = data[i];
        int di = (int16_t) d;
        float df = (float) di;
        if (i < 3) printf("%6.2f   ", ASF * df);
        if (i > 3) printf("%6.2f   ", GSF * df);
    }
    printf("\n");
}

