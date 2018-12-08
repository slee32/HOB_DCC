//**************************************************************************************
/** \file task_source.cpp
 *    This file contains the header for a task which sends test data to another task,
 *    the data sink task, using a queue, a shared data class object, and a dangerous
 *    global variable. The data will be checked to test the reliability of each method
 *    of conveying the information. 
 *
 *  Revisions:
 *    \li 09-30-2012 JRR Original file was a one-file demonstration with two tasks
 *    \li 10-05-2012 JRR Split into multiple files, one for each task
 *    \li 11-04-2012 JRR Changed into the test data source task
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


#include "task_user.h"						// Header for user interface task
#include "task_source.h"						// Header file for this task


//-------------------------------------------------------------------------------------
/** This constructor creates a new data source task. Its main job is to call the
 *  parent class's constructor which does most of the work.
 *  @param a_name A character string which will be the name of this task
 *  @param a_priority The priority at which this task will initially run (default: 0)
 *  @param a_stack_size The size of this task's stack in bytes 
 *                      (default: configMINIMAL_STACK_SIZE)
 *  @param p_ser_dev Pointer to a serial device (port, radio, SD card, etc.) which can
 *                   be used by this task to communicate (default: NULL)
 */

task_source::task_source (const char* a_name, 
						  unsigned portBASE_TYPE a_priority, 
						  size_t a_stack_size,
						  emstream* p_ser_dev
						 )
	: TaskBase (a_name, a_priority, a_stack_size, p_ser_dev)
{
}


//-------------------------------------------------------------------------------------
/** This task function takes data at regular intervals, then puts the data into a 
 *  queue to be received by another task. 
 */

void task_source::run (void)
{
	// Set up by declaring and initializing variables
	uint32_t some_data = 0;
	uint8_t toggler = false;

	// This is the task loop. Once the task has been initialized in the code just
	// above, the task loop runs, and it keeps running until the power is shut off
	for (;;)
	{
		// Create some data and send it to the sink task in the queue
		some_data = random ();
		p_queue_1->put (some_data & 0xFF00FF00);

		// Write standard values here; we'll check what is received by the sink
		if (toggler)
		{
			p_share_1->put (0x12345678);
			glob_of_probs = 0x12345678;
		}
		else
		{
			p_share_1->put (0x87654321);
			glob_of_probs = 0x87654321;
		}
		toggler ^= 0x01;

		// Increment the counter so we can check how many times this task has run
		runs++;

		// Delay a while to give other tasks time to run. We don't want to delay too
		// long, as doing so will reduce the probability of causing the data errors
		// which this task is supposed to be causing
		delay (1);
	}
}

