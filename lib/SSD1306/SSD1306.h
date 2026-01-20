#pragma once 

#include "pico/stdlib.h"

#include "content.h"

/*! \brief Initialise the display subsystem that controls the SSD1306 
 *
 * \param chnl  The I2C channel to be used
 * 
 * \param size  The size (SMALL or BIG) of the font to be used
 */
void display_init(uint chnl, font_size_t  size);     
                              

/*! \brief  Initiate the display of the current frame. 
 * Note that this function is non-blocking; the actual update of the display frame
 * buffer is carried out later under interrupt control.
 */
void display();

/*! \brief Specifies a function that will be executed once an update of the frame
 *  buffer has been completed.
 *  Note that, since the function will be executed _within_ an interrupt,
 *  it must execute quickly.
 * 
 * \param action The function to be called
 */
void display_set_action_function(irq_handler_t action);   


