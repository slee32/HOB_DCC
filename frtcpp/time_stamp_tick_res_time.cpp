//**************************************************************************************
/** \file time_stamp_tick_res_time.cpp
 *    This file contains a function which computes time quickly using only the count of 
 *    RTOS tick overflows. It's not a particularly precise measurement of time, but 
 *    it's good enough for some uses. 
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


//--------------------------------------------------------------------------------------
/** This function computes time quickly using only RTOS timer ticks and makes a 
 *  printable string. It ignores the hardware counter. The string contains the time in 
 *  seconds and (roughly) milliseconds. This is a regular C function because it doesn't 
 *  need anything from a task class; it just uses what's built into FreeRTOS already. 
 *  @return A pointer to the string containing the current time
 */

const char* tick_res_time (void)
{
	static char cur_time_str[16];           // Holds string showing the time
	ldiv_t div_result;                      // Holds results of long integer division
	TickType_t tick_count;                  // Count of RTOS ticks which measure time

	// Disable interrupts, get the tick count, and re-enable interrupts quickly
	portENTER_CRITICAL ();
	tick_count = xTaskGetTickCount ();
	portEXIT_CRITICAL ();

	// Compute and print the integer part of the number of seconds
	// char* ultoa (unsigned long int __val, char *__s, int __radix)
	div_result = ldiv ((uint32_t)tick_count, configTICK_RATE_HZ);
	ultoa (div_result.quot, cur_time_str, 10);

	// Set a pointer to the end of the integer part and put a '.' there
	char* pstr = cur_time_str;
	while (*pstr)
	{
		pstr++;                             // Advance to the null character
	}
	*pstr++ = '.';

	// Multiply the remainder by 1000; this gives us milliseconds in the time
	int32_t millisec = (div_result.rem * 1000) / configTICK_RATE_HZ;

	// Divide by 10 a lot, using ldiv to keep the remainder handy
	div_result = ldiv (millisec, 10);
	*(pstr + 2) = div_result.rem + '0';
	div_result = ldiv (div_result.quot, 10);
	*(pstr + 1) = div_result.rem + '0';
	*(pstr + 0) = div_result.quot + '0';

	// Don't forget the null terminator at the end of the string
	*(pstr + 3) = '\0';

	// Always return a pointer to the time string
	return (cur_time_str);
}
