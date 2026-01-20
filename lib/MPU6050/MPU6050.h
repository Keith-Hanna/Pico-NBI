#pragma once 

#include "pico/stdlib.h"

/*! \brief  Initialise the MPU6050 subsystem
 * 
 *  \param chnl The number of the I2C channel to be used
  */
void acc_gyro_init(uint chnl);


/*! \brief Initiate a read operation on the MPU6050. Note that this
 *  function is non-blocking: the actual reading of the peripheral
 *  will be carried out under interrupt control.
 */
void acc_gyro_read();

/*! \brief Specify a function that will be carried out once a read
 * operation has completed and the data it produced is available.
 * Note that, since the function will be executed _within_ an interrupt,
 * it must execute quickly.
 *
 * \param action  The function to be called 
 */
void acc_gyro_set_action(irq_handler_t action);


// The following functions all assume that the results from an
// earlier use of the above 'read' function are available. This
// pre-condition can be assured with the use of the 
// above 'set_action'  function.

/*! \brief  Yields the acceleration of the device along its X axis
 * 
 * \return The acceleration (in m/s/s) along the X axis (this
 *  is usually parallel to the shorter edge of the module on which the MPU6050
 *  is mounted).
 */ 
float acc_gyro_get_aX();


/*! \brief  Yields the acceleration of the device along its Y axis
 */
float acc_gyro_get_aY();


/*! \brief  Yields the acceleration of the device along its Z axis (this
 * is usually orthogonal to the plane of the module on which the MPU6050
 * is mounted).
 */
float acc_gyro_get_aZ();


/*! \brief  Yields the angular velocity of the device about its Z axis.
 * 
 * \return The angular velocity (in degrees/s) about the Z axis.
 */ 
float acc_gyro_get_wZ();


/*! \brief  Prints the accelerations and angular velecities about each 
 *  of the three axes.
*/
void acc_gyro_print();
