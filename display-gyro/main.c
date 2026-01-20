#include <stdio.h>   
#include <string.h> 
#include <math.h> 

#include "pico/stdlib.h"
#include "pico/rand.h"

#include "../lib/nbi.h"
#include "../lib/SSD1306/SSD1306.h"
#include "../lib/MPU6050/MPU6050.h"

static void compute_orientation();
static float integrate(float x, bool reset);

static uint SDA_A = 6;              // MPU6050 connected to this line
static uint SDA_B = 10;             // SSD1306 connected to this line
static uint FONT_SELECT_PIN = 12;   // Jumper  connected to this line

static uint CALIB_TIME = 3 * 1000 * 1000;   // Time (microseconds) allocated for drift calibration

uint t_initial;     // Initial value of system timer (in microseconds)

int main() {  

    stdio_init_all();

    uint chan_A = nbi_create_channel(SDA_A, FAST_PLUS);   // For use by MPU6050
    uint chan_B = nbi_create_channel(SDA_B, STANDARD);    // For use by SSD1306

    // Set the display font size according to whether the font-select pin is grounded or not
    gpio_init(FONT_SELECT_PIN);
    gpio_set_dir(FONT_SELECT_PIN, false);
    gpio_pull_up(FONT_SELECT_PIN);
    sleep_ms(1);                // Allow time for pull-up to pull up . . .
    uint use_big_font = gpio_get(FONT_SELECT_PIN);
    font_size_t font_size = (use_big_font) ? BIG : SMALL;
   
    display_init(chan_B, font_size);  

    display_set(0, "Angle");
    display_set(1, "calib");

    display();
   
    acc_gyro_init(chan_A);
    t_initial = time_us_32();

    nbi_print_channel_info();
    printf("Display and AccGyro initialised\n");

    acc_gyro_set_action(compute_orientation);    

    // Start the iterative background processes
    acc_gyro_read();        // *** Comment out this line for benchmark comparison  ***


    // Now set a benchmark process going (running in the foreground), with
    // a display of the data it produces (an estimate of the value of pi) being continuously
    // updated on the SSD1306 
    const uint N_THROWS = 1000 * 1000;
    const double RADIUS = UINT64_MAX;
    volatile uint throws = 0, hits = 0;

    uint t1 = time_us_32();

    for (uint i = 0; i < N_THROWS; i++) {
        double x = get_rand_64();
        double y = get_rand_64();

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

    printf("Time taken: %5.1f s", elapsed_time_s);

    while (true) ;          // Allow time for any active interrupts to complete)
}


static volatile float drift_rate = 0.0;     // Initial assumption

static bool calibrated = false;

// This function will be invoked as soon as acc_gyro data is available
static void compute_orientation() {
    uint t_now = time_us_32() - t_initial;
    float wZ = acc_gyro_get_wZ() - drift_rate;         // Angular velocity (degrees/s) about Z axis

    float angle = integrate(wZ, false);

    if (calibrated) {
        display_set(1, "%4d", (int) angle);  
    } else {
        if (t_now > CALIB_TIME) {
            drift_rate = angle / (CALIB_TIME * 1.0E-6);
            integrate(0.0, true);
            calibrated = true;
        }
    }

    display();          // Comment out this line for benchmark test
    acc_gyro_read();
}


// A basic first-order integrator
static float integrate(float x, bool reset) {
    static float t_prev;            // WOULD BE BETTER to compute dt using Integer arithmetic  ********
    static float integral;

    float t = time_us_32() * 1.0E-6;

    if (reset) {
        integral = x;
    
    } else {
        float dt = t - t_prev;
        integral += x * dt;
    }

    t_prev = t;

    return integral;
}

