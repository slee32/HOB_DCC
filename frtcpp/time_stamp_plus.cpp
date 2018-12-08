//**************************************************************************************
/** \file time_stamp_plus.cpp
 *    This file contains an operator belonging to class \c time_stamp which adds two
 *    time stamps together. 
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
/** This overloaded addition operator adds another time stamp's time to this one.  It 
 *  can be used to find the time in the future at which some event is to be caused to 
 *  happen, such as the next time a task is supposed to run. 
 *  @param addend The other time stamp which is to be added to this one
 *  @return The newly created time stamp
 */

time_stamp time_stamp::operator + (const time_stamp& addend)
{
	time_stamp ret_stamp;

	ret_stamp.hardware_count = this->hardware_count + addend.hardware_count;
	ret_stamp.tick_count = this->tick_count + addend.tick_count;

	// If the hardware count would cause an overflow, carry the overflow into the
	// RTOS tick count
	if (ret_stamp.hardware_count >= TMR_MAX_CT)
	{
		ret_stamp.hardware_count -= TMR_MAX_CT;
		ret_stamp.tick_count++;
	}

	return ret_stamp;
}

