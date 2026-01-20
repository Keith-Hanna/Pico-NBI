#pragma once  
#include "pico/stdlib.h"       

typedef enum {STANDARD, FAST, FAST_PLUS} speed_t;
typedef enum {WRITE, READ} rw_t;
typedef enum {STOP = 0x01, REPLY = 0x02, ACK = 0x00, NACK = 0x04, START = 0x800} options_t;

typedef uint8_t byte;

/*! \brief Create a new I2C channel
 *
 *  \param sda  The GPIO pin number for the SDA line
 * 
 *  \param speed  One of the three standard I2C speeds: {STANDARD, FAST, FAST_PLUS}
 * 
 *  \return The number (0, 1, 2, ...) of the newly created channel
 */
uint nbi_create_channel(uint sda, speed_t speed);


/*! \brief Print a table summarising the properties of the created I2C channels
 *
 *
 */
void nbi_print_channel_info();

/*! \brief Create a record from the given parameters and emit it on the given I2C channel. 
 *
 *  \param chnl  The number of the I2C channel to be used
 *  
 *  \param data  The byte to be transmitted (or 0xFF if a byte is to be received)
 *
 *  \param options  A subset of {START, REPLY, ACK, NACK, STOP}
 * 
 *  \return
 */
void nbi_send(uint chnl, byte data, options_t options);

// Accept a record from the given channel. Update the data and return
// a response of true if an ACK is received, or false for a NACK

/*! \brief Accept a record from the given channel and update the data accordingly
 *
 *  \param  chnl    The number of the I2C channel to be used
 * 
 *  \param  data_ptr A pointer to the byte that will hold the receive data
 *
 *  \return True if an ACK response is received
 */
bool nbi_receive(uint chnl, byte * data_ptr);

/*! \brief Specifies the function to be used for handling TX FIFO "Not full"  interrupts
 *
 *  \param chnl The number of the I2C channel to be used
 * 
 *  \param handler  The function to be used to handle the interrupt
 *  
 *  \note Use of this function is presently restricted to channels 0 to 3, and to a _single_ use only.
 */
void TX_FIFO_set_irq_handler(uint chnl, irq_handler_t  handler);

/*! \brief Enables TX FIFO "Not full" interrupts.
 *
 *  \param chnl The number of the I2C channel to be used
 * 
 *  \param enable  True if interrupts are to be enabled
*/
void enable_tx_fifo_interrupts(uint chnl, bool enable);


/*! \brief Specifies the function to be used for handling RX FIFO "Not empty"  interrupts
 *
 *  \param chnl The number of the I2C channel to be used
 * 
 *  \param handler  The function to be used to handle the interrupt
 *  
 *  \note Use of this function is presently restricted to channels 0 to 3, and to a _single_ use only.
 */
void RX_FIFO_set_irq_handler(uint chnl, irq_handler_t  handler);

/*! \brief Enables RX FIFO "Not empty" interrupts
 *
 *  \param chnl The number of the I2C channel to be used
 * 
 *  \param enable  True if interrupts are to be enabled
 */
void enable_rx_fifo_interrupts(uint chnl, bool enable);

