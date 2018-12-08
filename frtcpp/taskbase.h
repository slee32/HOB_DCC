//*************************************************************************************
/** @file    taskbase.h
 *  @brief   Headers for a class which implements tasks for task and state based 
 *           programs.
 *  @details This file contains a wrapper class which makes the use of FreeRTOS 
 *           threads a bit easier. It also adds a simple mechanism to support a finite
 *           state machine based construct within each task. 
 *
 *  Revised:
 *    \li 10-21-2012 JRR Original file
 *    \li 08-25-2012 JRR Modified to run with STM32's as well as AVR's
 *    \li 08-26-2014 JRR Changed file names and base task class name to TaskBase
 *    \li 09-03-2014 JRR Minor upgrades; renamed method to @c delay_from_for()
 *
 *  Credits:
 *      Much of this code uses techniques learned from Amigo software, which is 
 *      copyright 2012 by Digital Aggregates Corporation of Colorado, USA. The license
 *      used by Digital Aggregates is a modified version of the GNU Public License. 
 *      See http://www.diag.com/navigation/downloads/Amigo.html.
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

// This define prevents this .h file from being included more than once in a .cpp file
#ifndef _TASKBASE_H_
#define _TASKBASE_H_

#ifdef __AVR
	#include <avr/wdt.h>                    // Header for watchdog timer that reboots
	#include "time_stamp.h"                 // Header for timekeeping class
#endif

#include "FreeRTOS.h"                       // Main header for FreeRTOS
#include "task.h"                           // Header for FreeRTOS tasks

#include "mechutil.h"                       // Utility functions for the ME405 code
#include "emstream.h"                       // Pull in the base class header file


/* The forward declaration is needed so we can make last_created_task_pointer usable
 * from anywhere in this file. It would be nicer if this variable could be a static
 * member of class @c TaskBase, but the C++ standard won't allow it to be initialized
 * if it's a static member. For some reason, only integer types can be initialized at
 * startup if they're static class members.
 */
class TaskBase;

// This is the pointer to the last created task, created in taskbase.cpp
extern TaskBase* last_created_task_pointer;

/** This macro is used to set the priority of a task. In addition to being a little 
 *  more readable than the usual priority which is calculated from the idle task's
 *  priority, this macro checks to make sure that the asked for priority doesn't 
 *  exceed the maximum priority set in \c FreeRTOSConfig.h.
 */
#define task_priority(x) ((x) < (configMAX_PRIORITIES) ? \
                ((tskIDLE_PRIORITY) + (x)) : (configMAX_PRIORITIES))


/// @cond NO_DOXY 
//-------------------------------------------------------------------------------------
// If the AVR has 256KB flash, it uses a 3-byte program counter and the task function
// needs some special handling to work correctly. This only applies to Atmel AVR's

#if defined(__AVR_3_BYTE_PC__)
	extern "C" void _call_static_run_method (TaskBase* p_task) 
		__attribute__((section(".lowtext")));
#else
	extern "C" void _call_static_run_method (TaskBase* p_task);
#endif
/// \endcond


//-------------------------------------------------------------------------------------
/** @brief   Base class for implementations of tasks in task/state based programs.
 *  @details This class is a C++ wrapper for the FreeRTOS task functions with some 
 *           extra functionality for keeping track of state transitions and for 
 *           printing diagnostic information about how tasks are configured and how 
 *           they are running. 
 * 
 *  @section Usage
 *  In order to use a task, one must first create a child class of class @c TaskBase.
 *  The child class must at least have a constructor and a run method: 
 *  @code
 *  class TaskExample : public TaskBase
 *  {
 *  protected:
 *      // Task specific data goes here
 * 
 *  public:
 *      // This constructor creates an example task object
 *      	TaskExample (const char*, unsigned portBASE_TYPE, size_t, emstream*);
 * 
 *      // This run method is called by the RTOS and contains a loop 
 *      void run (void);
 *  };
 *  @endcode
 * 
 *  The task class's constructor can sometimes be quite simple, only calling the parent 
 *  constructor as shown in the example below:
 *  @code
 *  TaskExample::TaskExample (const char* a_name,
 *                            unsigned portBASE_TYPE a_priority,
 *                            size_t a_stack_size,
 *                            emstream* p_serial_dev
 *                           )
 *      : TaskBase (a_name, a_priority, a_stack_size, p_serial_dev)
 *  {
 *  }
 *  @endcode
 *  If the task needs to initialize device drivers or other things it owns, that can
 *  be done in the constructor. 
 * 
 *  Every task must have a user-written run method named @c run(). The run method is 
 *  where most of the functionality of the task is implemented. The run method must 
 *  contain an endless loop in which the task's state machine is implemented. A 
 *  variable called @c state is kept by the parent class @c TaskBase and used to 
 *  monitor the state in which the user's code is running, so one should \b not 
 *  declare another variable called @c state inside the run method:
 *  @code
 *  void TaskExample::run (void)
 *  {
 *      portTickType LastWakeTime;                 // For scheduling how often task runs
 *  
 *      // Initialise the LastWakeTime variable with the current time. This happens just
 *      // once, before the infinite loop is entered
 *      	LastWakeTime = get_tick_count ();
 * 
 *      // This is the task loop. Once the task has been initialized in the code just
 *      // above, the task loop runs, and it keeps running until the power is shut off
 *      for (;;)
 *      {
 *          // Run the task's state machine here
 *          switch (state)
 *          {
 *              case 0:
 *                  // Do one thing in State 0
 *                  ...
 *                  // Check for a transition to another state
 *                  if (something)
 *                  {
 *                      transition_to (1);
 *                  }
 *                  break;
 * 
 *              case 1:
 *                  // Do another thing in State 1
 *                  ...
 *                  break;
 * 
 *              case 2:
 *                  // Do something completely different in State 2
 *                  ...
 *                  break;
 *          };
 * 
 *          // Tell the RTOS to delay until the given number of RTOS timer ticks have
 *          // elapsed. This means the code after this line runs every ticks_per_run
 *          // milliseconds if the RTOS interrupt is set to go off every 1 millisecond
 *          delay_from_for (LastWakeTime, ticks_per_run);
 *      }
 *  }
 *  @endcode
 * 
 *  Tasks are traditionally instantiated within @c main(). The most important and
 *  tricky part about creating a task is getting its \b stack size right. If a task's
 *  stack is too large, it will waste RAM, and small microcontrollers don't have much
 *  RAM to waste. If the task's stack is too small, it will cause disaster, usually in
 *  the form of the processor either hanging or rebooting as the stack pointer for the
 *  task goes outside the allocated range and overwrites the stack for another task. 
 *  For FreeRTOS tasks on an 8-bit AVR microcontroller, the smallest stack size that
 *  is likely to ever work is about 100 bytes. As soon as data is created within the 
 *  run method, queues are used, and other memory is used, the necessary stack size
 *  goes up; it is common to need 300 -- 500 bytes of stack space for a task that does
 *  a lot of work. Because it's very difficult to calculate the stack space needed for
 *  a task, the easiest way to set the stack size is usually to make it large enough
 *  that the program runs reliably, then decrease it by about 20 bytes at a time, 
 *  recompile and test the program, again and again until something fails. Then make
 *  the stack size a couple dozen bytes larger than the smallest size that reliably
 *  worked during testing...just in case. 
 *  
 * The code used to create a task object looks like the following:
 *  @code
 *  // Create an example task. It runs at priority 2 and has a 200 byte stack
 *  TaskExample* my_example_task 
 *      = new TaskExample ("Example", tskIDLE_PRIORITY + 2, 200, &ser_port);
 *  ...
 *  // Start up the scheduler, causing all tasks to be run
 *  vTaskStartScheduler ();
 *  @endcode
 *  In this example code, someone must have created the serial port object named 
 *  @c ser_port previously; the serial port will be used by the task to print out
 *  diagnostic information. If no diagnostic information needs to be printed, the
 *  serial port may be left out of the task constructor call or set to @c NULL. 
 * 
 */

class TaskBase
{
	// These private functions can't be accessed from outside this class. The two
	// methods here can't be used to construct anything; this is intentional, as
	// it prevents an object of this class from being copied. One shouldn't copy a
	// task object; it's unnatural and can lead one to moral instability and ennui
	private:
		/** This copy constructor is poisoned by being declared private so that it 
		 *  can't be used. One should not copy a task object.
		 *  @param that_clod A reference to a task which ought not be copied
		 */
		TaskBase (const TaskBase& that_clod);

		/** This assignment operator is poisoned by being declared private so that it
		 *  can't be used to copy the contents of one task into another. There is no
		 *  legitimate, morally acceptable reason to copy tasks that way. 
		 *  @param that_clod A reference to a task which ought not be copied
		 */
		TaskBase& operator= (const TaskBase& that_clod);

	// This protected data can only be accessed from this class or its descendents
	protected:
		/// This is the handle of this RTOS task. It's typedef'd as a pointer type. 
		TaskHandle_t handle;

		/** This is a pointer to the previously created task. The pointer will be used
		 *  to implement a linked list of tasks through which one can traverse.
		 */
		TaskBase* prev_task_pointer;

		/** This pointer can point to a serial output device or port which will be used
		 *  for various diagnostic printouts or logging.
		 */
		emstream* p_serial;

		#if (INCLUDE_uxTaskGetStackHighWaterMark == 1)
			/// This is the size of the task's total stack space. 
			size_t total_stack;

			/** This is a pointer to the top (beginning) of the task's stack. It is
			 *  used when we want to print out the stack for debugging purposes.
			 */
			size_t top_of_stack;
		#endif

		/** This is the state in which the finite state machine of the task is. This
		 *  variable should be used inside run() to implement the state machine so
		 *  that state transitions can be tracked, if needed, by this parent class.
		 */
		uint8_t state;

		/** This variable keeps track of the previous (before run() runs) value of the
		 *  state so that transitions can be conveniently detected.
		 */
		uint8_t previous_state;

		/** This variable keeps track of how many times the task has run through its
		 *  loop. In order for it to work, the user must put a line of code that
		 *  increments this variable, as in @c runs++; somewhere in the task loop.
		 */
		uint32_t runs;

		/** This method allows descendent classes to find out how many times the
		 *  @c loop() method has run.
		 *  @return The number of times the loop has been run
		 */
		uint32_t get_loop_runs (void)
		{
			return (runs);
		}

	// Public methods can be called from anywhere in the program where there is a 
	// pointer or reference to an object of this class
	public:
		// This constructor creates a FreeRTOS task with the given task run function, 
		// name, priority, and stack size
		explicit TaskBase (const char* a_name, 
						   unsigned portBASE_TYPE a_priority = 0, 
						   size_t a_stack_size = configMINIMAL_STACK_SIZE,
						   emstream* p_ser_dev = NULL);

		// Method called by the task's static run method which is, in turn,
		// called by the FreeRTOS run function
		static void _call_users_run_method (TaskBase*);

		/** @brief   Run method which holds the user's task code.
		 *  @details This is the method in which the task runs the user's task code.
		 *           The base class's @c run() method contains no code, and an object
		 *           of class @c TaskBase cannot be instantiated. The user's code for
		 *           each task goes in the @c run() method for each of the user's task
		 *           classes, each task class being descended from @c TaskBase. Often
		 *           the user's @c run() method contains a finite state machine; the 
		 *           variable @c state is provided so that the state machine's 
		 *           operation can be monitored by the parent class to help with 
		 *           debugging. 
		 *
		 *           Because this code is expected to run in a preemptive multitasking
		 *           environment, the run method should usually contain an infinite 
		 *           loop. An exception is when tasks are being dynamically created 
		 *           and deleted; in this case, one can program a run method which 
		 *           exits, and after exiting a task will be deleted. If tasks are not
		 *           being deleted, exiting a run method just causes the task to hang 
		 *           in an indefinite loop of repeated delays. Note that a finite state
		 *           machine pretty much replaces the functionality of tasks which are
		 *           dynamically created and deleted, except that memory is not 
		 *           recovered when the state machine transitions from one state to 
		 *           another. 
		 */
		virtual void run (void) = 0;

		// This method is called within run() to cause a state transition
		void transition_to (uint8_t);

		/** @brief   Set the task's serial device pointer.
		 *  @details This method sets the task's serial device pointer to the given 
		 *           address. Changing this serial device pointer means that debugging
		 *           output will be directed to the given device. This can be helpful 
		 *           if task transitions or other debugging information should be 
		 *           shown on a serial console as the task is being created but logged
		 *           somewhere else when the task is running.
		 *  @param   p_new_dev A pointer to the serial device on which debugging
		 *                     information will be printed in the near future
		 */
		void set_serial_device (emstream* p_new_dev)
		{
			p_serial = p_new_dev;
		}

		/** @brief   Turn off serial logging by un-setting the task's serial pointer.
		 *  @details This method un-sets the task's serial device pointer. Doing so 
		 *           prevents serial debugging output from being sent or logged in 
		 *           the future unless @c set_serial_device() is called to set the 
		 *           serial device pointer to a serial device.
		 */
		void unset_serial_device (void)
		{
			p_serial = NULL;
		}

		/** @brief   Check if a task is ready to run.
		 *  @details This overloaded operator allows one to check if the task is valid
		 *           and ready to run.  It looks at the task's handle, which is nonzero
		 *           if the task has been successfully created and hasn't been stopped.
		 *  @return  True if the task has a valid RTOS task handle and false if not
		 */
		operator bool () const
		{
			return (handle != 0);
		}

		/** @brief   Stop running the task for the given number of RTOS ticks.
		 *  @details This method causes the task to stop running for the given number
		 *           of RTOS ticks from the time it is called. This method should 
		 *           @b not be used to make a task run at regular intervals, because
		 *           the time between when the previous delay ended and and when 
		 *           @c delay() is called subsequently can vary, leading to the 
		 *           accumulation of errors in timing. For periodic runs of code in a
		 *           task, see @c delay_from_for().
		 *  @param   duration The amount of time, as a number of timer ticks, this 
		 *                    task should wait until next running
		 */
		void delay (TickType_t duration)
		{
			vTaskDelay (duration);
		}

		/** @brief   Stop the task for approximately the given number of milliseconds.
		 *  @details This method causes the task to stop running for approximately the
		 *           given number of milliseconds. This method should @b not be used 
		 *           to make a task run at regular intervals, because the time between
		 *           when the previous delay ended and and when @c delay_ms() is 
		 *           called subsequently can vary, leading to the accumulation of 
		 *           errors in timing. For periodic runs of code in a task, see 
		 *           @c delay_from_for_ms().
		 *  @param   duration_ms The duration for the task to stop in milliseconds
		 */
		void delay_ms (TickType_t duration_ms)
		{
			TickType_t ticks = ((uint32_t)duration_ms * configTICK_RATE_HZ) / 1000UL;
            vTaskDelay (ticks);
        }

		/** @brief   Stop the task from running for a precise time interval.
		 *  @details This method causes the task to stop running from a given time 
		 *           for a specified duration. The start time and duration are given
		 *           in units of RTOS timer ticks. This method can be used to 
		 *           implement a task that regularly wakes up and performs some 
		 *           action, like a clown waking up to terrify children. Because the
		 *           time at which each awakening takes place is recorded, this method
		 *           won't accumulate errors as it is repeatedly invoked. 
		 *  @param   from_ticks The beginning time of the duration to delay. It is
		 *                      usually set equal to the time at which the previous
		 *                      delay began so as to get precise, regular timing
		 *  @param   for_how_long The duration of the delay interval in RTOS ticks
		 */
		void delay_from_for (TickType_t& from_ticks, TickType_t for_how_long)
		{
			vTaskDelayUntil (&from_ticks, for_how_long);
		}

		/** @brief   Stop the task from running for a precise number of milliseconds.
		 *  @details This method causes the task to delay from a given time for a 
		 *           specified duration in milliseconds. This is usually done to make
		 *           a task that runs code at a fairly precise and regular interval. 
		 *           The start time is given in units of RTOS timer ticks; although 
		 *           ticks might not equal milliseconds, the user need not care, just
		 *           store the value in a variable of type \c TickType_t. The delay
		 *           duration is given in milliseconds. An example of how this method
		 *           is used follows:
         *           \code
         *           // Make a variable which will hold tick counts and initialize it
         *           TickType_t previousTicks = xTaskGetTickCount ();
         *           ...
         *           for (;;)                   // The task loop
         *           {
         *               ...                    // User code to run every 10 ms
         *               ...
         *               delay_from_for_ms (previousTicks, 10);
         *           }
         *           \endcode
		 *           @b Warning: In order for this function to provide accurate timing,
		 *           the RTOS tick rate (set in @c FreeRTOSConfig.h ) must be set to a
		 *           rate such that RTOS ticks occur at the desired time interval. For
		 *           example, if a task must awaken once per millisecond, ticks at 1 ms
		 *           or 0.5 ms are OK, but ticks every 2 ms or 0.7 ms will not allow
		 *           pricise timing. 
         *  @param   from_ticks The beginning time of the duration to delay. It is
         *                      usually set equal to the end time of the previous
         *                      delay so as to get precise, regular timing
         *  @param   millisec The duration of the delay interval in milliseconds
         */
        void delay_from_for_ms (TickType_t& from_ticks, TickType_t millisec)
        {
            TickType_t ticks = ((uint32_t)millisec * configTICK_RATE_HZ) / 1000UL;
            vTaskDelayUntil (&from_ticks, ticks);
        }

		/** @brief   Find out how many RTOS ticks since the scheduler was started.
		 *  @details This method returns the number of RTOS ticks from the time the
		 *           scheduler was started up until the time the method is called. By
		 *           using the number of RTOS ticks per second, one can determine real
		 *           time approximately. The precision of this method of timekeeping
		 *           isn't really good because RTOS ticks typically only occur every
		 *           millisecond or so; for high precision timing one needs to use 
		 *           a timer/counter running much faster than that. 
		 *  @return  The number of RTOS ticks which have happened since RTOS startup
		 */
		TickType_t get_tick_count (void)
		{
			return (xTaskGetTickCount ());
		}

		/** @brief   The task gives control to the RTOS immediately.
		 *  @details This method causes the task in which it's called to yield, giving
		 *           control of the CPU back to the RTOS, immediately. The RTOS will
		 *           then decide which task should run; if another task of higher
		 *           priority wants to run, that task will take over. 
		 */
		void yield (void)
		{
			taskYIELD ();
		}

		/** @brief   Return the task's current priority.
		 *  @return  The priority at which the task is currently running
		 */
		portBASE_TYPE get_priority (void)
		{
			return (uxTaskPriorityGet(handle));
		}

		/** @brief   Set this task's priority to a new value. 
		 *  @param   new_priority The new priority to set for this task
		 */
		void set_priority (portBASE_TYPE new_priority) 
		{
			vTaskPrioritySet (handle, new_priority);
		}

		/** @brief   Return the number of unused bytes in the heap.
		 *  @details This method returns the number of bytes left to be used in the 
		 *           heap. This means that the number returned is how many bytes 
		 *           available for allocation. 
		 *  @return  The approximate number of bytes left for use in the heap
		 */
		size_t heap_left (void)
		{
			return (xPortGetFreeHeapSize ());
		}

		#if (INCLUDE_uxTaskGetStackHighWaterMark == 1)
			/** @brief   Return the number of bytes available in this task's stack.
			 *  @details This method returns an estimate of the number of bytes left 
			 *           in this task's stack. The estimate is made based on the 
			 *           smallest number of bytes seen to be left in the stack when 
			 *           this task was previously context-switched, the "high water 
			 *           mark", so it's not necessarily an  exact measure of the 
			 *           available stack space. 
			 *  @return  The approximate number of bytes left in this task's stack
			 */
			size_t stack_left (void)
			{
				return (uxTaskGetStackHighWaterMark(handle));
			}
		#endif

		/** @brief   Print a display of this task's stack space. 
		 *  @details This method prints a hexadecimal and text format dump of the 
		 *           stack's contents for debugging and instructional purposes.
		 *  @param   p_ser_dev The serial device to which the stack will be printed
		 */
		void dump_stack (emstream* p_ser_dev)
		{
			hex_dump_memory ((uint8_t*)(top_of_stack - total_stack), 
								(uint8_t*)(top_of_stack), p_ser_dev);
		}

		/** @brief   Print a stack dump within a dump of all tasks' stacks.
		 *  @details This method prints a stack dump, then asks the next task in the 
		 *           linked list of tasks to do the same, and so on and so on, so all
		 *           the tasks have shown their stack space. 
		 *  @param   p_ser_dev A pointer to a serial device on which the stack is shown
		 */
		void print_stack_in_list (emstream* p_ser_dev);

		/** @brief   Return the total stack size for this task.
		 *  @details This method returns the task's total stack size, which was set in
		 *           the constructor call.
		 *  @return  The task's total stack size in bytes
		 */
		size_t get_total_stack (void)
		{
			return (total_stack);
		}

		/** @brief   Return the current state in which this task's state machine is.
		 *  @details This method returns the transition logic state in which the task
		 *           is at the current time. This is the value of the variable @c state
		 *           which is manipulated by the user within the @c run() method to 
		 *           cause state transitions.
		 *  @return  The current state
		 */
		uint8_t get_state (void)
		{
			return (state);
		}

		/** @brief   Return an approximate measurement of time from the tick count.
		 *  @details This method gets the tick count, which is the number of RTOS ticks
		 *           which have occurred since the RTOS was started up, and converts
		 *           the value from ticks into seconds. The resolution of this time
		 *           measurement is only as good as the resolution of the RTOS tick 
		 *           rate, typically around a millisecond or so.
		 *  @return  The approximate real time
		 */
		float get_tick_time (void)
		{
			return ((float)(xTaskGetTickCount ()) / (float)configTICK_RATE_HZ);
		}

		/** @brief   Return a pointer to this task's name.
		 *  @details This method returns a pointer to the task's name, which resides in
		 *           a null terminated character array belonging to the task. Because
		 *           the pointer is of type @c const @c char, that pointer cannot be
		 *           used to change the task's name (unless typecasting tricks are used
		 *           in a way that is poor programming practice). 
		 *  @return  A pointer to the task's name
		 */
		const char* get_name (void)
		{
			return ((const char*)(pcTaskGetTaskName (handle)));
		}

		// This method prints task status information, then asks the next task in the
		// list to do so
		void print_status_in_list (emstream*);

		/** @brief   Return a pointer to the most recently created task.
		 *  @details This method returns a pointer to the most recently created task.
		 *           This pointer is the head of a linked list of tasks; the list is 
		 *           maintained by the task objects themselves. This pointer to the 
		 *           most recently created task is used to begin traversing the list 
		 *           of all tasks when some action needs to be taken by all the tasks.
		 *  @return  A pointer to the most recently created task
		 */
		static const TaskBase* get_last_created_task_pointer (void)
		{
			return (last_created_task_pointer);
		}

		/** @brief   Return a handle to the FreeRTOS task wrapped by this task object.
		 *  @details This method returns the handle of the FreeRTOS task which is 
		 *           inside this object. Advanced users might want to use it to access
		 *           task manipulation functions that aren't in this wrapper class or
		 *           for other creative hacking.
		 *  @return  The handle of the FreeRTOS task which is wrapped in this handy
		 *           C++ class
		 */
		TaskHandle_t get_handle (void)
		{
			return (handle);
		}

		// Print the status of this task
		virtual void print_status (emstream&);

		// Print an error message and reset the processor in an extreme emergency
		void emergency_reset (const char* message);

		// TODO: Add other functions we might like: suspend(), resume(), 
		// resumeFromISR(), etc. 
};

// This operator writes information about the task's status to the given serial device.
// That information can be used for debugging or perhaps reliability testing
emstream& operator << (emstream&, TaskBase&);

// This function prints information about how all the tasks are doing
void print_task_list (emstream* ser_dev);

// This function has all the tasks print their stacks
void print_task_stacks (emstream* ser_dev);

// Get time from the RTOS tick count, converted to seconds
float get_tick_time_float (void);

#endif  // _TASKBASE_H_
