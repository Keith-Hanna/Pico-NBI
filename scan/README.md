# Scanning an I2C bus for devices

This example shows how to scan an I2C channel for attached devices.

We will assume that the I2C channel to be scanned uses GPIO pins 6 and 7 as SDA and  SCL, and that it is to be run at the `FAST_PLUS` rate (a clock speed of 1 MHz). Thus:

```c
    const uint SDA = 6;
    uint chan = nbi_create_channel(SDA, FAST_PLUS);
```

To determine whether there is a device that responds to a given I2C address, addr, we initiate a READ request for that address and check to see if an ACK response is received. For this we use the `nbi_send` function with these options:

- START  Since all I2C transaction must begin with a start sequence
- NACK   The I2C bus is bidirectional: at our end we emit a `NACK` signal
- REPLY   So as to be able to check the response
- STOP   Since all I2C transactions must end with a stop sequence.

So we define

```
    byte addr_R = (addr << 1) | READ;
    const options_t  opts = START | STOP | NACK | REPLY;
```

and initiate the I2C transaction with:

```
    nbi_send(chan, addr_R, opts);
```

We then check the response, ignoring the returned data. Any device should respond with an `ACK`:

```
    byte data;          // Just a placeholder
    bool resp = nbi_receive(chan, &data);        // true if ACK received
```

Finally, we define the range of I2C addresses to be scanned (not all  I2C addresses are valid):

```
    uint ADDR_START = 0x08,  ADDR_END = 0x77;
```

The overall scanning program is found in the accompanying file, `main.c`. When this program is run on the test board it yields the response:

```
Device found at address 0x20
Device found at address 0x68
```

These are, as expected, the default I2C addresses of a PCF8574 bus-expander and an MPU6050 accelerometer/gyro.
