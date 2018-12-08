//*************************************************************************************
/** \file base232.h
 *    This file contains a set of macros which are used by several classes and class
 *    templates that interact with the asynchronous (RS-232 style) serial port on an 
 *    AVR microcontroller. Classes which use this stuff include rs232 and packet232. 
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
 *    \li 12-14-2009 JRR Changed CPU_FREQ_Hz to F_CPU to be compatible with avr-libc
 *
 *  License:
 *    This file is released under the Lesser GNU Public License, version 2. This 
 *    program is intended for educational use only, but it is not limited thereto. 
 */
//*************************************************************************************

/// This define prevents this .h file from being included more than once in a .cpp file
#ifndef _BASE232_H_
#define _BASE232_H_

// #include "emstream.h"				// Pull in the base class header file

// Check that the user has set the CPU frequency in the Makefile; if not, complain
#if defined (__AVR) && !defined (F_CPU)
	#error The macro F_CPU must be set in the Makefile
#endif

//-------------------------------------------------------------------------------------
/** The number of tries to wait for the transmitter buffer to become empty */
#define UART_TX_TOUT		20000

//-------------------------------------------------------------------------------------
/** If this macro is defined, the UART will run in double-speed mode. This is often
 *  a good idea, as it allows higher baud rates to be used with not so high CPU clock
 *  frequencies. 
 */
#define UART_DOUBLE_SPEED

//-------------------------------------------------------------------------------------
/** This macro computes a value for the baud rate divisor from the desired baud rate 
 *  and the CPU clock frequency. The CPU clock frequency should have been set in the 
 *  macro F_CPU, which is normally configured in the Makefile. The divisor is
 *  calculated as (frequency, MHz / (16 * baudrate)), unless the USART is running in
 *  double-speed mode, in which case the baud rate divisor is twice as big. 
 */

#ifdef UART_DOUBLE_SPEED
	#define calc_baud_div(baud_rate) ((F_CPU) / (16UL * (baud_rate)))
#else
	#define calc_baud_div(baud_rate) ((F_CPU) / (8UL * (baud_rate)))
#endif


//-------------------------------------------------------------------------------------
/** \brief This is a base class for classes that use an RS-232 port on an AVR
 *  microcontroller. It is not designed to be used by itself; descendents such as 
 *  class \c rs232 should be used. 
 * 
 *  This class provides a basic hardware interface for an RS-232 port (also called a
 *  UART or USART in the AVR datasheets). It sets up the port, checks for characters, 
 *  and sends characters one at a time. Its descendent classes are the ones that do 
 *  useful things such as sending text messages and packets. 
 * 
 *  Code is present in this class to allow use in a Linux PC if \c __AVR is not
 *  defined. This may be helpful for testing and debugging RS-232 based code. 
 */

class base232
{
	protected:
	#ifdef __AVR
		/// This is a pointer to the data register used by the UART
		volatile unsigned char* p_UDR;

		/// This is a pointer to the status register used by the UART
		volatile unsigned char* p_USR;

		/// This is a pointer to the control register used by the UART
		volatile unsigned char* p_UCR;

		/// This bitmask identifies the bit for data register empty, UDRE
		unsigned char mask_UDRE;

		/// This bitmask identifies the bit for receive complete, RXC
		unsigned char mask_RXC;

		/// This bitmask identifies the bit for transmission complete, TXC
		unsigned char mask_TXC;
	#else
		/// This is the file handle for the serial port file device on a PC
		int serial_file;
	#endif // __AVR

	public:
	#ifdef __AVR
		/// The constructor sets up the port with the given baud rate and port number.
		base232 (unsigned int = 9600, unsigned char = 0);
	#else
		/// The constructor sets up the port with the given name.
		base232 (char*);
	#endif

		/// This method checks if the serial port is ready to transmit data.
		bool ready_to_send (void);

		/// This method returns true if the port is currently sending a character out.
		bool is_sending (void);
};

#endif  // _BASE232_H_
