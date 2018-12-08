//*************************************************************************************
/** \file textqueue.cpp
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

#include <string.h>                         // C language string handling functions
#include "textqueue.h"                      // Pull in the base class header file


//-------------------------------------------------------------------------------------
/** @brief   Constructor for a queue which transfers text characters.
 *  @details This constructor creates a @c TextQueue object. The queue should be large
 *           enough so that if the writing task gets ahead of the reading task for a 
 *           little while, there's room in the queue to hold the extra characters; if 
 *           not, writing to the queue will block until the reading task has read 
 *           characters from the queue and done something with them. 
 *  @param   queue_size The number of characters which can be stored in the queue
 *  @param   p_name A name to be shown in the list of task shares (default @c NULL)
 *  @param   a_wait_time How long, in RTOS ticks, to wait for a full queue to become
 *                       empty before a character can be sent. The default value of
 *                       portMAX_DELAY causes a send to block indefinitely
 *  @param   p_ser_dev A pointer which points to a serial device which can be used for
 *                     diagnostic logging or printing
 */

TextQueue::TextQueue (uint16_t queue_size, const char* p_name, emstream* p_ser_dev,
					  TickType_t a_wait_time)
	: emstream (), BaseShare (p_name)
{
	// Save the pointer to the serial device which is used for debugging
	p_serial = p_ser_dev;

	// Create a FreeRTOS queue object which holds the given number of characters
	the_queue = xQueueCreate (queue_size, sizeof (char));

	// Store the wait time; it will be used when writing to the queue
	ticks_to_wait = a_wait_time;

	// Save the buffer size
	buf_size = queue_size;
}


//-------------------------------------------------------------------------------------
/** @brief   Write one character to the text queue.
 *  @details This method writes one character to the queue. If the second constructor 
 *           parameter isn't given, the write operation will block until there is room
 *           in the queue for the character being written. Otherwise, the write will 
 *           block for the given number of RTOS ticks waiting for an empty space in 
 *           the queue, then if the queue has not become empty, give up in frustration
 *           and return. 
 *  @param   a_char The character to be sent to the queue
 */

inline void TextQueue::putchar (char a_char)
{
	xQueueSendToBack (the_queue, &a_char, ticks_to_wait);
}


//-------------------------------------------------------------------------------------
/** @brief   Check if a character is ready to be read from the queue.
 *  @details This method checks if there is a character in the queue. It just calls 
 *           the FreeRTOS function @c uxQueueMessagesWaiting(); if there's anything in
 *           the queue, the return value from that function will be greater than zero. 
 *  @return  True for character available, false for no character available
 */

inline bool TextQueue::check_for_char (void)
{
	return (uxQueueMessagesWaiting (the_queue));
}


//-------------------------------------------------------------------------------------
/** @brief   Read one character from the queue.
 *  @details This method reads a character from the queue and returns it. If there 
 *           isn't one conveniently in the queue, it blocks until a character is 
 *           received. This blocking can be helpful in task synchronization; the 
 *           receiving task will not waste processor time until a character shows up.
 *  @return  The character which was received from the queue
 */

inline char TextQueue::getchar (void)
{
	char recv_char;							// Character read from the queue

	// If xQueueReceive doesn't return pdTrue, nothing was found in the queue
	if (xQueueReceive (the_queue, &recv_char, portMAX_DELAY) != pdTRUE)
	{
		return (-1);
	}

	// OK, we got good data from the queue, so return it
	return (recv_char);
}


//-------------------------------------------------------------------------------------
/** @brief   Print the status of the queue.
 *  @details This method writes the status of the text queue to the given serial 
 *           device. 
 *  @param   p_ser_dev Pointer to a serial device on which to print the status
 */

void TextQueue::print_in_list (emstream* p_ser_dev)
{
	// Print this task's name and pad it to 16 characters
	*p_ser_dev << name;
	for (uint8_t cols = strlen (name); cols < 16; cols++)
	{
		p_ser_dev->putchar (' ');
	}

	// Print the type
	p_ser_dev->puts ("txt_q\t");

	// Print the free and total number of spaces in the queue
	*p_ser_dev << uxQueueSpacesAvailable (the_queue) << '/' << buf_size << '\t';

	// End the line
	*p_ser_dev << endl;

	// Call the next item
	if (p_next != NULL)
	{
		p_next->print_in_list (p_ser_dev);
	}
}
