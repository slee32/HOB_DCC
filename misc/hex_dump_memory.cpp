//*************************************************************************************
/** \file hex_dump_memory.cpp
 *    This file contains a function which prints a hex(adecimal) dump of a section of
 *    the processor's memory to a serial device which is a descendent of the 
 *    \c emstream class. The hex dump is handy for viewing a FreeRTOS task's
 *    stack to see if the available stack space has been used up. 
 *
 *  Revisions
 *    \li 04-12-2008 JRR Original file, material from source above
 *    \li 09-30-2012 JRR Added code to make memory allocation work with FreeRTOS
 *    \li 12-22-2013 JRR Didn't change anything to make it work with ChibiOS
 *    \li 07-02-2014 JRR Added \c size_type macros to make it work with STM32's
 *    \li 12-29-2014 JRR Made empty-memory code A5 print blank in ASCII field
 *
 *  License:
 *    This file is copyright 2012 by JR Ridgely and released under the Lesser GNU 
 *    Public License, version 2. It intended for educational use only, but its use
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

#include "mechutil.h"


#define DUMP_BYTES_PER_LINE 20              ///< The number of bytes per hex-dump line

//-------------------------------------------------------------------------------------
/** This function displays a bunch of binary data from the SRAM of an AVR processor 
 *  as a hexadecimal data dump. 
 *  @param start_address The memory address at which to begin the dump
 *  @param end_address The address which is one byte past the last one to be dumped
 *  @param p_ser_dev Pointer to the serial device on which data will be shown
 */

void hex_dump_memory (uint8_t* start_address, uint8_t* end_address,
					  emstream* p_ser_dev)
{
	uint8_t byte_count;                     // Counts through the bytes in a line
	uint8_t temp_byte;                      // Holds a byte temporarily and obviously

	// Print DUMP_BYTES_PER_LINE bytes on each line
	*p_ser_dev << hex;
	while (start_address < end_address)
	{
		// Print the line address 
		*p_ser_dev << (size_t)(start_address) << PMS ("  ");

		// Show a line full of data in hexadecimal format
		for (byte_count = 0; byte_count < DUMP_BYTES_PER_LINE; byte_count++)
		{
			*p_ser_dev << (uint8_t)(*start_address++);
			p_ser_dev->putchar (' ');
		}

		// Show the same line full of data in text format
		p_ser_dev->putchar (' ');
		for (start_address -= DUMP_BYTES_PER_LINE; byte_count > 0; byte_count--)
		{
			temp_byte = *start_address++;
			if (temp_byte == 0xA5)
			{
				p_ser_dev->putchar (' ');
			}
			else if (temp_byte >= ' ' && temp_byte <= '~')
			{
				p_ser_dev->putchar (temp_byte);
			}
			else
			{
				p_ser_dev->putchar ('.');
			}
		}

		// Put a return at the end of the line
		*p_ser_dev << endl;
	}
	*p_ser_dev << dec;
}

