//*************************************************************************************
/** \file rs232int.cpp
 *    This file contains a class which allows the use of a serial port on an AVR 
 *    microcontroller. This version of the class uses the serial port receiver 
 *    interrupt and a buffer to allow characters to be received in the background.
 *    The port is used in "text mode"; that is, the information which is sent and 
 *    received is expected to be plain ASCII text, and the set of overloaded left-shift 
 *    operators "<<" in emstream.* can be used to easily send all sorts of data 
 *    to the serial port in a manner similar to iostreams (like "cout") in regular C++. 
 *
 *  Revised:
 *    \li 04-03-2006 JRR For updated version of compiler
 *    \li 06-10-2006 JRR Ported from C++ to C for use with some C-only projects; also
 *        serial_avr.h now has defines for compatibility among lots of AVR variants
 *    \li 08-11-2006 JRR Some bug fixes
 *    \li 03-02-2007 JRR Ported back to C++. I've had it with the limitations of C.
 *    \li 04-16-2007 JO  Added write (unsigned long)
 *    \li 07-19-2007 JRR Changed some character return values to bool, added m324p
 *    \li 01-12-2008 JRR Added code for the ATmega128 using USART number 1 only
 *    \li 02-14-2008 JRR Split between emstream and rs232 files
 *    \li 05-31-2008 JRR Changed baud calculations to use CPU_FREQ_MHz from Makefile
 *    \li 06-01-2008 JRR Added getch_tout() because it's needed by 9Xstream modems
 *    \li 07-05-2008 JRR Changed from 1 to 2 stop bits to placate finicky receivers
 *    \li 12-22-2008 JRR Split off stuff in base232.h for efficiency
 *    \li 06-30-2009 JRR Received data interrupt and buffer added
 *
 *  License:
 *		This file is copyright 2012 by JR Ridgely and released under the Lesser GNU 
 *		Public License, version 2. It intended for educational use only, but its use
 *		is not limited thereto. */
/*		THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 *		AND	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * 		IMPLIED 	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * 		ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
 * 		LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUEN-
 * 		TIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
 * 		OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
 * 		CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
 * 		OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * 		OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */
//*************************************************************************************

#include <stdint.h>
#include <stdlib.h>
#include <avr/io.h>
#include "rs232int.h"


// Every AVR has at least one serial port, so enable at least one receiver buffer
/// This buffer holds characters received through serial port 0 by the ISR. 
uint8_t* rcv0_buffer = NULL;

/// This index is used to write into serial character receiver buffer 0.
uint16_t rcv0_read_index;

/// This index is used to read from serial character receiver buffer 0. 
uint16_t rcv0_write_index;

// If there's a UCSR0A register, there are 2 serial ports, so enable another buffer
#ifdef UCSR1A
	/// This buffer holds characters received through serial port 1 by the ISR. 
	uint8_t* rcv1_buffer = NULL;

	/// This index is used to write into serial character receiver buffer 1.
	uint16_t rcv1_read_index;

	/// This index is used to read from serial character receiver buffer 1. 
	uint16_t rcv1_write_index;
#endif


//-------------------------------------------------------------------------------------
/** This method sets up the AVR UART for communications.  It calls the emstream
 *  constructor, which prepares to convert numbers to text strings, and the base232 
 *  constructor, which does the work of setting up the serial port. Note that the user
 *  program must call sei() somewhere to enable global interrupts so that this driver
 *  will work. It is not called in this constructor because it's common to construct 
 *  many drivers which use interrupts, including this one, and then enable interrupts
 *  globally using sei() after all the constructors have been called. 
 *  @param baud_rate The desired baud rate for serial communications. Default is 9600
 *  @param port_number The number of the serial port, 0 or 1 (the second port numbered
 *                     1 only exists on some processors). The default is port 0 
 */

rs232::rs232 (uint16_t baud_rate, uint8_t port_number)
	: emstream (), base232 (baud_rate, port_number)
{
	// Save the number of the serial port, 0 or 1
	port_num = port_number;

	// If we're compiling for a chip with UCSR0A defined, it has dual serial ports
	// (examples are ATmega324P and ATmega128). Set up Port 0 or Port 1
	#if defined UCSR0A // Serial port number 0
		if (port_number == 0)
		{
			UCSR0B |= (1 << RXCIE0);		// Receive complete interrupt enable

			// Allocate some memory for the receiver buffer and reset the indices
			rcv0_buffer = new uint8_t[RSINT_BUF_SIZE];
			rcv0_read_index = 0;
			rcv0_write_index = 0;
		}
		else  // Serial port number 1
		{
		#if defined UCSR1A
			UCSR1B |= (1 << RXCIE1);		// Receive complete interrupt enable

			// Allocate some memory for the receiver buffer and reset the indices
			rcv1_buffer = new uint8_t[RSINT_BUF_SIZE];
			rcv1_read_index = 0;
			rcv1_write_index = 0;
		#endif // UCSR1A
		}
	// We're compiling for a chip which doesn't define UCSR0A; assume it has only one
	// serial port.
	#else
		UCSRB |= (1 << RXCIE);				// Receive complete interrupt enable

		// Allocate some memory for the receiver buffer and reset the indices
		rcv0_buffer = new uint8_t[RSINT_BUF_SIZE];
		rcv0_read_index = 0;
		rcv0_write_index = 0;
	#endif

	// The Xiphos 1.0 board may need the pullup activated on the RXD1 line in order to
	// use the XBee radio module
	#ifdef XIPHOS_HACKS
		if (port_number == 1)
			PORTD |= 0x04;
	#endif
}


//-------------------------------------------------------------------------------------
/** This method sends one character to the serial port.  It waits until the port is
 *  ready, so it can hold up the system for a while.  It times out if it waits too 
 *  long to send the character; you can check the return value to see if the character
 *  was successfully sent, or just cross your fingers and ignore the return value.
 *  Note 1:  It's possible that at slower baud rates and/or higher processor speeds, 
 *  this routine might time out even when the port is working fine.  A solution would
 *  be to change the count variable to an integer and use a larger starting number. 
 *  Note 2:  Fixed!  The count is now an integer and it works at lower baud rates.
 *  @param chout The character to be sent out
 *  @return True if everything was OK and false if there was a timeout
 */

void rs232::putchar (char chout)
{
	// Now wait for the serial port transmitter buffer to be empty	 
	for (uint16_t count = 0; ((*p_USR & mask_UDRE) == 0); count++)
	{
		if (count > UART_TX_TOUT)
		{
			return;
		}
	}

	// Clear the TXCn bit so it can be used to check if the serial port is busy.  This
	// check needs to be done prior to putting the processor into sleep mode.  Oddly,
	// the TXCn bit is cleared by writing a one to its bit location
	*p_USR |= mask_TXC;

	// The CTS line is 0 and the transmitter buffer is empty, so send the character
	*p_UDR = chout;
}


//-------------------------------------------------------------------------------------
/** This method gets one character from the serial port, if one is there.  If not, it
 *  waits until there is a character available.  This can sometimes take a long time
 *  (even forever), so use this function carefully.  One should almost always use
 *  check_for_char() to ensure that there's data available first. 
 *  @return The character which was found in the serial port receive buffer
 */

char rs232::getchar (void)
{
	uint8_t recv_char;						// Character read from the queue

	// Wait until there's a character in the receiver queue
	#ifdef UCSR0A  // If this is a dual-port chip
		if (port_num == 0)
		{
			while (rcv0_read_index == rcv0_write_index);
			recv_char = rcv0_buffer[rcv0_read_index];
			if (++rcv0_read_index >= RSINT_BUF_SIZE)
				rcv0_read_index = 0;
		}
		else  // This is port 1 of a dual-port chip
		{
		#if defined UCSR1A
			while (rcv1_read_index == rcv1_write_index);
			recv_char = rcv1_buffer[rcv1_read_index];
			if (++rcv1_read_index >= RSINT_BUF_SIZE)
				rcv1_read_index = 0;
		#endif // UCSR1A
		}
	#else  // This chip has only one serial port
		while (rcv0_read_index == rcv0_write_index);
		recv_char = rcv0_buffer[rcv0_read_index];
		if (++rcv0_read_index >= RSINT_BUF_SIZE)
			rcv0_read_index = 0;
	#endif

	return (recv_char);
}


//-------------------------------------------------------------------------------------
/** This method checks if there is a character in the serial port's receiver queue.
 *  The queue will have been filled if a character came in through the serial port and
 *  caused an interrupt. 
 *  @return True for character available, false for no character available
 */

bool rs232::check_for_char (void)
{
	#ifdef UCSR1A							// If this is a dual-port chip
		if (port_num == 0)
			return (rcv0_read_index != rcv0_write_index);
		else
			return (rcv1_read_index != rcv1_write_index);
	#else									// This chip has only one serial port
		return (rcv0_read_index != rcv0_write_index);
	#endif
}


//-------------------------------------------------------------------------------------
/** This method sends the ASCII code to clear a display screen. It is called when the
 *  format modifier 'clrscr' is inserted in a line of "<<" stuff.
 */

void rs232::clear_screen (void)
{
	putchar (CLRSCR_STYLE);
}


//-------------------------------------------------------------------------------------
/** \cond NOT_ENABLED  (This ISR is not to be documented by Doxygen)
 *  This interrupt service routine runs whenever a character has been received by the
 *  first serial port (number 0).  It saves that character into the receiver buffer.
 */

ISR (RSI_CHAR_RECV_INT_0)
{
	// When this ISR is triggered, there's a character waiting in the USART data reg-
	// ister, and the write index indexes the place where that character should go

	#if defined UCSR0A  // If this is a dual-serial-port chip (ATmega324P, 128, etc.)
		rcv0_buffer[rcv0_write_index] = UDR0;
	#else  // If this chip has only a single serial port (ATmega8, 32, etc.)
		rcv0_buffer[rcv0_write_index] = UDR;
	#endif

	// Increment the write pointer
	if (++rcv0_write_index >= RSINT_BUF_SIZE)
		rcv0_write_index = 0;

	// If the write pointer is now equal to the read pointer, that means we've just
	// overwritten the oldest data. Increment the read pointer so that it doesn't seem
	// as if the buffer is empty
	if (rcv0_write_index == rcv0_read_index)
		if (++rcv0_read_index >= RSINT_BUF_SIZE)
			rcv0_read_index = 0;
}


#ifdef UCSR1A // The second ISR is only compiled for processors with dual serial ports
	//-------------------------------------------------------------------------------------
	/** This interrupt service routine runs whenever a character has been received by the
	*  first serial port (number 0).  It saves that character into the receiver buffer.
	*/

	ISR (RSI_CHAR_RECV_INT_1)
	{
		// Read the character from the serial port receiver buffer
		rcv1_buffer[rcv1_write_index] = UDR1;

		// Increment the write pointer
		if (++rcv1_write_index >= RSINT_BUF_SIZE)
			rcv1_write_index = 0;

		// If the write pointer is now equal to the read pointer, that means we've just
		// overwritten the oldest data. Increment the read pointer so that it doesn't seem
		// as if the buffer is empty
		if (rcv1_write_index == rcv1_read_index)
			if (++rcv1_read_index >= RSINT_BUF_SIZE)
				rcv1_read_index = 0;
	}
#endif // Dual serial ports
/** \endcond  (End of section which is not to be documented by Doxygen) */
