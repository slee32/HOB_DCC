//**************************************************************************************
/** \file time_stamp_to_now_ISR.cpp
 *    This file contains a method belonging to class \c time_stamp in which a time
 *    stamp sets itself to the current time as measured by FreeRTOS timers. This method
 *    must \b only be called from within an Interrupt Service Routine.
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
/** This method saves the current time as measured by the RTOS in the time stamp. It
 *  is designed to be called from within an ISR, as it uses xTaskGetTickCountFromISR()
 *  instead of the non-interrupt version of that method. 
 */

void time_stamp::set_to_now_in_ISR (void)
{
	// Since we're inside an interrupt service routine, we don't need to disable 
	// interrupts -- by default they're disabled anyway. 

	// Now grab the hardware timer count. The tick count can't be updated, even if the
	// hardware timer overflows, because interrupts are disabled
	#ifdef TIMER3_COMPA_vect
		hardware_count = TCNT3;
	#else
		hardware_count = TCNT1;
	#endif

	// Now get the tick count (interrupts are still disabled)
	tick_count = xTaskGetTickCountFromISR ();
}

