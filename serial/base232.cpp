//*************************************************************************************
/** \file base232.cpp
 *    This file contains a class which operates an asynchronous (RS-232 style) serial 
 *    port on an AVR microcontroller. Classes which use this stuff include rs232 and 
 *    packet232. 
 *
 *  Revisions:
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
 *    \li 01-30-2009 JRR Added class with port setup in constructor
 *    \li 06-02-2009 JRR Changed baud rate divisor formula to work better
 *
 *  License:
 *    This file is released under the Lesser GNU Public License, version 2. This 
 *    program is intended for educational use only, but it is not limited thereto. 
 */
//*************************************************************************************

#ifdef __AVR
	#include <avr/io.h>						// Definitions of AVR's I/O registers
#else
	#include <stdlib.h>						// Standard stuff such as exit()
	#include <fcntl.h>						// File devices on Linux computers
	#include <iostream>						// Headers for I/O streams so we can use
	#include <iomanip>						// 'cout << stuff' style printing
	using namespace std;					// Use file I/O from standard library
#endif

#include "base232.h"


//-------------------------------------------------------------------------------------
/** This method sets up the AVR UART for communications.  It enables the appropriate 
 *  inputs and outputs and sets the baud rate divisor, and it saves pointers to the
 *  registers which are used to operate the serial port. Since some AVR processors
 *  have dual serial ports, this method allows one to specify a port number. 
 *  @param baud_rate The desired baud rate for serial communications. Default is 9600
 *  @param port_number The number of the serial port, 0 or 1 (the second port numbered
 *                     1 only exists on some processors). The default is port 0 
 */

// This section compiles for the AVR microcontroller
#ifdef __AVR
base232::base232 (unsigned int baud_rate, unsigned char port_number)
{
	// If we're compiling for a chip with UCSR0A defined, it has dual serial ports
	// (examples are ATmega324P and ATmega128). Set up Port 0 or Port 1
	#if defined UCSR0A
		if (port_number == 0)
		{
			p_UDR = &UDR0;
			p_USR = &UCSR0A;
			p_UCR = &UCSR0B;
			UCSR0B = (1 << RXEN0) | (1 << TXEN0);
			UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // | (1 << USBS0);
			UBRR0H = 0x00;
			UBRR0L = calc_baud_div (baud_rate);
			#ifdef UART_DOUBLE_SPEED					// Activate double speed mode
				UCSR0A |= U2X0;							// if required
			#endif
			mask_UDRE = (1 << UDRE0);
			mask_RXC = (1 << RXC0);
			mask_TXC = (1 << TXC0);
		}
		else	// The port number isn't 0, so it presumably must be serial port 1
		{
		#if defined UCSR1A
			p_UDR = &UDR1;
			p_USR = &UCSR1A;
			p_UCR = &UCSR1B;
			UCSR1B = (1 << RXEN1) | (1 << TXEN1);
			UCSR1C = (1 << UCSZ11) | (1 << UCSZ10); // | (1 << USBS1);
			UBRR1H = 0x00;
			UBRR1L = calc_baud_div (baud_rate);
			#ifdef UART_DOUBLE_SPEED		// If double-speed macro has been defined,
				UCSR1A |= U2X1;				// turn on double-speed operation
			#endif
			mask_UDRE = (1 << UDRE1);
			mask_RXC = (1 << RXC1);
			mask_TXC = (1 << TXC1);
		#endif // UCSR1A
		}
	// We're compiling for a chip which has no USCR0A, so it should have only one
	// serial port. Ignore the port number and set up the only serial port available
	#else // no UCSR0A
		// Chips such as ATmega8 and ATmega32, have UCSRA and UCSRB and UCSRC registers 
		// which are set up appropriately
		#ifdef UCSRA
			p_UDR = &UDR;
			p_USR = &UCSRA;
			p_UCR = &UCSRB;
			UCSRB = (1 << RXEN) | (1 << TXEN);
			UCSRC = (1 << URSEL) | (1 << UCSZ1) | (1 << UCSZ0);		// | (1 << USBS0);
			UBRRH = 0x00;
			UBRRL = calc_baud_div (baud_rate);
			#ifdef UART_DOUBLE_SPEED				// Activate double speed mode
				UCSRA |= U2X;						// if required
			#endif
			mask_UDRE = (1 << UDRE);
			mask_RXC = (1 << RXC);
			mask_TXC = (1 << TXC);
		// Older, simpler chips such as AT90S2313 have only the UCR register
		#else // no UCSRA
			p_UDR = &UDR;
			p_USR = &USR;
			p_UCR = &UCR;
			UCR = (1 << RXEN) | (1 << TXEN);		// 0x18 for mode N81
			UBRR = calc_baud_div (baud_rate);
			mask_UDRE = (1 << UDRE);
			mask_RXC = (1 << RXC);
			mask_TXC = (1 << TXC);
		#endif // UCSRA
	#endif // UCSR0A

	// Read the data register to ensure that it's empty
	port_number = *p_UDR;
	port_number = *p_UDR;
}
#else // If not AVR, we must be compiling for a Linux PC
base232::base232 (char* port_name)
{
	serial_file = open (port_name, O_RDWR | O_NOCTTY | O_NDELAY);
	if (serial_file <= 0)
	{
		cerr << "ERROR: Can't open serial port \"" << port_name << "\"" << endl;
		exit (-1);
	}
	else
	{
		cout << "Opened serial port " << port_name << " as device " << serial_file
			 << endl;
	}
}
#endif // __AVR


//-------------------------------------------------------------------------------------
/** This method checks if the serial port transmitter is ready to send data.  It 
 *  tests whether transmitter buffer is empty. 
 *  @return True if the serial port is ready to send, and false if not
 */

bool base232::ready_to_send (void)
{
#ifdef __AVR
	// If transmitter buffer is full, we're not ready to send
	if (*p_USR & mask_UDRE)
		return (true);

	return (false);
#else
	// The non-AVR (that is, PC) serial port buffer should always be ready
	return (true);
#endif
}


//-------------------------------------------------------------------------------------
/** This method checks if the serial port is currently sending a character. Even if the
 *  buffer is ready to accept a new character, the port might still be busy sending the
 *  last one; it would be a bad idea to put the processor to sleep before the character
 *  has been sent. 
 *  @return True if the port is currently sending a character, false if it's idle
 */

bool base232::is_sending (void)
{
#ifdef __AVR
	if (*p_USR & mask_TXC)
		return (false);
	else
		return (true);
#else
	// We don't really care if a PC is sending, as it has a buffer anyway
	return (false);
#endif
}
