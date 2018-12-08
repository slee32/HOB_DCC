//**************************************************************************************
/** \file time_stamp_minus_eq.cpp
 *    This file contains a \c -= operator belonging to class \c time_stamp which 
 *    subtracts time stamps. It is usually used to find an interval between two times.
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
/** This overloaded subtraction operator finds the duration between this time stamp's 
 *  recorded time and a previous one. The data in this timestamp is replaced with the 
 *  computed duration. 
 *  @param previous An earlier time stamp to be compared to the current one 
 */

void time_stamp::operator -= (const time_stamp& previous)
{
	tick_count -= previous.tick_count;
	hardware_count -= previous.hardware_count;

	// If the (unsigned) hardware count seems to be greater than the maximum possible
	// hardware count, it's actually a negative number, so borrow from the tick count
	if (hardware_count > TMR_MAX_CT)
	{
		tick_count--;
		hardware_count += TMR_MAX_CT;
	}
}

