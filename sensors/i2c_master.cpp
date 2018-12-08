//*************************************************************************************
/** @file i2c_master.cpp
 *    This file contains a base class for classes that use the I2C (also known as TWI)
 *    interface on an AVR. The terms "I2C" (the two means squared) and "TWI" are
 *    essentially equivalent; Philips has trademarked the former, and Atmel doesn't pay
 *    them a license fee, so Atmel chips that meet exactly the same specification are
 *    not allowed to use the "I2C" name, even though everything works the same. 
 *
 *    Note: The terms "master" and "slave" are standard terminology used in the
 *    electronics industry to describe interactions between electronic components only.
 *    The use of such terms in this documentation is made only for the purpose of 
 *    usefully documenting electronic hardware and software, and such use must not be
 *    misconstrued as diminishing our revulsion at the socially diseased human behavior 
 *    which is described using the same terms, nor implying any insensitivity toward
 *    people from any background who have been affected by such behavior. 
 *
 *  Revised:
 *    - 12-24-2012 JRR Original file, as a standalone HMC6352 compass driver
 *    - 12-28-2012 JRR I2C driver split off into a base class for optimal reusability
 *    - 05-03-2015 JRR Added @c ping() and @c scan() methods to check for devices
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
#include "i2c_master.h"                     // Header for this class


//-------------------------------------------------------------------------------------
/** @brief   This constructor creates an I2C driver object.
 *  @param   p_debug_port A serial port, often RS-232, for debugging text 
 *                        (default: @c NULL)
 */

i2c_master::i2c_master (emstream* p_debug_port)
{
	p_serial = p_debug_port;                // Set the debugging serial port pointer

	TWBR = I2C_TWBR_VALUE;                  // Set the bit rate for the I2C port

	// Create the mutex which will protect the I2C bus from multiple calls
	if ((mutex = xSemaphoreCreateMutex ()) == NULL)
	{
		I2C_DBG ("Error: No I2C mutex" << endl);
	}

}


//-------------------------------------------------------------------------------------
/** @brief   Cause a start condition on the I2C bus.
 *  @details This method causes a start condition on the I2C bus. In hardware, a start
 *           condition means that the SDA line is dropped while the SCL line stays 
 *           high. This gets the attention of all the other devices on the bus so that
 *           they will listen for their addresses. 
 *  @return  @c true if there was an error, @c false if the I2C start was successful
 */

bool i2c_master::start (void)
{
	// Cause the start condition to happen
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);

	// Wait for the TWINT bit to indicate that the start condition has been completed
	for (uint8_t tntr = 0; !(TWCR & (1 << TWINT)); tntr++)
	{
		if (tntr > 250)
		{
			return true;
		}
	}

	// Check that the start condition was transmitted OK
	if ((TWSR & 0b11111000) != 0x08)
	{
		return true;
	}
	return false;
}


//-------------------------------------------------------------------------------------
/** @brief   Cause a repeated start condition on the I2C bus. 
 *  @details This method causes a repeated start condition on the I2C bus. This is 
 *           similar to a regular start condition, except that it occurs during an
 *           already running conversation, and a different return code is expected if 
 *           things go as they should. 
 *  @return  @c true if there was an error, @c false if the I2C restart was successful
 */

bool i2c_master::restart (void)
{
	// Cause the start condition to happen
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);

	// Wait for the TWINT bit to indicate that the start condition has been completed
	for (uint8_t tntr = 0; !(TWCR & (1 << TWINT)); tntr++)
	{
		if (tntr > 250)
		{
			return true;
		}
	}

	// Check that the start condition was transmitted OK
	if ((TWSR & 0b11111000) != 0x10)
	{
		I2C_DBG (PMS ("I2C re-start: 0x") << hex << TWSR << PMS (" not 0x10")
			 << dec << endl); 
	}
	return false;
}


//-------------------------------------------------------------------------------------
/** @brief   Send a byte to a device on the I2C bus.
 *  @details This method performs an I2C send to transfer a byte to a remote device. 
 *           The expected response code varies depending on what is being sent at what
 *           time; some examples of expected responses are as follows:
 *           - @c 0x18 - When one has sent SLA+W, a slave address for a write command, 
 *                       and a good ACK has been received
 *           - @c 0x40 - When one has sent SLA+R, a slave address for a read command, 
 *                       and a good ACK has been received
 *           - @c 0x28 - When one has transmitted a data byte and received a good ACK
 *  @param   byte The byte which is being sent to the remote device
 *  @return  @c true if an acknowledge bit was detected, @c false if none was seen
 */

bool i2c_master::write_byte (uint8_t byte)
{
	TWDR = byte;
	TWCR = (1 << TWINT) | (1 << TWEN);
	for (uint8_t tntr = 0; !(TWCR & (1 << TWINT)); tntr++)
	{
		if (tntr > 250)
		{
			I2C_DBG (PMS ("I2C send timeout") << endl);
			return false;
		}
	}

	// Check that the byte was transmitted OK by looking at status bits 7-3
	uint8_t status = TWSR & 0b11111000;
	if (status == 0x18 || status == 0x28 || status == 0x40)
	{
		return true;
	}
	else                                    // Hopefully we got 0x20 or 0x30 for valid
	{                                       // NACK; anything else would be an error
		return false;
	}
}


//-------------------------------------------------------------------------------------
/** @brief   Receive a byte from a device on the I2C bus.
 *  @details This method receives a byte from the I2C bus. Other code must have 
 *           already run the @c start() command and sent and address byte which got the
 *           other device's attention.
 *  @param   ack @c true if we are to end our data request with ACK, telling the slave 
 *               that we want more data after this; false if we end our data request 
 *               with NACK, telling the slave that we don't want more data after this
 *  @return  The byte which was received from the remote device or @c 0xFF for an error
 */

uint8_t i2c_master::read_byte (bool ack)
{
	uint8_t expected_response;              // Code we expect from the AVR's I2C port

	if (ack)  // If we expect more data after this, send an ACK after we get the data
	{
		TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
		expected_response = 0x50;
	}
	else      // We're not going to ask for more data; send a NACK when we're done
	{
		TWCR = (1 << TWINT) | (1 << TWEN);
		expected_response = 0x58;
	}

	for (uint8_t tntr = 0; !(TWCR & (1 << TWINT)); tntr++)
	{
		if (tntr > 250)
		{
			return 0xFF;
		}
	}

	// Check that the address thingy was transmitted OK
	if ((TWSR & 0b11111000) != expected_response)
	{
		return 0xFF;
	}

	return TWDR;
}


//-------------------------------------------------------------------------------------
/** @brief   Read one byte from a slave device on the I2C bus.
 *  @details This method reads a single byte from the device on the I2C bus at the
 *           given address.
 *  @param   address The I2C address for the device. The address must already have 
 *                   been shifted so that it fills the 7 @b most significant bits of 
 *                   the byte. 
 *  @param   reg The register address within the device from which to read
 *  @return  The byte which was read from the device
 */

uint8_t i2c_master::read (uint8_t address, uint8_t reg)
{
	xSemaphoreTake (mutex, portMAX_DELAY);  // Take the mutex or wait for it

	start ();                               // Start the discussion
	if (!write_byte (address) || !write_byte (reg))
	{
		I2C_DBG ("<r:0>");
		xSemaphoreGive (mutex);
		return true;
	}

	stop ();
	start ();
// 	restart ();                             // Repeated start condition
	if (!write_byte (address | 0x01))       // Address with read bit set
	{
		I2C_DBG ("<R:d>");
		xSemaphoreGive (mutex);
		return 0xFF;
	}
	uint8_t data = read_byte (false);       // Read a byte, sending a NACK
	stop ();                                // Stop the conversation

	xSemaphoreGive (mutex);                 // Return the mutex, as we're done
	return (data);
}


//-------------------------------------------------------------------------------------
/** @brief   Read multiple bytes from a slave device on the I2C bus.
 *  @details This method reads multiple bytes from the device on the I2C bus at the
 *           given address.
 *  @param   address The I2C address for the device. The address should already have 
 *                   been shifted so that it fills the 7 @b most significant bits of 
 *                   the byte. 
 *  @param   reg The register address within the device from which to read
 *  @param   p_buffer A pointer to a buffer in which the received bytes will be stored
 *  @param   count The number of bytes to read from the device
 *  @return  @c true if a problem occurred during reading, @c false if things went OK
 */

bool i2c_master::read (uint8_t address, uint8_t reg, uint8_t *p_buffer, uint8_t count)
{
	xSemaphoreTake (mutex, portMAX_DELAY);  // Take the mutex or wait for it

	start ();                               // Start the discussion

	if (!write_byte (address) || !write_byte (reg))
	{
		I2C_DBG ("<R:0>");
		xSemaphoreGive (mutex);
		return true;
	}

	stop ();
	start ();
// 	restart ();                             // Repeated start condition

	if (!write_byte (address | 0x01))       // Address with read bit set
	{
		I2C_DBG ("<R:D>");
		xSemaphoreGive (mutex);
		return true;
	}
	for (uint8_t index = count - 1; index; index--)
	{
		*p_buffer++ = read_byte (true);     // Read bytes 
	}
	*p_buffer++ = read_byte (false);        // Last byte requires acknowledgement
	stop ();

	xSemaphoreGive (mutex);                 // Return the mutex, as we're done
	return false;
}


//-------------------------------------------------------------------------------------
/** @brief   Write one byte to a slave device on the I2C bus.
 *  @details This method writes a single byte to the device on the I2C bus at the
 *           given address.
 *  @param   address The I2C address for the device. The address should already have 
 *                   been shifted so that it fills the 7 @b most significant bits of 
 *                   the byte. 
 *  @param   reg The register address within the device to which to write
 *  @param   data The byte of data to be written to the device
 *  @return  @c true if there were problems or @c false if everything worked OK
 */

bool i2c_master::write (uint8_t address, uint8_t reg, uint8_t data)
{
	xSemaphoreTake (mutex, portMAX_DELAY);  // Take the mutex or wait for it

	start ();                               // Start the discussion
	if (!write_byte (address) || !write_byte (reg) || !write_byte (data))
	{
		I2C_DBG ("<w:0>");
		xSemaphoreGive (mutex);
		return true;
	}
	stop ();                                // Stop the conversation

	xSemaphoreGive (mutex);                 // Return the mutex, as we're done
	return false;
}


//-------------------------------------------------------------------------------------
/** @brief   Write a bunch of bytes to a slave device on the I2C bus.
 *  @details This method writes a number of bytes to the device on the I2C bus at the
 *           given address.
 *  @param   address The I2C address for the device. The address should already have 
 *                   been shifted so that it fills the 7 @b most significant bits of 
 *                   the byte. 
 *  @param   reg The register address within the device to which to write
 *  @param   p_buf Pointer to a memory address at which is found the bytes of data to 
 *                 be written to the device
 *  @param   count The number of bytes to be written from the buffer to the device
 *  @return  @c true if there were problems or @c false if everything worked OK
 */

bool i2c_master::write (uint8_t address, uint8_t reg, uint8_t* p_buf, uint8_t count)
{
	xSemaphoreTake (mutex, portMAX_DELAY);  // Take the mutex or wait for it

	start ();                               // Start the discussion
	if (!write_byte (address) || !write_byte (reg))
	{
		I2C_DBG ("<W:0>");
		xSemaphoreGive (mutex);
		return true;
	}
	for (uint8_t index = 0; index < count; index++)
	{
		if (!write_byte (*p_buf++))
		{
			I2C_DBG ("<W:" << index << '>');
			xSemaphoreGive (mutex);
			return true;
		}
	}
	stop ();

	xSemaphoreGive (mutex);                 // Return the mutex, as we're done
	return false;
}


//-------------------------------------------------------------------------------------
/** @brief   Check the status of the SDA line.
 *  @details This method just finds whether the SDA line is currently high or low. This
 *           method may be needed when some sensors may need to wait for the SDA line
 *           to be directly manipulated by the sensor to indicate that data is 
 *           available.
 *  @return  @c true if the SDA line is high, @c false if it is low
 */

bool i2c_master::check_SDA (void)
{
	if (I2C_PORT_SDA & (1 << I2C_PIN_SDA))
	{
		return true;
	}
	else
	{
		return false;
	}
}


//-------------------------------------------------------------------------------------
/** @brief   Check if a device is located at the given address.
 *  @details This method causes an I2C start, then sends the given address and checks
 *           for an acknowledgement. After that, it just sends a stop condition. 
 *  @param   address The I2C address for the device. The address should already have 
 *           been shifted so that it fills the 7 @b most significant bits of the byte. 
 *  @return  @c true if a device acknowledged the given address, @c false if not
 */

bool i2c_master::ping (uint8_t address)
{
	xSemaphoreTake (mutex, portMAX_DELAY);  // Take the mutex or wait for it

	start ();
	bool is_someone_there = write_byte (address);
	stop ();

	xSemaphoreGive (mutex);

	return is_someone_there;
}


//-------------------------------------------------------------------------------------
/** @brief   Scan the I2C bus, pinging each address, and print the results.
 *  @details This handy dandy utility function scans each address on the I2C bus and
 *           prints a display showing the addresses at which devices responded to a 
 *           "ping" with acknowledgement.
 *  @param   p_ser A pointer to a serial device on which the scan results are printed
 */

void i2c_master::scan (emstream* p_ser)
{
	*p_ser << PMS ("   0 2 4 6 8 A C E") << hex << endl;
	for (uint8_t row = 0x00; row < 0x10; row++)
	{
		*p_ser << (uint8_t)row << '0';
		for (uint8_t col = 0; col < 0x10; col += 2)
		{
			p_ser->putchar (' ');
			if (ping ((row << 4) | col))
			{
				p_ser->putchar ('@');
			}
			else
			{
				p_ser->putchar ('-');
			}
		}
		*p_ser << endl;
	}
	*p_ser << dec;
}

