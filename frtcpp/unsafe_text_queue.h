//*************************************************************************************
/** \file unsafe_text_queue.h
 *    This file contains a class which writes characters into a simple circular buffer
 *    (that is, a thread \b unsafe queue) using the operators of class \c emstream to
 *    make the formatting really easy. There is no protection from interruption, thus
 *    the name. 
 *
 *  Revised:
 *    \li 10-21-2012 JRR Original file, class \c frt_text_queue
 *    \li 12-16-2012 JRR Made into the unsafe text queue to use for testing
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

// This define prevents this .h file from being included more than once in a .cpp file
#ifndef _UNSAFE_TEXT_QUEUE_H_
#define _UNSAFE_TEXT_QUEUE_H_

#include "FreeRTOS.h"                       // Main header for FreeRTOS
#include "circ_buffer.h"                    // Header for simple circular buffers
#include "emstream.h"                       // Pull in the base class header file


//-------------------------------------------------------------------------------------
/** @brief   A simple, @b non-thread-safe queue for formatting text within one task. 
 *  @details This class uses the \c emstream structure and its \c "<<" operator to 
 *           convert data to characters and write the characters into a simple circular
 *           buffer. This queue is \b not thread safe and must \b not be used for 
 *           communication between tasks; it may only be used to buffer text which is 
 *           used within the same task in which it is created. The syntax used for 
 *           writing information to this buffer uses the overloaded shift operator 
 *           \c "<<" in the same style as the C++ standard library's \c iostream 
 *           objects such as \c cout. 
 * 
 *  \section Usage
 *  In some task we create a pointer to a \c unsafe_text_queue object and use the 
 *  \c new operator to create the queue itself:
 *  \code
 *  unsafe_text_queue* p_my_queue;
 *  ...
 *  main ()
 *  {
 *      ...
 *      p_my_queue = new unsafe_text_queue (50);
 *      ...
 *  }
 *  \endcode
 *  Data is put into the queue in the example below. Here, we use the Program Memory
 *  String macro \c PMS() to tell the compiler to read a string constant directly
 *  from program memory in order to save RAM; for information about \c PMS() see
 *  documentation for \c emstream.h:
 *  \code
 *  uint16_t a_data_item;
 *  ...
 *  *p_my_queue << PMS ("The data is: ") << a_data_item << endl;
 *  \endcode
 *  Data can then read from the queue and then written to some serial device. In this 
 *  case, the data will be written to an SD card, assuming that an object called 
 *  \c my_card of class \c sd_card has previously been created. Note that we would not
 *  actually do this because the class \c sd_card is already a descendent of class
 *  \c emstream, so and SD card driver can format things with the \c << operator and
 *  doesn't need that kind of help. It's just an example: 
 *  \code
 *  if (p_my_queue->check_for_char ())
 *  {
 *      my_card->putchar (p_my_queue->getchar ());
 *  }
 *  \endcode
 */

template <size_t size>
class unsafe_text_queue : public emstream
{
	// This protected data can only be accessed from this class or its descendents
	protected:
		/// This pointer points to a serial device used to print debugging messages.
		emstream* p_serial;

		/// This circular buffer holds the characters being "printed" by this object.
		circ_buffer<char, size>* p_buffer;

		/// The number of RTOS ticks to wait on a full buffer before giving up.
		portTickType ticks_to_wait;

	// Public methods can be called from anywhere in the program where there is a 
	// pointer or reference to an object of this class
	public:
		// The constructor creates a circular buffer to hold the data
		unsafe_text_queue (emstream* = NULL, portTickType = portMAX_DELAY);

		bool putchar (char);					// Write one character to the queue
		bool check_for_char (void);			// Check if a character is in the queue
		int16_t getchar (void);				// Read a character from the queue

		/** This overloaded boolean operator allows one to check if the queue has any
		 *  contents which can be read by just checking if the queue is true. It might
		 *  not be the most intuitive method to use, but it sure is convenient. 
		 */
		operator bool () const
		{
			return (!(p_buffer->is_empty ()));
		}

		/** This method returns the character in the buffer at the given index.
		 *  @return Character number \c index in the buffer
		 */
		char operator[] (size_t index)
		{
			return ((*p_buffer)[index]);
		}
};


//-------------------------------------------------------------------------------------
/** This constructor creates a \c unsafe_text_queue object. This object can be used to
 *  make up character strings inside one task. It is \b not thread safe, so it must
 *  never be used to communicate things between tasks. The queue size should be large 
 *  enough so that if the writing task gets ahead of the reading task for a little 
 *  while, there's room in the queue to hold the extra characters; if not, writing to 
 *  the queue will block until the reading task has read characters from the queue and 
 *  done something with them. 
 *  @param p_ser_dev A pointer which points to a serial device which can be used for
 *                   diagnostic logging or printing
 *  @param a_wait_time How long, in RTOS ticks, to wait for a full queue to become
 *                     empty before a character can be sent. The default value of
 *                     portMAX_DELAY causes a send to block indefinitely
 */

template <size_t size>
unsafe_text_queue<size>::unsafe_text_queue (emstream* p_ser_dev,
										   portTickType a_wait_time)
{
	// Save the pointer to the serial device which is used for debugging
	p_serial = p_ser_dev;

	// Create a FreeRTOS queue object which holds the given number of characters
	p_buffer = new circ_buffer<char, size>;

	// Save the number of RTOS ticks to wait on a full buffer
	ticks_to_wait = a_wait_time;

	// Print an error message, if needed, and if debugging is enabled
	#ifdef SERIAL_DEBUG
		if (p_buffer == NULL)
		{
			DBG (p_serial, "Cannot allocate memory for unsafe_text_queue" << endl);
		}
	#endif

	// Store the wait time; it will be used when writing to the queue
	ticks_to_wait = a_wait_time;
}


//-------------------------------------------------------------------------------------
/** This method writes one character to the queue. If the second constructor parameter
 *  wasn't given, the write operation will block until there is room in the queue for
 *  the character being written. Otherwise, the write will block for the given number
 *  of RTOS ticks waiting for an empty space in the queue, then if the queue has not
 *  become empty, give up in frustration and return false. 
 *  @param a_char The character to be sent to the queue
 *  @return True if the character was successfully sent, false if something went wrong
 */

template <size_t size>
bool unsafe_text_queue<size>::putchar (char a_char)
{
	for (portTickType tick_count = ticks_to_wait; tick_count > 0; tick_count--)
	{
		// If the circular buffer's put() method returns true, the character was
		// successfully placed in the buffer and we're done
		if (p_buffer->put (a_char))
		{
			return (true);
		}
	}

	// If we get here, the loop timed out and data wasn't saved
	return (false);
}


// //-------------------------------------------------------------------------------------
// /** This method writes a string to the queue. It just calls putchar() lots of times
//  *  until a null character ('\0') is encountered. 
//  */
// 
// template <size_t size>
// inline void unsafe_text_queue<size>::puts (char const* p_str)
// {
// 	while (*p_str)
// 	{
// 		putchar (*p_str++);
// 	}
// }


//-------------------------------------------------------------------------------------
/** This method checks if there is a character in the queue. It just asks the circular
 *  buffer if it's empty and returns the opposite. 
 *  @return True for character available, false for no character available
 */

template <size_t size>
inline bool unsafe_text_queue<size>::check_for_char (void)
{
	return (!(p_buffer->is_empty ()));
}


//-------------------------------------------------------------------------------------
/** This method reads a character from the queue. If there isn't one conveniently in
 *  the queue, it blocks until a character is received.
 *  @return The character which was received from the queue
 */

template <size_t size>
inline int16_t unsafe_text_queue<size>::getchar (void)
{
	return (p_buffer->get ());
}

#endif  // _UNSAFE_TEXT_QUEUE_H_
