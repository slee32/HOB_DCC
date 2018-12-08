//*************************************************************************************
/** \file time_stamp.h
 *    This file contains a class which runs a task timer whose resolution is about a
 *    microsecond. It uses the FreeRTOS tick timer as a coarse timer and queries the
 *    hardware timer (generally Timer 1 or Timer 3) to get a more accurate measurement
 *    of time. The RTOS tick count is then used as a overflow count for the hardware
 *    timer. In typical usage, RTOS ticks occur at around once per millisecond. 
 *
 *  Revisions:
 *    \li 08-07-2007 JRR Created this file as daytimer.* with 1 second interrupts
 *    \li 08-08-2007 JRR Added event triggers
 *    \li 12-23-2007 JRR Made more general by allowing faster interrupt rates
 *    \li 01-05-2008 JRR Converted from time-of-day version to microsecond version
 *    \li 03-27-2008 JRR Added operators + and - for time stamps
 *    \li 03-31-2008 JRR Merged in stl_us_timer (int, long) and set_time (int, long)
 *    \li 05-15-2008 JRR Changed to use Timer 3 so Timer 1 can run motor PWM's
 *    \li 05-31-2008 JRR Changed time calculations to use CPU_FREQ_MHz from Makefile
 *    \li 01-04-2009 JRR Now uses CPU_FREQ_Hz (rather than MHz) for better precision
 *    \li 11-24-2009 JRR Changed CPU_FREQ_Hz to F_CPU to match AVR-LibC's name
 *    \li 09-30-2012 JRR Transmogrified into a version that works with FreeRTOS
 *    \li 10-10-2012 JRR Made time_stamp::set_to_now() return a reference to the stamp
 *    \li 12-02-2012 JRR Split many methods and operators into their own \c .cpp files
 *                       in order to save memory in the compiled machine code
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

// This define prevents this .h file from being included more than once in a .cpp file
#ifndef _TIME_STAMP_H_
#define _TIME_STAMP_H_

#include "FreeRTOS.h"
#include "emstream.h"


//--------------------------------------------------------------------------------------
/** This define contains the type of variable needed to hold a time count. If the tick
 *  counter in FreeRTOS uses a 16-bit counter such as Timer 1 or Timer 3, it's a 
 *  uint16_t; if the tick counter uses an 8-bit timer such as Timer 0 or Timer 2, it's
 *  a uint8_t.
 */
#define HW_CTR_TYPE			uint16_t


/// This constant holds the number of hardware timer ticks per second.
const uint32_t HW_TICK_RATE_HZ = configCPU_CLOCK_HZ / portCLOCK_PRESCALER;

/** This constant holds the number of hardware timer ticks until a compare match. When
 *  a compare match occurs, the hardware timer clears itself and causes the interrupt 
 *  which invokes the RTOS scheduler. This number is therefore the number of hardware
 *  ticks per RTOS tick.
 */
const uint32_t TMR_MAX_CT = configCPU_CLOCK_HZ / 
						   (configTICK_RATE_HZ * portCLOCK_PRESCALER);


//--------------------------------------------------------------------------------------
/** \brief This class holds a time stamp which is used to measure the passage of real 
 *  time in the world of an AVR processor with approximately microsecond resolution. 
 * 
 *  This version of the time stamp uses a tick count from the FreeRTOS scheduler, 
 *  combined with a count from the hardware timer which is used by the tick counter, to 
 *  get a high-resolution measurement of time. 
 */

class time_stamp
{
	protected:
		/// This is the value of the FreeRTOS tick counter for the time stamp.
		TickType_t tick_count;

		/// This is the value which was in the hardware timer when time was stamped.
		HW_CTR_TYPE hardware_count;

	public:
		/** This constructor creates a time stamp with its time set to zero.
		 */
		time_stamp (void)
		{
			tick_count = 0;
			hardware_count = 0;
		}

		/** This constructor creates a time stamp object and fills the time stamp's 
		 *  variables with the given values.
		 *  @param rtos_ticks The number of RTOS ticks to be put in the time stamp
		 *  @param hw_ticks The number of hardware counter ticks to go in time stamp
		 */
		time_stamp (TickType_t rtos_ticks, HW_CTR_TYPE hw_ticks)
		{
			tick_count = rtos_ticks;
			hardware_count = hw_ticks;
		}

		/** This method returns the number of RTOS ticks in the time stamp.
		 *  @return The number of RTOS ticks since the scheduler was started
		 */
		TickType_t get_RTOS_ticks (void)
		{
			return tick_count;
		}

		/** This method returns the number of seconds in the time stamp. It is assumed
		 *  that the hardware counter is ticking at an integer number of ticks per 
		 *  second so that the hardware timer count does not need to be used in 
		 *  computing the number of seconds. 
		 *  @return The number of whole seconds in the time stamp
		 */
		uint32_t get_seconds (void)
		{
			return (tick_count / configTICK_RATE_HZ);
		}

		// This method returns the number of microseconds in the time stamp
		uint32_t get_microsec (void);

		// This method returns the time in the time stamp as a float
		float to_float (void);

		// This function gets the time from the RTOS scheduler into this time stamp
		time_stamp& set_to_now (void);

		// This function gets the time from the RTOS scheduler into this time stamp 
		// in a way that can be used within an Interrupt Service Routine
		void set_to_now_in_ISR (void);

		// This overloaded addition operator adds two time stamps together
		time_stamp operator + (const time_stamp&);

		// This overloaded subtraction operator finds the time between two time stamps
		time_stamp operator - (const time_stamp&);

		// This overloaded addition operator adds two time stamps together
		void operator += (const time_stamp&);

		// This overloaded subtraction operator finds the time between two time stamps
		void operator -= (const time_stamp&);

		// This overloaded equality operator tests if two time stamps are the same
		bool operator == (const time_stamp&);

		// This overloaded equality operator tests if two time stamps are not the same
		bool operator != (const time_stamp&);

		// This operator tests if a time stamp is greater (later) or equal to this one
		bool operator >= (const time_stamp&);

		// This overloaded operator tests if a time stamp is greater (later) than this
		bool operator > (const time_stamp&);

		// This operator tests if a time stamp is less (earlier) than this one
		bool operator <= (const time_stamp&);

		// This operator tests if a time stamp is less (earlier) or equal to this one
		bool operator < (const time_stamp&);
};


//--------------------------------------------------------------------------------------
// This operator allows a time stamp to be written to serial device 'cout' style
emstream& operator<< (emstream&, time_stamp&);

//--------------------------------------------------------------------------------------
// This function computes time quickly using only RTOS timer ticks and makes a string
const char* tick_res_time (void);

#endif  // _TIME_STAMP_H_
