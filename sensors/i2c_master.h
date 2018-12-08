//*************************************************************************************
/** \file i2c_master.h
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

// This define prevents this file from being included more than once in a *.cpp file
#ifndef _I2C_MASTER_H_
#define _I2C_MASTER_H_

#include <stdlib.h>                         // Standard C/C++ library stuff
#include "FreeRTOS.h"                       // Header for the RTOS
#include "semphr.h"                         // FreeRTOS semaphores (we use a mutex)
#include "emstream.h"                       // Header for base serial devices


/// @brief The desired bit rate for the I2C interface in bits per second.
#define I2C_BITRATE         100000L

/// @brief This value is put in the TWBR register to set the desired bitrate. 
const uint8_t I2C_TWBR_VALUE = (((F_CPU / I2C_BITRATE) - 16) / 2);

/// @brief Macro to print I2C interface debugging information if needed.
#define I2C_DBG(x)  if (p_serial) *p_serial << x
// #define I2C_DBG(x)

#if defined (__AVR_ATmega324__) || defined (__AVR_ATmega324P__) \
	|| defined (__AVR_ATmega644__) || defined (__AVR_ATmega644P__) \
	|| defined (__AVR_ATmega1284__) || defined (__AVR_ATmega1284P__) \
	|| defined (__DOXYGEN__)
	/** @brief   General purpose I/O port used by the I2C port's SDA line.
	 *  @details This is the port which shares a pin with the I2C port's SDA line. It 
	 *           is used to test the status of the SDA wire directly. Since different 
	 *           AVR processors have their I2C lines connected to different I/O port 
	 *           pins, the value of this macro is processor dependent.
	 *           | Processors                             | SDA | SCL |
	 *           |:---------------------------------------|:---:|:---:|
	 *           | ATmega128, 1281, 2561, 640, 1280, 2560 | PD1 | PD0 |
	 *           | ATmega164/P, 324/P, 644/P, 1284/P      | PC1 | PC0 |
	 */
	#define I2C_PORT_SDA    PORTC

	/// @brief   Data direction register used by the I2C port's I/O port.
	#define I2C_DDR_SDA

	/// @brief   Pin number of the SDA line, used as a regular pin, in its I/O port.
	#define I2C_PIN_SDA     1
#elif defined (__AVR_ATmega128__) || defined (__AVR_ATmega1281__) \
	|| defined (__AVR_ATmega2561__) || defined (__AVR_ATmega2560__)
	#define I2C_PORT_SDA    PORTD
	#define I2C_PIN_SDA     1
#endif


//-------------------------------------------------------------------------------------
/** @brief   Driver class for an I2C (also known as TWI) bus on an AVR processor. 
 *  @details It encapsulates basic I2C functionality such as the ability to send and
 *           receive bytes through the TWI bus. Currently only operation of the AVR as
 *           an I2C bus master is supported; this is what's needed for the AVR to 
 *           interface with most I2C based sensors. 
 */

class i2c_master
{
protected:
	/// This is a pointer to a serial port object which is used for debugging the code.
	emstream* p_serial;

	/// @brief   Mutex used to prevent simultaneous uses of the I2C port.
	SemaphoreHandle_t mutex;

public:
	// This constructor sets up the driver
	i2c_master (emstream* = NULL);

	// This method causes a start condition on the TWI bus
	bool start (void);

	// This method causes a repeated start on the TWI bus
	bool restart (void);

	/** @brief   Cause a stop condition on the I2C bus.
	 *  @details This method causes a stop condition on the I2C bus. It's inline 
	 *           because causing a stop condition is a one-liner (in C++ and even 
	 *           in assembly). 
	 *  @return  @c false meaning OK because there's no test to show an error
	 */
	bool stop (void)
	{
		TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
		return false;
	}

	// This method sends a byte to a device on the I2C bus
	bool write (uint8_t address, uint8_t reg, uint8_t data);

	// This method writes many bytes to a device on the I2C bus
	bool write (uint8_t address, uint8_t reg, uint8_t* p_buf, uint8_t count);

	// Read a byte from a device on the I2C bus
	uint8_t read (uint8_t address, uint8_t reg);

	// Read a bunch of bytes from a device on the I2C bus
	bool read (uint8_t address, uint8_t reg, uint8_t *p_buffer, uint8_t count);

	// Write one byte to the I2C bus
	bool write_byte (uint8_t byte);

	// Read one byte from the I2C bus
	uint8_t read_byte (bool ack);

	// Method to check the status of the SDA line
	bool check_SDA (void);

	// Method to check if there's a device at a given I2C bus address
	bool ping (uint8_t address);

	// Method which scans the I2C bus for devices and prints the result
	void scan (emstream* p_ser);

	/** @brief   Take the mutex associated with this I2C bus.
	 *  @details This method takes the mutex which controls access to this I2C bus.
	 *           The mutex is automatically taken by the @c read() and @c write()
	 *           commands, but when a device driver needs to use the @c read_byte()
	 *           and @c write_byte() commands directly, that driver needs to handle
	 *           the mutex with this command and the @c give_mutex() command.
	 */
	void take_mutex (void)
	{
		xSemaphoreTake (mutex, portMAX_DELAY);
	}

	/** @brief   Give back the mutex associated with this I2C bus.
	 *  @details This method gives the mutex which controls access to this I2C bus.
	 *           It's a complement for the @c take_mutex() method.
	 */
	void give_mutex (void)
	{
		xSemaphoreGive (mutex);
	}
};

#endif // _I2C_MASTER_H_

