//*************************************************************************************
/** \file emstream_uint8_t.cpp
 *    This file contains an operator which prints an 8-bit unsigned number using the
 *    \c emstream class. 
 *
 *  Revised:
 *    \li 12-02-2012 JRR Split this file off from the main \c emstream.cpp to
 *                       allow smaller machine code if stuff in this file isn't used
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

#include <stdlib.h>
#include "emstream.h"


//-------------------------------------------------------------------------------------
/** This operator writes a character to the serial port as a text string showing the 
 *  8-bit unsigned number in that character, unless the format control term \c ascii 
 *  was called prior to this printing, in which case the character will be printed as 
 *  an ASCII character.
 *  @return A reference to the serial device to which the data was printed. This
 *          reference is used to string printable items together with "<<" operators
 *  @param num The 8-bit number or character to be sent out
 */

emstream& emstream::operator<< (uint8_t num)
{
	uint8_t temp_char;                      // Temporary storage for a nibble

	if (print_ascii)
	{
		putchar (num);
	}
	else if (base == 2)
	{
		for (uint8_t bmask = 0x80; bmask != 0; bmask >>= 1)
		{
			if (num & bmask) putchar ('1');
			else             putchar ('0');
		}
	}
	else if (base == 16)
	{
		temp_char = (num >> 4) & 0x0F;
		putchar ((temp_char > 9) ? temp_char + ('A' - 10) : temp_char + '0');
		temp_char = num & 0x0F;
		putchar ((temp_char > 9) ? temp_char + ('A' - 10) : temp_char + '0');
	}
	else
	{
		char out_str[9];
		utoa ((uint16_t)num, out_str, base);
		puts (out_str);
	}
	return (*this);
}

