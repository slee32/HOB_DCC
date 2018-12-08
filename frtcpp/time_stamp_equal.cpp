//**************************************************************************************
/** \file time_stamp_equal.cpp
 *    This file contains an \c == operator belonging to class \c time_stamp which tests
 *    if two time stamps are exactly the same. It has limited practical utility because
 *    in order for two time stamps to be equal, they must have been recorded at exactly
 *    the same time, down to one real-time counter tick. 
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


//--------------------------------------------------------------------------------------
/** This overloaded equality test operator checks if the time measurement in some other
 *  time stamp is exactly equal to those in this one. It's usually not especially
 *  useful to perform this test, because equality means that the time must be exactly
 *  the same down to the resolution of the hardware counter/timer. Usually one uses an
 *  inequality test or checks if the difference between two times is within some small
 *  tolerance. 
 *  @param other A time stamp to be compared to this one 
 *  @return True if the time stamps contain equal data, false if they don't
 */

bool time_stamp::operator == (const time_stamp& other)
{
	if ((other.hardware_count == hardware_count) && (other.tick_count == tick_count))
	{
		return (true);
	}
	else
	{
		return (false);
	}
}

