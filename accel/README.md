# Reading the acceleration using an MPU6050

This example shows how to read the acceleration along the X axis of an MPU6050 accelerometer/gyro module.

This involves the following steps:

- Setting up the I2C channel
- Using an I2C  WRITE transaction to tell the device which of its many internal registers is to be read.
- Then using an I2C READ transaction to retrieve the data.

In detail, these steps are carried out as follows:

### Setting up the I2C channel

Assuming we are using GPIO pins 6 and 7 for the I2C bus and choose to drive it at the `STANDARD` clock rate, creating the channel is accomplished with:

```c
    const uint SDA = 6
    uint chan = nbi_create_channel(SDA, STANDARD);
```

### Specifying the X Acceleration register

We define the I2C base address of an MPU6050 device and (within the device) the address of its X acceleration register:

```c
    const uint DA = 0x68;      // Device address of an MPU6050
    const uint XH = 0x3B;      // The MSB of the X acceleration register
```

We start an I2C WRITE transaction to the device and then send it the register address, XH:

```c
    nbi_send(chan, (DA << 1) | WRITE, START | NACK);
    nbi_send(chan, XH, NACK)
```

### Retrieving the data

We then do an  I2C transaction (using a _repeated start_  since the previous transaction has not been terminated), this time for reading data from the device:

```c
    nbi_send(chan, (DA << 1) | READ,  START | NACK);
```

and then we retrieve the data and release the channel:

```c
    nbi_send(chan, 0xFF,  REPLY | NACK | STOP);
    byte data;
    nbi_receive(chan, &data);
```

The acceleration, `acc`, is returned as a signed 8-bit integer :

```c
    int8 acc = (int8_t) data;
```

The overall program, defined in the accompanying  file, `main.c`, encapsulates the above steps in a function and this function is repeatedly invoked to read the X-axis acceleration (which is along the long axis of an MPU6050 module). By default, the sensitivity of an MPU6050 is such that the acceleration, *g*, produced by the Earth's gravitational field will register as a value of about 64

This example has accessed the device in _blocking_ mode; in fact, each access takes about 390 us. In the `display-gyro`  example, we will see how to access the device in a _non-blocking_ manner.
