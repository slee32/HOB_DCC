//*************************************************************************************
/** @file    emstream.cpp
 *  @brief   Source code for a base class that imitates C++ I/O streams on serial 
 *           devices. 
 *  @details This file contains a base class for devices which send information in 
 *           text form over serial devices. Example devices are serial ports (both 
 *           traditional RS-232 ports and USB-serial adapters) and wireless modems. 
 *
 *  Revised:
 *    \li 04-03-2006 JRR For updated version of compiler
 *    \li 06-10-2006 JRR Ported from C++ to C for use with some C-only projects
 *    \li 08-11-2006 JRR Some bug fixes
 *    \li 03-02-2007 JRR Ported back to C++. I've had it with the limitations of C.
 *    \li 04-16-2007 JO  Added write (unsigned long)
 *    \li 07-19-2007 JRR Changed some character return values to bool, added m324p
 *    \li 01-12-2008 JRR Added code for the ATmega128 using USART number 1 only
 *    \li 02-13-2008 JRR Split into base class and device specific classes; changed
 *                       from write() to overloaded << operator in the "cout" style
 *    \li 05-12-2008 ALS Fixed bug in which signed numbers came out unsigned
 *    \li 07-05-2008 JRR Added configuration macro by which to change what "endl" is
 *    \li 07-05-2008 JRR Added 'ascii' and 'numeric' format codes
 *    \li 11-24-2009 JRR Changed operation of 'clrscr' to a function to work with LCD
 *    \li 11-26-2009 JRR Integrated floating point support into this file
 *    \li 12-16-2009 JRR Improved support for constant strings in program memory
 *    \li 10-22-2012 JRR Fixed (OK, hacked around) bug which caused spurious warning 
 *                       for all Program Memory Strings
 *    \li 11-12-2012 JRR Made puts() non-virtual; made ENDL_STYLE() a function macro
 *    \li 12-21-2013 JRR Ported to ChibiOS
 *    \li 10-17-2014 JRR Made compatible with FreeRTOS for Cal Poly class use
 *
 *  License:
 *    This file released under the Lesser GNU Public License, version 2. This program
 *    is intended for educational use only, but it is not limited thereto. This code
 *    incorporates elements from Xmelkov's ftoa_engine.h, part of the avr-libc source,
 *    and users must accept and comply with the license of ftoa_engine.h as well. See
 *    emstream.h for a copy of the relevant license terms. */
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

#include "emstream.h"


//-------------------------------------------------------------------------------------
/** @brief   Temporary holder for @c setprecision() format function.
 *  @details This variable allows the nonmember function @c setprecision() to 
 *           communicate the number of digits after the decimal point to a specific 
 *           text serial port object. The @c setprecision() function returns a 
 *           manipulator that causes a specific serial object (the one in whose @c cout
 *           style output line it is called) to read the value of this variable. 
 */
uint8_t bts_new_prec = 3;


//-------------------------------------------------------------------------------------
/** @brief   Temporary holder for the @c setbase() format function.
 *  @details This variable allows the nonmember function @c setbase() to communicate a
 *           new numeric base to a specific serial device object. The @c setbase() 
 *           method returns a manipulator that causes a specific serial object (the one
 *           in whose @c cout style output line it is called) to read the value of this
 *           variable. 
 */
uint8_t bts_new_base = 10;


//-------------------------------------------------------------------------------------
/** @brief   Create a base text stream object.
 *  @details This constructor sets up the base serial port object. It sets the default
 *           base for the conversion of numbers to text and the default format for 
 *           converting chars. 
 */

emstream::emstream (void)
{
	base = 10;                              // Numbers are shown as decimal by default
	precision = 3;                          // Print 3 digits after a decimal point
	#ifdef __AVR
		pgm_string = false;                 // Print strings from SRAM by default
	#endif
}


//-------------------------------------------------------------------------------------
/** @brief   Check if a serial device is ready to transmit characters.
 *  @details This function checks if the serial port transmitter is ready to send data.
 *           It's an empty base method, overridden by most serial devices. Some serial
 *           devices might always be ready to send data; those need not bother to 
 *           override this method. 
 *  @return  True if the serial port is ready to send, and false if not
 */

bool emstream::ready_to_send (void)
{
	return (true);                          // By default the port's always ready
}


//-------------------------------------------------------------------------------------
/** @brief   Get a character from a serial device.
 *  @details This base method just returns zero, because it shouldn't be called. There
 *           might be classes which only send characters and don't ever receive them, 
 *           and this method could be left as a placeholder for those classes. 
 *  @return  A zero (null character) which means nothing
 */

char emstream::getchar (void)
{
	return ('\0');                          // Nothing to return, really
}


//-------------------------------------------------------------------------------------
/** @brief   Check if a character is ready to read from a serial device.
 *  @details This method checks if there is a character in the serial port's receiver 
 *           buffer. There isn't, as this base class isn't connected to a buffer; 
 *           descendent classes will override this method and check a real buffer for
 *           real characters. 
 *  @return  @c false, because no character will ever be available
 */

bool emstream::check_for_char (void)
{
	return (false);
}


//-------------------------------------------------------------------------------------
/** @brief   Cause a transmitter to send buffered characters immediately.
 *  @details This is a base method for causing immediate transmission of a buffer 
 *           holding some data. The base method doesn't do anything, because it can be
 *           used without overriding in descendent classes which have no buffers, send 
 *           everything immediately by default, and don't need to respond to calls for
 *           immediate transmission. 
 */

void emstream::transmit_now (void)
{
}


//-------------------------------------------------------------------------------------
/** @brief   Clear a display screen.
 *  @details This is a base method to clear a display screen, if there is one. It is 
 *           called when the format modifier @c clrscr is inserted in an output line.
 *           Descendant classes which do something other than writing a clear-screen 
 *           character to the device should override this method. For example, LCD 
 *           screens should operate the device's screen clearing sequence, while data 
 *           storage devices should do nothing in response to the @c clrscr modifier
 *           because there's no screen to clear. 
 */

void emstream::clear_screen (void)
{
	putchar (CLRSCR_STYLE);
}


//-------------------------------------------------------------------------------------
/** @brief   Write a character string to a serial device.
 *  @details This method writes a string to the serial device. It goes through the 
 *           character buffer to which its parameter points, calling \c putchar() to 
 *           print each character until and end of string character (the null 
 *           character, \c '\0') is reached.
 *  @param   p_string A pointer to the string which is to be printed
 */

void emstream::puts (const char* p_string)
{
#ifdef __AVR
	char ch;                                // Temporary storage for a character

	// If the program-string variable is set, this string is to be found in program
	// memory rather than data memory
	if (pgm_string)
	{
		pgm_string = false;
		while ((ch = pgm_read_byte_near (p_string++)))
		{
			putchar (ch);
		}
	}
	// If the program-string variable is not set, the string is in RAM and printed
	// in the normal way
	else
#endif
	{
		while (*p_string)
		{
			putchar (*p_string++);
		}
	}
}


//-------------------------------------------------------------------------------------
/** @brief   Set the number of digits shown after a decimal point.
 *  @details This function sets the global precision value, then returns a manipulator 
 *           which causes a serial object to change its floating point precision (the 
 *           number of digits after the decimal point which will be printed). The new 
 *           precision is "sticky" in that its value persists until this function is 
 *           called again to change it again. 
 *  @param   digits A new value for the number of digits after the decimal point;
 *                  a maximum of 7 digits are allowed
 *  @return  The serial manipulator called "manip_set_precision"
 */

ser_manipulator setprecision (uint8_t digits)
{
	if (digits > 7)
	{
		digits = 7;
	}
	bts_new_prec = digits;

	return (manip_set_precision);
}


//-------------------------------------------------------------------------------------
/** @brief   Set the base for subsequent numeric conversions.
 *  @details This function, not a member of class emstream, sets the global numeric 
 *           base and then returns a manipulator which causes a serial object to change
 *           to that base. The new base is "sticky" in that its value persists until 
 *           this function is called (or the manipulators @c bin, @c oct, @c dec or
 *           @c hex are used) to change it again. Setting the base automatically turns
 *           off raw binary printing mode. 
 *  @param   new_base A new value for the numeric base, as long as it's from 2 to 16
 *  @return  The serial manipulator called @c manip_set_base
 */

ser_manipulator setbase (uint8_t new_base)
{
	if ((new_base >= 2) && (new_base <= 16))
	{
		bts_new_base = new_base;
		return (manip_set_base);
	}

	return (manip_set_base);
}


//-------------------------------------------------------------------------------------
/** @brief   Overloaded operator used to print things to a serial device.
 *  @details This overload allows manipulators to be used to change the base of 
 *           displayed numbers to binary, octal, decimal, or hexadecimal, to print
 *           special codes such as end-of-line, and to cause immediate transmission by
 *           devices which save stuff to be transmitted in buffers. 
 *  @param   new_manip The serial manipulator which was given
 *  @return  A reference to the serial device to which the data was printed. This
 *           reference is used to string printable items together with @c << operators
 */

emstream& emstream::operator<< (ser_manipulator new_manip)
{
	switch (new_manip)
	{
		case (bin):                         // Print integers in binary
			base = 2;
			break;
		case (oct):                         // Print integers in octal
			base = 8;
			break;
		case (dec):                         // Print integers in decimal
			base = 10;
			break;
		case (hex):                         // Print integers in hexadecimal
			base = 16;
			break;
		case (endl):                        // Send an end-of-line
			ENDL_STYLE ();
			break;
		case (clrscr):                      // Send a clear-screen code
			clear_screen ();
			break;
		case (send_now):                    // Send whatever's in the send buffer
			transmit_now ();
			break;
		#ifdef __AVR
			case (_p_str):                  // The next string is in program memory
				pgm_string = true;
				break;
		#endif
		case (manip_set_precision):         // Set precision for printing floats
			precision = bts_new_prec;
			break;
		case (manip_set_base):              // Set numeric base to a number 2 to 16
			base = bts_new_base;
			break;
		default:                            // Not recognized?  Do nothing then
			break;
	};

	return (*this);
}

