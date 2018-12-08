//**************************************************************************************
/** \file time_stamp_greater.cpp
 *    This file contains an operator belonging to class \c time_stamp which tests if a
 *    time stamp is greater than another one. 
 *
 *  Revisions:
 *    \li 12-02-2012 JRR Split off from time_stamp.cpp to save memory in machine file
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
//**************************************************************************************

#include <stdlib.h>                         // Used for itoa()
#include <string.h>                         // Header for character string functions
#include <avr/interrupt.h>                  // For using interrupt service routines

#include "FreeRTOS.h"                       // Main header for FreeRTOS 
#include "task.h"                           // The FreeRTOS task functions header
#include "emstream.h"                       // Base for text-type serial port objects
#include "time_stamp.h"                     // Header for this file


//-------------------------------------------------------------------------------------
/** This operator tests if a time stamp is greater than another one. 
 *  @param other A time stamp to be compared to this one 
 *  @return True if this time stamp is greater than the other one
 */

bool time_stamp::operator > (const time_stamp& other)
{
	// First compare the RTOS tick counts; if they're different, we know the answer
	if (tick_count > other.tick_count)
	{
		return (true);
	}
	if (tick_count < other.tick_count)
	{
		return (false);
	}

	// If we get here, the RTOS tick counts were the same; compare hardware counts
	if (hardware_count > other.hardware_count)
	{
		return (true);
	}
	else
	{
		return (false);
	}
}

