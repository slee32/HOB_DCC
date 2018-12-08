//**************************************************************************************
/** \file task_multi.cpp
 *    This file contains the code for a task class which will be instantiated many
 *    times to test how many tasks can be safely run on various AVR processors using
 *    the ME507/FreeRTOS software. 
 *
 *  Revisions:
 *    \li 09-30-2012 JRR Original file was a one-file demonstration with two tasks
 *    \li 10-05-2012 JRR Split into multiple files, one for each task
 *    \li 10-25-2012 JRR Changed to a more fully C++ version with class task_sender
 *    \li 10-27-2012 JRR Altered from data sending task into LED blinking class
 *    \li 11-04-2012 JRR Altered again into the multi-task monstrosity
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

#include "textqueue.h"                      // Header for text queue class
#include "task_multi.h"                     // Header for this task
#include "shares.h"                         // Shared inter-task communications


//-------------------------------------------------------------------------------------
/** This constructor creates a generic do-nothing-useful task. The main job of this 
 *  constructor is to call its parent class's constructor which does most of the work.
 *  @param a_name A character string which will be the name of this task
 *  @param a_priority The priority at which this task will initially run (default: 0)
 *  @param a_stack_size The size of this task's stack in bytes 
 *                      (default: configMINIMAL_STACK_SIZE)
 *  @param p_ser_dev Pointer to a serial device (port, radio, SD card, etc.) which can
 *                   be used by this task to communicate (default: NULL)
 */

task_multi::task_multi (const char* a_name, 
						unsigned portBASE_TYPE a_priority, 
						size_t a_stack_size,
						emstream* p_ser_dev
					   )
	: TaskBase (a_name, a_priority, a_stack_size, p_ser_dev)
{
}


//-------------------------------------------------------------------------------------
/** This run method uses up a little processor time for testing purposes. There's a
 *  random delay time in case any race conditions cause something bad to happen. 
 */

void task_multi::run (void)
{
	volatile uint8_t temp_byte;             // Storage for meaningless data

	// Setup: seed the random number generator. It's a bit silly that each task_multi
	// task seeds the generator at startup (we're kind of over-planting) but harmless
	srand (TCNT1);

	// This is the task loop. Once the task has been initialized in the code just
	// above, the task loop runs, and it keeps running until the power is shut off
	for (;;)
	{
		// Read an I/O register. This just forces the task to do something that
		// uses a little processor time
		temp_byte = PIND;
		temp_byte++;                        // This shuts up a compiler warning

		// Every now and then, say that this task is OK. If the modulo number is
		// about 135, the task reports at about 10 second intervals
// 		if ((++runs % 135) == 0)
// 		{
// 			print_ser_queue << get_name () << PMS (":") << runs << endl;
// 		}
		++runs;

		delay ((rand () & 0x007F) + 10);    // Delay for 10 to 137 RTOS ticks
	}
}

