//**************************************************************************************
/** \file task_user.cpp
 *    This file contains source code for a user interface task for a ME507/FreeRTOS
 *    test suite. 
 *
 *  Revisions:
 *    \li 09-30-2012 JRR Original file was a one-file demonstration with two tasks
 *    \li 10-05-2012 JRR Split into multiple files, one for each task
 *    \li 10-25-2012 JRR Changed to a more fully C++ version with class task_user
 *    \li 11-04-2012 JRR Modified from the data acquisition example to the test suite
 *    \li 12-06-2018 SBL Modified to run on an AtMega644a
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

#include <avr/io.h>                         // Port I/O for SFR's
#include <avr/wdt.h>                        // Watchdog timer header

#include "task_user.h"                      // Header for this file
#include "external_references.h"


/** This constant sets how many RTOS ticks the task delays if the user's not talking.
 *  The duration is calculated to be about 5 ms.
 */
const TickType_t ticks_to_delay = ((configTICK_RATE_HZ / 1000) * 5);


//-------------------------------------------------------------------------------------
/** This constructor creates a new data acquisition task. Its main job is to call the
 *  parent class's constructor which does most of the work.
 *  @param a_name A character string which will be the name of this task
 *  @param a_priority The priority at which this task will initially run (default: 0)
 *  @param a_stack_size The size of this task's stack in bytes 
 *                      (default: configMINIMAL_STACK_SIZE)
 *  @param p_ser_dev Pointer to a serial device (port, radio, SD card, etc.) which can
 *                   be used by this task to communicate (default: NULL)
 */

task_user::task_user (const char* a_name, 
					  unsigned portBASE_TYPE a_priority, 
					  size_t a_stack_size,
					  emstream* p_ser_dev
					 )
	: TaskBase (a_name, a_priority, a_stack_size, p_ser_dev)
{
}


//-------------------------------------------------------------------------------------
/** This task interacts with the user for force him/her to do what he/she is told. It
 *  also forces the data acquisition task to do what it is told. What a rude task this
 *  is. This method is the one which runs continuously and performs the functions
 *  described above. 
 */

void task_user::run (void)
{
	char char_in;							// Character read from serial device
	time_stamp a_time;						// Holds the time so it can be displayed

	// Right before running the loop, print the help message so the user knows which 
	// keys to press to do something
	print_help_message ();

	// This is the task loop. Once the task has been initialized in the code just
	// above, the task loop runs, and it keeps running until the power is shut off.
	// In the loop, we check for characters typed into the serial port and also check
	// for characters in the print queue; those characters are to be displayed
	for (;;)
	{
		// Check for any characters which might have been typed by the user
		if (p_serial->check_for_char ())
		{
			char_in = p_serial->getchar ();			// Read the character

			switch (char_in)
			{
                // Flip Flap #
                case '0':
                    *p_serial << "Stopping flip flap" << char_in << endl;
                    flap_num_com->put(0);
                    break;
                case '9':
                    *p_serial << "Running flip flap " << char_in << endl;
                    flap_num_com->put(9);
                    break;    
				// The 't' command asks what time it is now
				case 'n':
					*p_serial << (a_time.set_to_now ()) << endl;
					break;

				// The 's' command means dump all the tasks' stacks for examination
				case 's':
					print_task_stacks (p_serial);
					break;

				// Pressing 'v' gives the version number and setup of this program
				case 'v':
					show_status ();
					break;

				// A '?' or 'h' is a plea for help; respond with a help message
				case '?':
				case 'h':
					print_help_message ();
					break;
				// If the character isn't recognized, ask: What's That Function?
				default:
					//p_serial->putchar (char_in);
					*p_serial << endl;
					break;
			};
		}
		// If no character was typed in, check the print queue to see if another task
		// has sent this task something to be printed
		else	
		{
			delay_ms (1);
		}

		// We've made it safely through the loop one more time; claim some credit
		runs++;
	}
}


//-------------------------------------------------------------------------------------
/** This method prints a simple help message.
 */

void task_user::print_help_message (void)
{
	*p_serial << PMS ("FreeRTOS Task Communications Test Program help") << endl;
	*p_serial << PMS (" n:  Show the real time NOW") << endl;
	*p_serial << PMS (" v:  Show program version and setup") << endl;
	*p_serial << PMS (" s:  Dump all tasks' stacks") << endl;
	*p_serial << PMS (" h:  Print this help message") << endl;
}


//-------------------------------------------------------------------------------------
/** This method displays information about the status of the system, including
 *    \li The name and version of the program
 *    \li The name, status, priority, and free stack space of each task
 *    \li Processor cycles used by each task
 *    \li Amount of heap space free and setting of RTOS tick timer
 */

void task_user::show_status (void)
{
	time_stamp the_time;					// Holds current time for printing

	// First print the program version, compile date, etc. 
	*p_serial << endl << PROGRAM_VERSION << PMS (__DATE__) << endl 
			  << PMS ("System time: ") << the_time.set_to_now () << endl << endl
			  << PMS ("Heap: ") << heap_left() << "/" << configTOTAL_HEAP_SIZE
			  #ifdef OCR5A
				<< PMS (", OCR5A=") << OCR5A << endl;
			  #elif (defined OCR3A)
				<< PMS (", OCR3A=") << OCR3A << endl;
			  #else
				<< PMS (", OCR1A=") << OCR1A << endl;
			  #endif

	// Have the tasks print their status; then the same for the shared data items
	print_task_list (p_serial);
	*p_serial << endl;
	print_all_shares (p_serial);
}


// void task_user::run_vTaskList (void)
// {
// 	// If task execution time profiling is turned on, show the report
// 	#if (configGENERATE_RUN_TIME_STATS == 1)
// 		*p_serial << PMS ("Task\t\tCycles\t\t% Time") << endl;
// 		*p_serial << PMS ("----\t\t------\t\t------");
// 		vTaskGetRunTimeStats ((signed char*)task_info_buffer);
// 		*p_serial << task_info_buffer << endl;
// 	#endif // configGENERATE_RUN_TIME_STATS
// }

