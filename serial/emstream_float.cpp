//*************************************************************************************
/** \file emstream_float.cpp
 *    This file contains the operators which print floating-point numbers using the
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

#include <stdint.h>
#include <stdlib.h>
#include "emstream.h"


//-------------------------------------------------------------------------------------
/** This operator writes a single-precision floating point number to the serial port in
 *  exponential format (always with the 'e' notation). It calls the utility function 
 *  __ftoa_engine, which is hiding in the AVR libraries, used by the Xprintf() 
 *  functions when they need to convert a float into text. 
 *  @return A reference to the serial device to which the data was printed. This
 *          reference is used to string printable items together with "<<" operators
 *  @param num The floating point number to be sent out
 */

emstream& emstream::operator<< (float num)
{
	uint8_t digit = precision;
	char buf[20];
	char* p_buf = buf;

	int exponent = __ftoa_engine ((double)num, buf, digit, 16);
	uint8_t vtype = *p_buf++;
	if (vtype & FTOA_NAN)
	{
		*this << "NaN";
		return (*this);
	}

	// Display the sign if it's negative
	if (vtype & FTOA_MINUS)
	{
		putchar ('-');
	}

	// Show the mantissa
	putchar (*p_buf++);
	if (digit)
	{
		putchar ('.');
	}
	while ((digit-- > 0) && *p_buf)
	{
		putchar (*p_buf++);
	}

	// Now display the exponent
	putchar ('E');
	if (exponent > 0)
	{
		putchar ('+');
	}
	*this << exponent;
	return (*this);
}


//-------------------------------------------------------------------------------------
/** This operator writes a double-precision floating point number to the serial port in
 *  exponential format (always with the 'e' notation). It calls the utility function 
 *  @c __ftoa_engine(), which is hiding in the AVR libraries, used by the @c Xprintf() 
 *  functions when they need to convert a double into text. 
 *  @return A reference to the serial device to which the data was printed. This
 *          reference is used to string printable items together with "<<" operators
 *  @param num The double-precision floating point number to be sent out
 */

emstream& emstream::operator<< (double num)
{
	uint8_t digit = precision;
	char buf[20];
	char* p_buf = buf;

	int exponent = __ftoa_engine (num, buf, digit, 16);
	uint8_t vtype = *p_buf++;
	if (vtype & FTOA_NAN)
	{
		*this << "  NaN";
		return (*this);
	}

	// Display the sign if it's negative
	if (vtype & FTOA_MINUS)
		putchar ('-');

	// Show the mantissa
	putchar (*p_buf++);
	if (digit)
		putchar ('.');
	do
		putchar (*p_buf++);
	while (--digit && *p_buf);

	// Now display the exponent
	putchar ('e');
	if (exponent > 0)
		putchar ('+');
	*this << exponent;
	return (*this);
}
