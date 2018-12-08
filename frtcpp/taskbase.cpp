//*************************************************************************************
/** @file    taskbase.cpp
 *  @brief   Source code for a class which implements tasks for task and state based 
 *           programs.
 *  @details This file contains a wrapper class which makes the use of FreeRTOS 
 *           threads a bit easier. It also adds a simple mechanism to support a finite
 *           state machine based construct within each task. 
 *
 *  Revised:
 *    \li 10-21-2012 JRR Original file
 *    \li 08-25-2012 JRR Modified to run with STM32's as well as AVR's
 *    \li 08-26-2014 JRR Changed file names and base task class name to TaskBase
 *
 *  Credits:
 *      This code uses techniques learned from Amigo software, which is copyright 2012 
 *      by Digital Aggregates Corporation of Colorado, USA. The license used by Digital
 *      Aggregates is a modified version of the GNU Public License; see 
 *      http://www.diag.com/navigation/downloads/Amigo.html.
 *
 *  License:
 *		This file is copyright 2014 by JR Ridgely and released under the Lesser GNU 
 *		Public License, version 2. It intended for educational use only, but its use
 *		is not limited thereto. */
/*		THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 *		AND	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 *		IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 *		ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
 *		LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUEN-
 *		TIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
 *		OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
 *		CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
 *		OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 *		OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */
//*************************************************************************************

#ifdef __ARMEL__
	#include "stm32f4xx_iwdg.h"             // Used for system reset function
#endif
#include "taskbase.h"                       // Pull in the base class header file


/** @brief   A pointer to the most recently created task. 
 *  @details This static pointer to the most recently created task is used to find 
 *           the beginning of a linked list of tasks. It's initially set to @c NULL,
 *           but each time a task is created, it's changed to point to the newly 
 *           created task.
 */
TaskBase* last_created_task_pointer = NULL;


//-------------------------------------------------------------------------------------
/** @brief   Constructor which creates and initializes a task object.
 *  @details This constructor creates a FreeRTOS task with the given task run function,
 *           name, priority, and stack size. It saves a pointers to a serial device to 
 *           be used for debugging; it also saves a pointer to the previously created 
 *           task (if any) so that tasks can form a linked list. Any function, such as 
 *           diagnostic printouts, that is to be performed by all tasks can be done by
 *           telling the most recently created task to do it, then have that most 
 *           recently created task tell the previously created task to do it, and so 
 *           on.
 * 
 *           The odd parameter with the @c reinterpret_cast directive is a pointer to 
 *           the FreeRTOS task run function. This pointer, which is supplied to 
 *           FreeRTOS as a pointer to a function that takes one void pointer parameter 
 *           and returns nothing, contains a reference to a method belonging to this 
 *           class called @b _call_static_run_method(). FreeRTOS will then call 
 *           @b _call_static_run_method() which is a regular C (not C++) function that
 *           is given a pointer to the task object which the user has created. 
 *           @b _call_static_run_method(), being a member of this class, then calls the
 *           user's \b run() method using the fancy virtual method function finding 
 *           tricks that it can use because it is a C++ method rather than a plain old 
 *           C function. All this C++ and virtual method fanciness only has to take
 *           place once as the scheduler is starting up, so it doesn't slow real-time 
 *           operation down. 
 *  @param   a_name A character string which will be the name of this task
 *  @param   a_priority The priority at which this task will initially run (default: 0)
 *  @param   a_stack_size The size of this task's stack in bytes 
 *                        (default: @c configMINIMAL_STACK_SIZE)
 *  @param   p_ser_dev Pointer to a serial device (port, radio, SD card, etc.) which 
 *                     can be used by this task to communicate (default: NULL)
 */

TaskBase::TaskBase (const char* a_name, unsigned portBASE_TYPE a_priority, 
					size_t a_stack_size, emstream* p_ser_dev)
{
	// Create the task with a call to the RTOS task creation function
	portBASE_TYPE task_status = xTaskCreate
		(
		 reinterpret_cast<void(*)(void*)>(_call_static_run_method), // The run method
		 (const char*)a_name,                                       // Task name
		 a_stack_size,                                              // Task stack size
		 this,                                      // Pointer to this frt_task object
		 a_priority,                                // Priority for the new task
		 &handle                                    // The new task's handle
		);

	// Save the serial port pointer and the total stack size
	p_serial = p_ser_dev;
	total_stack = a_stack_size;

	// Set the handles to the previously and last created tasks
	prev_task_pointer = last_created_task_pointer;
	last_created_task_pointer = this;

	// Initialize the finite state machine and its transition logger
	state = 0;
	previous_state = 0;

	// If stack tracing is being used, save the address of the top of the stack
	top_of_stack = ++portStackTopForTask;

	// Initialize the run counter
	runs = 0;

	// If the serial port is being used, let the user know if the task was created
	// successfully
	if (p_serial != NULL)
	{
		if (task_status == pdPASS)
		{
			*p_serial << PMS ("Task \"") << a_name << PMS ("\" created, stack at 0x")
				<< hex << (portPOINTER_SIZE_TYPE)top_of_stack << dec << endl;
		}
		else
		{
			*p_serial << PMS ("ERROR creating task \"") << a_name << '"' << endl;
		}
	}
}


//-------------------------------------------------------------------------------------
/** @brief   Internal use only function which calls the @c run() method.
 *  @details <b>This function never needs to be called by user-written code.</b>
 *           This is the task function which is called by the RTOS scheduler. It needs
 *           to call the task's run function which is a member of the user's task 
 *           class, which is a descendent of class @c TaskBase. In order to call the 
 *           user's run method, the RTOS run function must first call a static task 
 *           method, \c _call_users_run_method(), which is also defined in class 
 *           @c TaskBase. 
 *  @param   p_task A pointer to the task is given as a parameter to this task 
 *                  function; the pointer is used to find the task's static run method
 */

extern "C" inline void _call_static_run_method (TaskBase* p_task)
{
	TaskBase::_call_users_run_method (p_task);
}


//-------------------------------------------------------------------------------------
/** @brief   Internal use only function which calls the @c run() method.
 *  @details <b>This method never needs to be called by user-written code.</b>
 *           This @c static method calls the user-written \c run() method when it has 
 *           been called by @c _call_static_run_method(). The function 
 *           @c _call_static_run_method() function was, in turn, called by FreeRTOS; 
 *           it is the C (not C++) function which was registered with the scheduler.
 *  @param   p_task A pointer to the task (this task) whose run method is to be called
 */

void TaskBase::_call_users_run_method (TaskBase* p_task)
{
	// This is where the user's run() method is actually called
	p_task->run ();

	// If the run() method exits, and if task deletion is enabled, delete this task
	#if (INCLUDE_vTaskDelete == 1)
		void* temp_handle = p_task->handle;
		p_task->handle = 0;
		vTaskDelete (temp_handle);
	// If task deletion is disabled, set this task's handle to 0 but don't delete task
	#else
		p_task->handle = 0;
	#endif

	// If task deletion is not enabled, we get here if the task run method has exited 
	// (which it shouldn't have, though). 
	for (;;)
	{
		vTaskDelay (portMAX_DELAY);
	}
}


//-------------------------------------------------------------------------------------
/** @brief   Cause this task to transition to another state.
 *  @details This method is called within @c run() to cause a state transition. It 
 *           changes the variable @c state, and if transition logging is enabled, it 
 *           logs the transition to help with debugging.
 *  @param   new_state The state to which this task will transition
 */

void TaskBase::transition_to (uint8_t new_state)
{
	state = new_state;

	// If transition tracing is enabled, print data about the transition
	#ifdef TRANSITION_TRACE
		if (p_serial)
		{
			*p_serial << tick_res_time () << ":" << (char*)(pcTaskGetTaskName (handle))
					<< ":" << previous_state << PMS ("->") << state << endl;
		}
	#endif // TRANSITION_TRACE

	previous_state = state;
}


//-------------------------------------------------------------------------------------
/** @brief   Print an error message if possible and reset the processor.
 *  @details This method prints an error message (if there is a valid serial device 
 *           pointer available) and resets the processor. It should only be used in 
 *           cases of things going seriously to heck.
 *  @param   message A message to print before restarting; @c endl is appended to it
 */

void TaskBase::emergency_reset (const char* message)
{
	if (p_serial != NULL)
	{
		*p_serial << message << endl;
	}
	#ifdef __AVR
		wdt_enable (WDTO_120MS);
		for (;;);
	#elif defined __ARMEL__
	if (p_serial != NULL)
	{
		*p_serial << "Unable to reset STM32" << endl;
	}

// 		NVIC_SystemReset ();

// 		portENTER_CRITICAL ();
// 
// 		// Use the STM32 watchdog timer to cause a reset in about 1 ms
// 		IWDG_WriteAccessCmd (IWDG_WriteAccess_Enable);
// 		while (IWDG_GetFlagStatus (IWDG_FLAG_PVU));
// 		IWDG_SetPrescaler (IWDG_Prescaler_32);
// 		while (IWDG_GetFlagStatus (IWDG_FLAG_RVU));
// 		IWDG_SetReload (1);
// 		IWDG_Enable ();
// 		// Wait until the reset occurs
// 		for (;;)
// 		{
// 		}
	#endif
}


//-------------------------------------------------------------------------------------
/** @brief   Get time from the RTOS tick count, converted to seconds.
 *  @details This function returns the time since the RTOS was started up. The time is
 *           measured as a count of RTOS ticks since startup, divided by the rate of
 *           ticks per second, to give a floating point number holding time in seconds.
 */

float get_tick_time_float (void)
{
	return ((float)(xTaskGetTickCount ()) / (float)(configTICK_RATE_HZ));
}
