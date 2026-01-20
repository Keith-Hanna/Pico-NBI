# Reading switch values

This example shows how to use the three core functions in the NBI library  to create an I2C interface and access a device --- specifically a PCF8574 *bus expander* chip connected to a set of 8 switches (each representing 0 or 1). 

The three  basic functions are:

- `uint nbi_create_channel(uint sda, speed_t speed)`  
  Creates a new I2C channel.

- `void nbi_send(uint channel, byte data, options_t options)`  
  Transmits a byte of data over the channel.

- `bool nbi_receive(uint channel, byte * data_ptr)`  
  Receives data placed on the channel by the device.

## Creating an I2C channel

First, we initialise a new I2C channel using `nbi_create_channel` . This involves choosing an adjacent pair of pins for the SDA and SCL signals and selecting the speed at which the interface will run. The I2C standard defines three primary speeds which we name `STANDARD` (100 kb/s), `FAST` (400 kb/s) and `FAST_PLUS` (1 Mb/s). In this example, we use GP6 and GP7 to realise the I2C bus and use the `STANDARD` speed. 

```c
    uint chan = nbi_create_channel(6, STANDARD);  
```

This function returns the index of the newly created channel number (0, 1, etc.). With a Pico 2 up to 12 such channels can be created.

## Reading the switch values

To read the 8-bit switch values from the PCF8574 we must execute an I2C transaction that performs the following steps:

- Place the PCF8574's device address on the I2C bus in READ mode.

- Initiate the READ operation

- Receive the returned data

##### Step 1: Address the device

The base I2C address for a PCF8574 is 0x20. For a Read operation, we format the address byte as follows:

```c
    const uint DA = 0x20;
    const byte addr_R = (DA << 1) | READ;
```

We set these options:

- START  Every I2C transaction must begin with a start sequence.
- NACK   Since the I2C bus is bidirectional, we emit a NACK signal at our end.

so we define

```c
    const options_t  opts1 = START | NACK;
```

The operation is then carried out by

```c
    nbi_send(chan, addr_R, opts1);
```

##### Step 2: Initiate the Read

Next, we trigger the actual read operation. We call `nbi_send`  again, this time using the "neutral" data value of 0xFF with the following options:

- NACK   Used here since this is the *final* operation in the transaction
- REPLY  To signal that we expect to receive a result (the bitwise AND of the device data and the "neutral" value 0xFF)
- STOP  Every I2C transaction must finish with a stop sequence

This operation is carried out by

```c
      const options_t opts2 = NACK | REPLY | STOP;
      nbi_send(chan, 0xFF, opts2);
```

##### Step 3:  Receive the data

Finally, we retrieve the data requested in the previous step using nbi_receive. This function captures the data currently on the I2C bus and returns true if an ACK  is received:

```c
        byte data;
        nbi_receive(chan,&data);
```

## Expected output

The complete program (found in the file main.c)  retrieves and display the settings of the 8 switches. When executed, it produces a response like:

```
Switch 0 is ON
Switch 1 is OFF
Switch 2 is OFF
. . .
Switch 7 is ON
```
