//**************************************************************************************
/** \file taskbase_status.cpp
 *    This file contains methods which print the status of each task, showing things
 *    such as the task's name, its priority, and if enabled, stack usage and number of
 *    times its loop has run. 
 *
 *  Revisions:
 *    \li 12-02-2012 JRR Split off from time_stamp.cpp to save memory in machine file
 *    \li 08-26-2014 JRR Changed file names and base task class name to TaskBase
 *    \li 01-04-2015 JRR Moved items around for more efficient use of screen space
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

#include <string.h>                         // For strlen() and Sparta (the Mean Kitty)
#include "taskbase.h"                       // Pull in the base class header file


//-------------------------------------------------------------------------------------
/** This method prints task status information, then asks the next task in the list of
 *  tasks to do so. The list is kept by the tasks, each having a pointer to another.
 *  @param ser_device The serial device to which each task prints its status
 */

void TaskBase::print_status_in_list (emstream* ser_device)
{
	*ser_device << *this << endl;

	if (prev_task_pointer != NULL)
	{
		prev_task_pointer->print_status_in_list (ser_device);
	}
}


//-------------------------------------------------------------------------------------
/** This method prints information about the task. It is called by the overloaded "<<"
 *  operator which is used by the task to print itself when asked to. This function is
 *  declared virtual so that descendents can override it to print additional 
 *  information. 
 *  @param ser_dev A reference to the serial device to which to print the task status
 */

void TaskBase::print_status (emstream& ser_dev)
{
	ser_dev.puts (pcTaskGetTaskName (handle));
	ser_dev.putchar ('\t');
	if (strlen ((const char*)(pcTaskGetTaskName (handle))) < 8)
	{
		ser_dev.putchar ('\t');
	}
	ser_dev << (uint8_t)(uxTaskPriorityGet (handle)) << PMS ("\t")
			<< get_state ()
		#if (INCLUDE_uxTaskGetStackHighWaterMark == 1)
			<< PMS ("\t") << uxTaskGetStackHighWaterMark(handle) << PMS ("/") 
			<< (size_t)(get_total_stack ()) << PMS ("\t")
		#endif
			<< PMS ("\t") << runs;
}


//-------------------------------------------------------------------------------------
/** This overloaded operator writes information about the task's status to the given 
 *  serial device. That information can be used for debugging or perhaps reliability 
 *  testing. Information written is:
 *    \li The task's name
 *    \li The priority
 *    \li The amount of stack used and total available
 *  @param ser_dev A reference to the serial device on which we're writing information
 *  @param a_task A reference to the task whose information is to be written
 *  @return A reference to the same serial device on which we write information.
 *          This is used to string together things to write with "<<" operators
 */

emstream& operator << (emstream& ser_dev, TaskBase& a_task)
{
	a_task.print_status (ser_dev);
	return (ser_dev);
}


//-------------------------------------------------------------------------------------
/** This function prints information about how all the tasks are doing. Beginning with
 *  the most recently created task, each task prints its status and then asks the next
 *  task in the list to do the same until the end of the list is found. 
 *  WARNING: The display of memory remaining in the task stacks, which is found by
 *  calls to FreeRTOS function uxTaskGetStackHighWaterMark(), seems to be suspicious.
 *  The author isn't sure if it can always be trusted. 
 *  @param ser_dev Pointer to a serial device on which the information will be printed
 */

void print_task_list (emstream* ser_dev)
{
	// Print the first line with the top of the headings
	*ser_dev << PMS ("Task\t\t  \t ")
		#if (INCLUDE_uxTaskGetStackHighWaterMark == 1)
			<< PMS ("\tStack")
		#endif
			<< endl;

	// Print the second line with the rest of the headings
	*ser_dev << PMS ("Name\t\tPri.\tState")
		#if (INCLUDE_uxTaskGetStackHighWaterMark == 1)
			<< PMS ("\tFree/Total")
		#endif
			<< PMS ("\tRuns") << endl;

	// Print the third line which shows separators between headers and data
	*ser_dev << PMS ("----\t\t----\t-----")
		#if (INCLUDE_uxTaskGetStackHighWaterMark == 1)
			<< PMS ("\t----------")
		#endif
			<< PMS ("\t----") << endl;

	// Now have the tasks each print out their status. Tasks form a linked list, so
	// we only need to get the last task started and it will call the next, etc.
	if (last_created_task_pointer != NULL)
	{
		last_created_task_pointer->print_status_in_list (ser_dev);
	}

	// Have the idle task print out its information
	*ser_dev << PMS ("IDLE\t\t0\t-\t")
		#if (INCLUDE_uxTaskGetStackHighWaterMark == 1)
			<< uxTaskGetStackHighWaterMark (xTaskGetIdleTaskHandle ())
			<< PMS ("/") << configMINIMAL_STACK_SIZE << PMS ("\t\t-")
		#endif
			<< endl;
}

