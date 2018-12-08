//**************************************************************************************
/** \file time_stamp_lshift.cpp
 *    This file contains an overloaded operator which prints out a \c time_stamp by
 *    showing the time represented by that time stamp. 
 *
 *  Revisions:
 *    \li 12-02-2012 JRR Split off from time_stamp.cpp to save memory in machine file
 *
 *  License:
 *      This file is copyright 2012 by JR Ridgely and released under the Lesser GNU 
 *      Public License, version 2. It intended for educational use only, but its use
 *      is not limited thereto. */
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
//**************************************************************************************

#include <stdlib.h>                         // Used for itoa()
#include <string.h>                         // Header for character string functions
#include <avr/interrupt.h>                  // For using interrupt service routines

#include "FreeRTOS.h"                       // Main header for FreeRTOS 
#include "task.h"                           // The FreeRTOS task functions header
#include "emstream.h"                       // Base for text-type serial port objects
#include "time_stamp.h"                     // Header for this file


//-------------------------------------------------------------------------------------
/** This overloaded operator allows a time stamp to be printed on a serial device such
 *  as a regular serial port or radio module in text mode. This allows lines to be set
 *  up in the style of \c cout. The timestamp is always printed as a decimal number. 
 *  @param serial A reference to the serial-type object to which to print
 *  @param stamp A reference to the time stamp to be displayed
 *  @return A reference to the serial device to which the data was printed. This
 *          reference is used to string printable items together with "<<" operators
 */

emstream& operator<< (emstream& serial, time_stamp& stamp)
{
	char dig_buffer[7];						// Holds digits we compute
	ldiv_t div_result;						// Holds results of long integer division

	// First write the seconds in the time stamp, then a decimal
	serial << stamp.get_seconds ();
	serial.putchar ('.');

	// Now get the microseconds; these will be written with leading zeros as needed
	div_result.quot = stamp.get_microsec ();
	for (int8_t index = 5; index >= 0; index--)
	{
		div_result = ldiv (div_result.quot, 10);
		dig_buffer[index] = div_result.rem + '0';
	}

	// The last item in the string must be a '\0' null character
	dig_buffer[6] = '\0';

	// Write the buffer, which now contains microsecond digits
	serial << dig_buffer;

	return (serial);
}

