//**************************************************************************************
/** \file task_user.cpp
 *    This file contains source code for a user interface task for a ME405/FreeRTOS
 *    test suite. 
 *
 *  Revisions:
 *    \li 09-30-2012 JRR Original file was a one-file demonstration with two tasks
 *    \li 10-05-2012 JRR Split into multiple files, one for each task
 *    \li 10-25-2012 JRR Changed to a more fully C++ version with class task_user
 *    \li 11-04-2012 JRR Modified from the data acquisition example to the test suite
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

#include "shared_data_sender.h"
#include "shared_data_receiver.h"
#include "task_user.h"                      // Header for this file
#include "shares.h"
#include "frt_shared_data.h"

/** This constant sets how many RTOS ticks the task delays if the user's not talking.
 *  The duration is calculated to be about 5 ms.
 */
const portTickType ticks_to_delay = ((configTICK_RATE_HZ / 1000) * 5);


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
	: frt_task (a_name, a_priority, a_stack_size, p_ser_dev)
{
	// Nothing is done in the body of this constructor. All the work is done in the
	// call to the frt_task constructor on the line just above this one
}


//-------------------------------------------------------------------------------------
/** This task interacts with the user for force him/her to do what he/she is told. It
 *  is just following the modern government model of "This is the land of the free...
 *  free to do exactly what you're told." 
 */


void task_user::run (void)
{
	uint8_t mode;
	mode = 3;
	UI_command->put(mode);
	char char_in;                           // Character read from serial device
	time_stamp a_time;                      // Holds the time so it can be displayed

	// This is an infinite loop; it runs until the power is turned off. There is one 
	// such loop inside the code for each task
	print_main_menu();
	for (;;)
	{
		// Run the finite state machine. The variable 'state' is kept by the parent class
		switch (state)
		{
			// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
			// In state 0, we are in the main menu
			case (0):
				if (p_serial->check_for_char ())        // If the user typed a
				{                                       // character, read
					char_in = p_serial->getchar ();     // the character

					// In this switch statement, we respond to different characters
					switch (char_in)
					{
						// "H" is the command to home the striker
						case ('h'):
							*p_serial << PMS ("Starting Homing") << endl<<endl;
							mode = 1;
							UI_command->put(mode);
							transition_to(1);
							break;

						// "R" is the command to run the hockey program
						case ('r'):
							mode = 2;
							UI_command->put(mode);
							*p_serial << PMS ("Starting Run") << endl<<endl;
							print_run_menu();
							transition_to (2);
							break;
							
							
						// "S" is the command to run the hockey striker
						case ('s'):
							*p_serial << PMS ("Stopping") << endl<<endl;
							mode = 3;
							UI_command->put(mode);
							transition_to (3);
							break;
							
						// "Z" is the command to Zero the striker
						case ('z'):
							*p_serial << PMS ("Zeroing") << endl<<endl;
							mode = 4;
							UI_command->put(mode);
							transition_to (4);
							break;
						
						// Any other character will be ignored
						default:
							break;
					};
				}

				// Check the print queue to see if another task has sent this task 
				// something to be printed
				else if (print_ser_queue.check_for_char ())
				{
					p_serial->putchar (print_ser_queue.getchar ());
				}

				break; // End of state 0

			// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
			// In state 1, we're in homing mode, so the task will wait for the signal from 
			// master that homing is complete
			case (1):
				*p_serial << PMS ("Homing...") << endl;
				//get function
				if (1 == true)
				{
					*p_serial << PMS ("Homing Complete") << endl<< endl<<endl<< endl<<endl;
					print_main_menu();
					mode = 3;
					UI_command->put(mode);
					transition_to(0);
				}
				break; // End of state 1
				
				
			// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
			// In state 2, we're in running mode, so the task will run the control loop
			case (2):
				if (p_serial->check_for_char ())        // If the user typed a
				{                                       // character, read
					char_in = p_serial->getchar ();     // the character

					// In this switch statement, we respond to different characters
					switch (char_in)
					{
						// "S" is the command to stop the striker
						case ('s'):
						*p_serial << PMS ("Stopping") << endl<< endl<<endl<< endl<<endl<<endl;
						mode = 3;
						UI_command->put(mode);
						print_main_menu();
						transition_to(3);
						break;
					
						// Any other character will be ignored
						default:
						break;
					}
				}
				break; // End of state 2

			// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
			// In state 3, we're in stopping mode, so the task will stop the motors
			case (3):
				*p_serial << PMS ("Stopped") << endl<<endl<< endl<<endl;
				print_main_menu();
				transition_to(0);
				break; // End of state 3
			
			
			
			// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
			// In state 4, we're in Zeroing mode, so the task will zero the striker
			case (4):
			*p_serial << PMS ("Zeroed") << endl<<endl<< endl<<endl;
			print_main_menu();
			transition_to(0);
			break; // End of state 4	
				

			// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
			// We should never get to the default state. If we do, complain and restart
			default:
				*p_serial << PMS ("Illegal state! Resetting AVR") << endl;
				wdt_enable (WDTO_120MS);
				for (;;);
				break;

		} // End switch state

		runs++;                             // Increment counter for debugging

		// No matter the state, wait for approximately a millisecond before we 
		// run the loop again. This gives lower priority tasks a chance to run
		vTaskDelay (configMS_TO_TICKS (1));
	}
}


//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
/** This method prints a simple help message.
 */

void task_user::print_help_message (void)
{
	*p_serial << ATERM_BKG_CYAN << ATERM_TXT_BLACK << clrscr;
	*p_serial << PROGRAM_VERSION << PMS (" help") << endl;
	*p_serial << PMS ("  Ctl-C: Reset the AVR") << endl;
	*p_serial << PMS ("  Ctl-A: Enter command mode") << endl;
	*p_serial << PMS ("  In command mode only:") << endl;
	*p_serial << PMS ("    n:   Show the time right now") << endl;
	*p_serial << PMS ("    v:   Version and setup information") << endl;
	*p_serial << PMS ("    s:   Stack dump for tasks") << endl;
	*p_serial << PMS ("    e:   Exit command mode") << endl;
	*p_serial << PMS ("    h:   HALP!") << endl;
}

void task_user::print_main_menu (void)
{
	*p_serial << PMS ("Welcome to Hockey Death Match") << endl;
	*p_serial << PMS ("Press H to Home ") << endl;
	*p_serial << PMS ("Press R to Run Hockey") << endl;
	*p_serial << PMS ("Press S to Stop Slider") << endl;
	*p_serial << PMS ("Press Z to Zero Slider ") << endl;

	
}


void task_user::print_run_menu (void)
{
	*p_serial << PMS ("Hockey is Running") << endl;
	*p_serial << PMS ("Press S to Stop ") << endl;
}

//-------------------------------------------------------------------------------------
/** This method displays information about the status of the system, including the
 *  following: 
 *    \li The name and version of the program
 *    \li The name, status, priority, and free stack space of each task
 *    \li Processor cycles used by each task
 *    \li Amount of heap space free and setting of RTOS tick timer
 */
