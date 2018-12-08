//*************************************************************************************
/** \file textqueue.h
 *    This file contains a class which interacts with a FreeRTOS character queue to
 *    make writing things to a character queue very easy. 
 *
 *  Revised:
 *    \li 10-21-2012 JRR Original file
 *    \li 08-26-2014 JRR Changed file names and base task class name to TaskBase
 *
 *  License:
 *		This file is copyright 2014 by JR Ridgely and released under the Lesser GNU 
 *		Public License, version 2. It intended for educational use only, but its use
 *		is not limited thereto. */
/*		THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 *		AND	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * 		IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * 		ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
 * 		LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUEN-
 * 		TIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
 * 		OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
 * 		CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
 * 		OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * 		OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */
//*************************************************************************************

// This define prevents this .h file from being included more than once in a .cpp file
#ifndef _TEXT_QUEUE_H_
#define _TEXT_QUEUE_H_

#include "FreeRTOS.h"                       // Main header for FreeRTOS
#include "queue.h"                          // Header for FreeRTOS queues
#include "emstream.h"                       // Pull in the base class header file
#include "baseshare.h"                      // Base class for thread-safe shared data


//-------------------------------------------------------------------------------------
/** @brief   Converts data to characters with @c << and puts them into a FreeRTOS 
 *           queue. 
 *  @details This class uses the @c emstream operator @c << to print variables as text
 *           strings, then puts the text into a queue.  The queue conveys the 
 *           characters to another task which does something useful with those 
 *           characters, for example writing them to a serial port or saving them to 
 *           an SD card in a text file. The syntax used for writing information to this
 *           queue uses the overloaded shift operator @c << in the same style as the 
 *           C++ standard library's @c iostream objects such as @c cout. 
 * 
 *  \section Usage
 *  In the file which contains @c main() we create a pointer to a @c TextQueue 
 *  object and use the @c new operator to create the queue itself. (This can be done
 *  in another source file, but putting the code in the main file is usually most 
 *  convenient.) In this example, a serial port object named @c ser_port has previously
 *  been created:
 *  @code
 *  TextQueue* p_text_queue;
 *  ...
 *  main ()
 *  {
 *      ...
 *      p_text_queue = new TextQueue (50, &ser_port);
 *      ...
 *  }
 *  \endcode
 *  In a header file which is read by all the source files in the project we re-declare
 *  the queue pointer with the keyword @c extern to make it globally accessible in all
 *  files that @c \#include this header file:
 *  @code
 *  extern TextQueue* p_text_queue;
 *  \endcode
 *  In the sending task, data is put into the queue. Here, we use the Program Memory
 *  String macro @c PMS() to tell the compiler to read a string constant directly
 *  from program memory in order to save RAM; for information about @c PMS() see
 *  documentation for @c emstream.h:
 *  @code
 *  uint16_t a_data_item;
 *  ...
 *  *p_my_queue << PMS ("The data is: ") << a_data_item << endl;
 *  \endcode
 *  In the receiving task, data is read from the queue and then written to some serial
 *  device. In this case, the data will be written to an SD card, assuming that an 
 *  object called @c my_card of class @c sd_card has previously been created:
 *  @code
 *  if (p_text_queue->check_for_char ())
 *  {
 *      my_card->putchar (p_text_queue->getchar ());
 *  }
 *  \endcode
 *  The reason that the data was not directly written to the SD card in the sending
 *  task is timing: in this example, we assume that the sending task takes data at
 *  regular intervals. Writing data to an SD card, however, takes varying amounts of
 *  time, and a long SD card write could block the taking of data for several 
 *  milliseconds (very bad!) if the data were not being taken in another task and
 *  buffered in a queue. 
 */

class TextQueue : public emstream, public BaseShare
{
	// This protected data can only be accessed from this class or its descendents
	protected:
		QueueHandle_t the_queue;            ///< The handle for the queue we use
		TickType_t ticks_to_wait;           ///< RTOS ticks to wait for empty queue
		emstream* p_serial;                 ///< Serial device used for debugging
		uint16_t buf_size;                  ///< Size of queue buffer in bytes

	// Public methods can be called from anywhere in the program where there is a 
	// pointer or reference to an object of this class
	public:
		// The constructor creates a FreeRTOS queue with a fancy wrapper
		TextQueue (uint16_t size, const char* p_name, emstream* = NULL, 
				   TickType_t = portMAX_DELAY);

		void putchar (char);                // Write one character to the queue
		bool check_for_char (void);         // Check if a character is in the queue
		char getchar (void);                // Read a character from the queue

		/** This overloaded boolean operator allows one to check if the queue has any
		 *  contents which can be read by just checking if the queue is true. It might
		 *  not be the most intuitive method to use, but it sure is convenient. 
		 */
		operator bool () const
		{
			return (uxQueueMessagesWaiting (the_queue) != 0);
		}

		/** If somebody wants to do something which FreeRTOS queues can do but this
		 *  class doesn't support, a handle for the queue inside this class can be
		 *  used to access the queue directly. This isn't normally done.
		 *  @return A handle to the FreeRTOS queue structure which is wrapped up by
		 *          this handy C++ class
		 */
		QueueHandle_t get_handle (void)
		{
			return (the_queue);
		}

		// Print the status of this queue in the table of queue status printouts
		void print_in_list (emstream* p_ser_dev);
};

#endif  // _TEXT_QUEUE_H_
