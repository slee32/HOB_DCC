//*************************************************************************************
/** \file hmc6352.cpp
 *    This file contains a driver class for a Honeywell HMC6352 compass chip. It only
 *    supports setting up the chip and asking it for a compass heading, which are 
 *    usually the most important things we ask of a compass. 
 *
 *  Revised:
 *    - 12-24-2012 JRR Original file
 *    - 05-04-2015 JRR Cleaned up code and comments to make it a better ME405 example
 *
 *  License:
 *    This file is copyright 2012-2015 by JR Ridgely and released under the Lesser GNU 
 *    Public License, version 2. It is intended for educational use only, but its use
 *    is not limited thereto. */
/*    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 *    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 *    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 *    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
 *    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUEN-
 *    TIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
 *    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
 *    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
 *    OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 *    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */
//*************************************************************************************

#include "FreeRTOS.h"                       // Main header for FreeRTOS
#include "task.h"                           // Needed for the vTaskDelay() function
#include "hmc6352.h"                        // Header for this compass chip


//-------------------------------------------------------------------------------------
/** @brief   Create an HMC6352 driver object.
 *  @details This constructor creates an HMC6352 driver object and saves pointers to
 *           the serial port and I2C bus driver associated with it.
 *  @param   p_I2C A pointer to the I2C driver used to talk to the sensor
 *  @param   p_ser_dev A serial device for debugging text (default: @c NULL)
 */

hmc6352::hmc6352 (i2c_master* p_I2C, emstream* p_ser_dev)
{
	p_i2c = p_I2C;                          // Save pointers
	p_serial = p_ser_dev;
}


//-------------------------------------------------------------------------------------
/** @brief   Set the mode byte of the HMC6352 compass module.
 *  @details This method sets the mode byte of the HMC6352. The mode is only set in 
 *           RAM unless saved in EEPROM by some other bit of code.
 *  @param   mode_byte A byte specifying the mode into which the sensor will be set
 *  @return  @c true if there was a problem setting the mode, @c false if it went OK
 */

bool hmc6352::set_mode (uint8_t mode_byte)
{
	// These bytes are sent to the sensor to set its mode
	uint8_t set_mode_bytes[2];
	set_mode_bytes[0] = 0x74;
	set_mode_bytes[1] = mode_byte;

	// Make sure the mode byte is legal; if it's not, don't send it
	if ((mode_byte & 0b10001100) || ((mode_byte & 0b00000011) == 0b00000011))
	{
		return true;
	}

	// Now send the I2C address, write to RAM command, address of mode control byte,
	// and what to write into the mode control byte. The return value is true if
	// there were problems writing this stuff and false if things went OK
	return (p_i2c->write (HMC6352_ADDRESS, 'G', set_mode_bytes, 2));
}


//-------------------------------------------------------------------------------------
/** @brief   Get a heading from the HMC6352 compass module.
 *  @details This method gets a heading from the HMC6352 compass. It assumes that the 
 *           sensor is in standby mode; this means that it is necessary to send an 
 *           'A' command, then wait until the sensor has measured and computed its 
 *           heading, then read the heading. All this means that the \c heading() 
 *           method takes quite some time to run. @b Note: This method must be called
 *           from within a FreeRTOS task because the @c vTaskDelay() method is used
 *           to implement an efficient wait while the sensor prepares a reading. 
 *  @return  The measured heading, in tenths of a degree of angle, or 999.9 for errors
 */

int16_t hmc6352::heading (void)
{
	union                                   // This union holds the raw data of a
	{                                       // heading from the sensor. It gives us
		uint8_t byte[2];                    // two bytes which we put together to make
		uint32_t word;                      // a 16-bit word that will be returned
	}
	raw_data;

	p_i2c->take_mutex ();                   // Make sure nobody's using the I2C bus

	// Start the I2C communication
	p_i2c->start ();

	// Write the device address to the bus. If no acknowledgement is received, or if
	// no acknowledgement is received when writing an 'A' command, give up and send
	// a ridiculous value (999.9 degrees of angle) to indicate that things didn't work
	if (!(p_i2c->write_byte (HMC6352_ADDRESS)) || !(p_i2c->write_byte ('A')))
	{
		p_i2c->stop ();
		p_i2c->give_mutex ();
		return 9999;
	}

	// Stop this phase of the communication. 
	p_i2c->stop ();

	// Wait about 6 ms for the heading to be calculatificationized. Note that this 
	// technique is specific to the HMC6352 sensor; most other sensors don't need this
	vTaskDelay (configMS_TO_TICKS (7));

	// Start communications again, then send the sensor's read address, then get data
	p_i2c->start ();
	p_i2c->write_byte (HMC6352_ADDRESS | 0x01);
	raw_data.byte[1] = p_i2c->read_byte (true);    // Read MSB (true = send ACK)
	raw_data.byte[0] = p_i2c->read_byte (false);   // Read LSB (false = send NACK)
	p_i2c->stop ();

	p_i2c->give_mutex ();                   // Let others use the I2C bus

	return (raw_data.word);                 // Return the bytes we read, as one word
}


//-------------------------------------------------------------------------------------
/** This overloaded operator writes information about the status of an HMC6352 sensor 
 *  to the serial port. The printout shows the current heading as text, with the 
 *  integer part of the heading, a decimal point, and the fractional part. It is 
 *  assumed that the sensor is in standby mode, so the "A" command is sent to the 
 *  sensor, then the driver waits for the sensor to compute and make available the 
 *  heading. This process means that this \c << operator is quite slow to run. 
 *  @param ser_dev A reference to the serial device on which we're writing information
 *  @param sensor A reference to the sensor object whose status is being written
 *  @return A reference to the same serial device on which we write information.
 *          This is used to string together things to write with "<<" operators
 */

emstream& operator << (emstream& ser_dev, hmc6352& sensor)
{
	int16_t the_heading = sensor.heading ();
	div_t parts_is_parts = div (the_heading, 10);

	ser_dev << parts_is_parts.quot << '.' << parts_is_parts.rem;

	return (ser_dev);
}

