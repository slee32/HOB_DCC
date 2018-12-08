//*************************************************************************************
/** \file emstream_uint64_t.cpp
 *    This file contains an operator which prints uint64_t numbers using the
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
/** This operator writes a long long (64-bit) unsigned integer to the serial port as a 
 *  text string.  It only writes such numbers in unsigned hexadecimal format. The
 *  number is written by breaking it into two unsigned longs, writing them in order.
 *  @return A reference to the serial device to which the data was printed. This
 *          reference is used to string printable items together with "<<" operators
 *  @param num The 64-bit number to be sent out
 */

emstream& emstream::operator<< (uint64_t num)
{
	union {
		   uint64_t whole;
		   uint8_t bits[8];
		  } parts;
	parts.whole = num;
	*this << parts.bits[7] << parts.bits[6] << parts.bits[5] << parts.bits[4]
		  << parts.bits[3] << parts.bits[2] << parts.bits[1] << parts.bits[0];

	return (*this);
}

