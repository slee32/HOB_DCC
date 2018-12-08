//**************************************************************************************
/** \file task_sink.cpp
 *    This file contains the source code for a task which receives data via a queue, a
 *    shared data item, and a dangerous global variable so that the data may be checked
 *    for errors in transmission. 
 *
 *  Revisions:
 *    \li 09-30-2012 JRR Original file was a one-file demonstration with two tasks
 *    \li 10-05-2012 JRR Split into multiple files, one for each task
 *    \li 10-25-2012 JRR Changed to a more fully C++ version with class task_sender
 *    \li 11-03-2012 JRR Morphed again into a data sink task with error checks
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

#include "task_source.h"                    // Header for data sender task
#include "task_sink.h"                      // Header file for this task


//-------------------------------------------------------------------------------------
/** This constructor creates a new data sink task. Its main job is to call the
 *  parent class's constructor which does most of the work.
 *  @param a_name A character string which will be the name of this task
 *  @param a_priority The priority at which this task will initially run (default: 0)
 *  @param a_stack_size The size of this task's stack in bytes 
 *                      (default: configMINIMAL_STACK_SIZE)
 *  @param p_ser_dev Pointer to a serial device (port, radio, SD card, etc.) which can
 *                   be used by this task to communicate (default: NULL)
 */

task_sink::task_sink (const char* a_name, 
					  unsigned portBASE_TYPE a_priority, 
					  size_t a_stack_size,
					  emstream* p_ser_dev
					 )
	: TaskBase (a_name, a_priority, a_stack_size, p_ser_dev)
{
	// Set all the error counters to zero; nothing has messed up yet
	queue_errors = 0;
	share_errors = 0;
	global_errors = 0;
}


//-------------------------------------------------------------------------------------
/** This method prints out the number of data errors we've seen by each method of
 *  sending data from the source task to this one.
 */

void task_sink::show_errors (void)
{
	*p_print_ser_queue << PMS ("Transmission errors in queue ") << queue_errors
					   << PMS (", shared_data ") << share_errors
					   << PMS (", global data ") << global_errors << endl;
}


//-------------------------------------------------------------------------------------
/** This method receives data from the data acquisition task in a queue and sends the
 *  information or saves it, depending on what its serial device pointer points to. 
 */

void task_sink::run (void)
{
	// Setup
	uint32_t received_data = 0x12345678;

	// Wait a moment before starting the loop so that the source task has time to make
	// some data available
	delay (10);

	// This is the task loop. Once the task has been initialized in the code just
	// above, the task loop runs, and it keeps running until the power is shut off
	for (;;)
	{
		// Check the queue from the source task. If there's data, read it; if not, go
		// and check the shared data
		if (p_queue_1->not_empty ())
		{
			received_data = p_queue_1->get ();
			if ((received_data & 0x00FF00FF) != 0)
			{
				queue_errors++;
				*p_print_ser_queue << PMS ("ERROR in queue, got ") << hex 
								   << received_data << dec << endl;
			}
		}

		// Check the data sent by other means as well
		received_data = p_share_1->get ();
		if (received_data != 0x12345678 && received_data != 0x87654321)
		{
			share_errors++;
			*p_print_ser_queue << PMS ("ERROR in share, got ") << hex << received_data
							   << dec << endl;
		}
		received_data = glob_of_probs;
		if (received_data != 0x87654321 && received_data != 0x12345678)
		{
			global_errors++;

			// Since this is a high priority task, we need to delay for a few ticks
			// each time we get an error, or the user interface task never gets to run
			delay (20);
		}
		runs++;
	}
}


//-------------------------------------------------------------------------------------
/** This method prints information about the status of this task. It is called by the
 *  overloaded "<<" operator so that when the task prints itself to a serial device,
 *  whatever this method wants printed gets printed. 
 *  @param ser_thing The serial device to which information will be printed
 */

void task_sink::print_status (emstream& ser_thing)
{
	// Call the parent task's printing function first
	TaskBase::print_status (ser_thing);

	// Show errors in transmission
	ser_thing << PMS ("\tErrors in queue: ") << queue_errors
			  << PMS (", shared_data: ") << share_errors
			  << PMS (", global data: ") << global_errors;
}

