# Real-time programming

This example shows how two peripherals, each on their own I2C bus, can be driven under interrupt control.  It is intended to be a realistic example of a typical high-frequency real-time control task.

The example has three tasks running concurrently:

- A benchmark task that estimates a value for pi (the same task as in the previous example)

- A background task that uses an MPU6050 accelerometer-gyro to measure its angular velocity, $\omega$, about the vertical axis and integrates this to keep track of its current orientation, $\theta = \int \omega .dt$ 

- A background task that uses an SSD1306 to display both the current state of the benchmark task and the current orientation of the test board as it is rotated.

In order that the orientation can be determined accurately (possibly in the presence of rapid, jerky rotations of the test board) it is essential that the angular velocity is sampled at the maximum rate the device and the interface can support. For this reason, the MPU6050  and the SSD1306 are placed on separate I2C busses. 

### The NBI  MPU6050 functions

The MPU6050 is a combined accelerometer/gyroscope. An earlier example showed how it can be driven using the low-level primitive functions in the NBI library. In this example we use the high-level MPU6050-specific functions defined in the library. These include:

- `acc_gyro_init(chan)`  This initialises the system to use I2C channel number `chan` for this device.

- `acc_gyro_read()`  This function initiates a Read operation on the device but does not itself yield any result. The function is non-blocking: it returns immediately.

- `acc_gyro_set_action(fn)`   This function is used to specify that, after a Read operation has been completed and the results are available, the function `fn` will be called (since it will be invoked within an interrupt it must execute quickly).

- `acc_gyro_get_wZ( )`   This function is one of the several functions that yield the outcome of the most recent 'read'  operation. This particular function yields the angular velocity, $\omega$, about the Z axis of the device in degrees/s.
This function, like related functions such as `acc_gyro_get_aX()` that yields the acceleration in m/s<sup>2</sup> along the X axis, is non-blocking. It *assumes* that the results from an earlier use of the `acc_gyr_read()` function are available. This pre-condition can be *guaranteed* with the use of the above `acc_gyro_set_action(fn)` function.

### The program

The program (see accompanying file, `main.c`) begins by setting up two I2C channels, one for the MPU6050 and the other for the  SSD1306.

The program provides an option of two display modes, according to whether or not a jumper is in place. With the jumper not in place the large font is used and this gives an easy-to-read display showing just the test board's orientation. With the jumper in place, the small font is used and this allows the current state of  the orientation,  of the $\pi-$estimation and the elapsed time all to be displayed. 

The program then uses `display_init` to set up the SSD1306 display on one of the I2C channels, then `display_set` to specify the initial display  and finally `display` to cause it to be displayed.

Next,  `acc_gyro_init` is used to set up the MPU6050 on the other I2C channel. 

The `nbi_print_channel_info()` function then provides a summary of the I2C channels:

```
Channel     SDA       Speed     kHz  PIO   SM  TX_IRQ  RX_IRQ
      0       6   FAST PLUS    1000    0    0       -      16
      1      10    STANDARD     100    0    1      15       -
```

##### The orientation task

The heart of the overall program is the use of the Z-axis gyro to keep track of the board's current orientation. To this end, the statement

```c
acc_gyro_set_action(compute_orientation);
```

is used to specify that, as soon as a set of readings (of accelerations and angular velocities) become available (which will happen within an interrupt) the `compute orientation` function will be called.

The next statement, a call of the `acc_gyro_read` function, launches a cycle that will repetitively read the gyro, and then update and display the current orientation.  This function (it is defined immediately following the main program), captures the angular velocity, $\omega_Z$ and then, using a simple first-order integrator, integrates it to obtain the current orientation, `angle`.

Because gyros are prone to drift, it is necessary that the gyro is first calibrated by determining its drift rate and then subtracting this offset from future readings. This calibration (indicated by the  `calibrated` flag) is carried out during the first 3 seconds (`CALIB_TIME`) of the program's execution.  The board must remain stationary during the time when the display shows "CALIB".

Once the current orientation is available, the `display` function is invoked to update the display and then the `compute_orientation` function is invoked. And this causes the entire cycle to be repeated (under interrupt control), endlessly.

Finally, the estimate-$\pi$ benchmark program is started; it runs for a million iterations, each time invoking the `display_set` and `display`  functions to keep the display of its current state up to date.

### Performance
The time taken to run the benchmark task, with both the SSD1306 and the MPU6050 being continuously accessed at their maximal rate is about 31.9 s.  This compares with the time taken, 25.1 s, to run the benchmark task by itself.

These videos [*NOT* yet included] demonstrate the program in action both with and without the font-size jumper present:

[Video clips to be inserted]

Due to the rapid rate at which the gyro is read, the program manages to maintain the orientation accurately even in the presence of quite rapid rotational transients.



