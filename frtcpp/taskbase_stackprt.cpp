//**************************************************************************************
/** \file taskbase_stackprt.cpp
 *    This file contains functions to print the stacks of all the tasks for debugging
 *    and educational purposes. 
 *
 *  Revisions:
 *    \li 12-02-2012 JRR Split off from time_stamp.cpp to save memory in machine file
 *    \li 08-26-2014 JRR Changed file names and base task class name to TaskBase
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

#include "taskbase.h"                       // Pull in the base class header file


//-------------------------------------------------------------------------------------
/** This function has all the tasks in the task list do a "stack dump", printing their
 *  stacks in hex dump format. The idle task's stack is printed afterwards. 
 *  @param ser_dev Pointer to a serial device on which the information will be printed
 */
void print_task_stacks (emstream* ser_dev)
{
	// Tell the most recently created task to print its stack and keep the process
	// going by telling the next most recently created task to print its stack, etc.
	if (last_created_task_pointer != NULL)
	{
		last_created_task_pointer->print_stack_in_list (ser_dev);
	}

	// Now print the stack for the idle task, which isn't user created and so isn't
	// in the regular task list
	*ser_dev << PMS ("Task: IDLE") << endl;
	hex_dump_memory ((uint8_t*)(portStackTopForTask - configMINIMAL_STACK_SIZE + 1), 
								(uint8_t*)(portStackTopForTask + 1), ser_dev);
}


//-------------------------------------------------------------------------------------
/** @brief   Show one task's stack as a hex dump.
 *  @details This method displays the task's stack in the format of a hex dump, with 
 *           the data in the stack shown as a bunch of hexadecimal numbers as well as 
 *           text. Then it finds the previously created task and asks that task to 
 *           print its stack as well; when the process is done all tasks (except the 
 *           idle task) will have printed their stacks. The idle task's stack is 
 *           printed separately afterwards. 
 *  @param   p_ser_dev The serial device to which each task prints its stack
 */

void TaskBase::print_stack_in_list (emstream* p_ser_dev)
{
	*p_ser_dev << PMS ("Task: ") 
			   << (const char*)(pcTaskGetTaskName (handle)) 
			   << endl;

	dump_stack (p_ser_dev);

	if (prev_task_pointer != NULL)
	{
		prev_task_pointer->print_stack_in_list (p_ser_dev);
	}
}
