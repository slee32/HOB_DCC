//*************************************************************************************
/** \file taskqueue.h
 *    This file contains a very simple wrapper class for the FreeRTOS queue. It makes
 *    using the queue just a little bit easier in C++ than it is in C. 
 *
 *  Revised:
 *    \li 10-21-2012 JRR Original file
 *    \li 08-26-2014 JRR Changed file names and queue class name to Queue
 *
 *  License:
 *		This file is copyright 2012 by JR Ridgely and released under the Lesser GNU 
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
#ifndef _TASKQUEUE_H_
#define _TASKQUEUE_H_

#include <string.h>                         // Standard C string handling stuff
#include "FreeRTOS.h"                       // Main header for FreeRTOS
#include "task.h"                           // Header for FreeRTOS tasks
#include "queue.h"                          // Header for FreeRTOS queues
#include "emstream.h"                       // Pull in the base class header file
#include "baseshare.h"                      // Header for thread-safe data items


//-------------------------------------------------------------------------------------
/** @brief   Implements a queue to transmit data from one RTOS task to another. 
 *  @details Since multithreaded tasks must not use unprotected shared data items 
 *  for communication, queues are a primary means of intertask communication. Other
 *  means include shared data items (see taskshare.h) and carrier pigeons. 
 *  The use of a C++ class template allows the compiler to check that you're putting 
 *  the right type of data into each queue and getting the right type of data out, 
 *  thus helping to prevent programming mistakes that can corrupt your data. 
 * 
 *  As a template class, <tt>TaskQueue<dataType></tt> can be used to make queues which 
 *  hold data of many types. "Plain Old Data" types such as @c bool or @c uint16_t are 
 *  supported, of course. But you can also use queues which hold compound data types. 
 *  For example, if you have <tt>class my_data</tt> which holds several measurements 
 *  together in an object, you can make a queue for @c my_data objects with 
 *  <tt>TaskQueue<my_data></tt>.  Each item in the queue will then hold several 
 *  measurements. 
 *  The size of FreeRTOS queues is limited to 255 items in 8-bit microcontrollers
 *  whose @c portBASE_TYPE is an 8-bit number. This is a FreeRTOS feature. 
 * 
 *  Normal writing and reading are done with methods @c put() and @c get(). 
 *  Normal writing means that the sending task must wait until there is empty space in
 *  the queue, and then it puts a data item into the "back" of the queue, where "back"
 *  means that the item in the back of the queue will be read after all items that were
 *  previously put into the queue have been read. Normal reading means that when an 
 *  item is read from the front of the queue, it will then be removed, making space for
 *  more items at the back. This process is often used to synchronize tasks, as the
 *  reading task's @c get() method blocks, meaning that the reading task gets
 *  stuck waiting for an item to arrive in the queue; it won't do anything useful until
 *  new data has been read. Note that this is acceptable behavior in an RTOS because
 *  the RTOS scheduler will ensure that other tasks get to run even while the reading
 *  task is blocking itself waiting for data. 
 * 
 *  In some cases, one may need to use less normal reading and writing methods. Methods
 *  whose name begins with @c ISR_ are to be used only within a hardware interrupt
 *  service routine. If one needs to put data at the front of the queue instead of the
 *  back, use @c butt_in() instead of @c put(). If one needs to read data from the
 *  queue without removing that data, the @c look_at() method allows this to be done. 
 *  If something particularly unusual needs to be done with the queue, one can use the
 *  method @c get_handle() to retrieve the handle used by the C language functions in
 *  FreeRTOS to access the TaskQueue object's underlying data structure directly. 
 * 
 *  @section Usage
 *  The following bits of code show how to set up and use a queue to transfer data of
 *  type @c uint16_t 
 *  from one hypothetical task called @c task_A to another called @c task_B.
 *  
 * In the file which contains @c main() we create a pointer to a queue and use the 
 * @c new operator to create the queue itself. The constructor of the 
 * @c TaskQueue<uint16_t> class is given the number of items in the queue (10) and 
 * a name which will appear in some diagnostic printouts. 
 * A pointer to a serial port object named @c serial_port that has previously been 
 * created can optionally follow the name: 
 *  @code 
 *  TaskQueue<uint16_t>* p_my_queue;
 *  ...
 *  main ()
 *  {
 *      ...
 *      p_my_queue = new TaskQueue<uint16_t> (10, "Data1");
 *  }
 *  @endcode
 *  In a header file which is read by all the source files in the project we re-declare
 *  the queue pointer with the keyword @c extern to make it globally accessible in all
 *  files that @c \#include this header file:
 *  @code
 *  extern TaskQueue<uint16_t>* p_my_queue;
 *  \endcode
 *  In the sending task, data is put into the queue:
 *  @code
 *  uint16_t a_data_item;
 *  ...
 *  p_my_queue->put (a_data_item);
 *  \endcode
 *  In the receiving task, data is read from the queue. In typical usage, the call to
 *  @c get_out() will block the receiving task until data is put into the queue by the
 *  sending task:
 *  @code
 *  uint16_t got_data;
 *  ...
 *  got_data = p_my_queue->get ();
 *  \endcode
 */

template <class dataType> class TaskQueue : public BaseShare
{
	// This protected data can only be accessed from this class or its descendents
	protected:
		QueueHandle_t handle;               ///< The handle for the queue we use
		TickType_t ticks_to_wait;           ///< RTOS ticks to wait for empty queue
		emstream* p_serial;                 ///< Serial device for debugging info.
		uint16_t buf_size;                  ///< Size of queue buffer in bytes

	// Public methods can be called from anywhere in the program where there is a 
	// pointer or reference to an object of this class
	public:
		// The constructor creates a FreeRTOS queue
		TaskQueue (BaseType_t queue_size, const char* p_name, emstream* = NULL, 
				   TickType_t = portMAX_DELAY);

		/** @brief   Put an item into the queue behind other items.
		 *  @details This method puts an item of data into the back of the queue, 
		 *           which is the normal way to put something into a queue. If 
		 *           you want to be rude and put an item into the front of the 
		 *           queue so it will be retreived first, use @c butt_in() instead.
		 *           This method must \b not be used within an ISR.
		 *  @param   item Reference to the item which is going to be put into the queue
		 *  @return  True if the item was successfully queued, false if not
		 */
		bool put (const dataType& item)
		{
			return ((bool)(xQueueSendToBack (handle, &item, ticks_to_wait)));
		}

		/*  This method puts an item of data into the back of the queue from within an
		 *  interrupt service routine. It must not be used within non-ISR code. 
		 */
		bool ISR_put (const dataType& item);

		/** @brief   Put an item into the front of the queue to be retrieved first.
		 *  @details This method puts an item into the front of the queue so that it 
		 *           will be received first as long as nothing else is put in front 
		 *           of it. This is not the normal way to put things into a queue; 
		 *           using @c put() to put items into the back of the queue is. If you
		 *           always use this method, you're really making a stack rather than 
		 *           a queue, you weirdo. This method must @b not be used within an 
		 *           interrupt service routine. 
		 *  @param   item Reference to the item which is going to be (rudely) put into
		 *                the front of the queue
		 *  @return  True if the item was successfully queued, false if not
		 */
		bool butt_in (const dataType& item)
		{
			return ((bool)(xQueueSendToFront (handle, &item, ticks_to_wait)));
		}

		/*  This method puts an item into the front of the queue from within an ISR.
		 *  It must not be used within normal, non-ISR code. 
		 */
		bool ISR_butt_in (const dataType& item);

		/** @brief   Return true if the queue is empty.
		 *  @details This method checks if the queue is empty. It returns @c true if
		 *           there are no items in the queue and @c false if there are items.
		 *  @return  @c true if the queue is empty, @c false if it's not empty
		 */
		bool is_empty (void)
		{
			return (uxQueueMessagesWaiting (handle) == 0);
		}

		/** @brief   Return true if the queue is empty, from within an ISR.
		 *  @details This method checks if the queue is empty from within an interrupt
		 *           service routine. It must not be used in normal non-ISR code. 
		 *  @return  @c true if the queue is empty, @c false if it's not empty
		 */
		bool ISR_is_empty (void)
		{
			return (uxQueueMessagesWaitingFromISR (handle) == 0);
		}

		// Get an item from the queue
		dataType get (void);

		// Get an item from the queue from within an interrupt service routine
		dataType ISR_get (void);

		// Look at the first item in the queue but don't remove it
		dataType look_at (void);

		// Look at the first item in the queue from within an interrupt service routine
		dataType ISR_look_at (void);

		/** @brief   Return true if the queue has contents which can be read.
		 *  @details This method allows one to check if the queue has any contents. 
		 *           It must @b not be called from within an interrupt service routine.
		 *  @return  @c true if there's something in the queue, @c false if not
		 */
		bool not_empty (void)
		{
			return (uxQueueMessagesWaiting (handle) != 0);
		}

		/** @brief   Return true if the queue has items in it, from within an ISR.
		 *  @details This method allows one to check if the queue has any contents 
		 *           from within an interrupt service routine. It must @b not be
		 *           used from within normal, non-ISR code. 
		 *  @return  @c true if there's something in the queue, @c false if not
		 */
		bool ISR_not_empty (void)
		{
			return (uxQueueMessagesWaitingFromISR (handle) != 0);
		}

		/** @brief   Return the number of items in the queue.
		 *  @details This method returns the number of items waiting in the queue. 
		 *           It must @b not be called from within an interrupt service 
		 *           routine; the method @c ISR_num_items_in() can be called from
		 *           within an ISR. 
		 *  @return  The number of items in the queue
		 */
		unsigned portBASE_TYPE num_items_in (void)
		{
			return (uxQueueMessagesWaiting (handle));
		}

		/** @brief   Return the number of items in the queue, to an ISR.
		 *  @details This method returns the number of items waiting in the queue; 
		 *           it must be called only from within an interrupt service routine.
		 *  @return  The number of items in the queue
		 */
		unsigned portBASE_TYPE ISR_num_items_in (void)
		{
			return (uxQueueMessagesWaitingFromISR (handle));
		}

		/** @brief   Print the queue's status to a serial device.
		 *  @details This method makes a printout of the queue's status on the given
		 *           serial device, then calls this same method for the next item of 
		 *           thread-safe data in the linked list of items. 
		 *  @param   p_ser_dev Pointer to the serial device on which to print
		 */
		void print_in_list (emstream* p_ser_dev);

		/** @brief   Return a handle to the FreeRTOS structure which runs this queue.
		 *  @details If somebody wants to do something which FreeRTOS queues can do 
		 *           but this class doesn't support, a handle for the queue wrapped by
		 *           this class can be used to access the queue directly. This isn't 
		 *           normally done.
		 *  @return  The handle of the FreeRTOS queue which is wrapped within this C++
		 *           class
		 */
		QueueHandle_t get_handle (void)
		{
			return (handle);
		}
}; // class TaskQueue 


//-------------------------------------------------------------------------------------
/** @brief   Construct a queue object, allocating memory for the buffer.
 *  @details This constructor creates the FreeRTOS queue which is wrapped by the 
 *           @c TaskQueue class. 
 *  @param   queue_size The number of characters which can be stored in the queue
 *  @param   p_name A name to be shown in the list of task shares (default @c NULL)
 *  @param   p_ser_dev Pointer to a serial device to be used for debugging printouts
 *                     Default: @c NULL
 *  @param   wait_time How long, in RTOS ticks, to wait for a full queue to become
 *                     empty before a character can be sent. Default: @c portMAX_DELAY
 *                     which causes the sending task to block until sending occurs.
 */

template <class dataType>
TaskQueue<dataType>::TaskQueue (BaseType_t queue_size, const char* p_name, 
								emstream* p_ser_dev, TickType_t wait_time)
	: BaseShare (p_name)
{
	// Create a FreeRTOS queue object with space for the data items
	handle = xQueueCreate (queue_size, sizeof (dataType));

	// Store the wait time; it will be used when writing to the queue
	ticks_to_wait = wait_time;

	// Save the pointer to a serial devide so it may be used for debugging
	p_serial = p_ser_dev;

	// Save the buffer size
	buf_size = queue_size;

	// If the serial device isn't NULL, print a failure message if needed
	if (handle == 0)
	{
		DBG (p_serial, PMS ("ERROR creating ") << queue_size << PMS("x") 
			 << sizeof (dataType) << PMS ("B queue #") << handle << endl);
	}
}


//-------------------------------------------------------------------------------------
/** @brief   Return and remove the item at the head of the queue.
 *  @details This method returns the item at the head of the queue and removes that 
 *           item from the queue. If there's nothing in the queue, this method waits, 
 *           blocking the calling task, for the number of RTOS ticks specified in the
 *           @c wait_time parameter to the queue constructor (the default is forever)
 *           or until something shows up. 
 *  @return  The data retrieved from the queue
 */

template <class dataType>
inline dataType TaskQueue<dataType>::get (void)
{
	dataType recv_item;                     // Data item read from the queue

	// If xQueueReceive doesn't return pdTrue, nothing was found in the queue, so
	// we'll return the item as created by its default constructor
	xQueueReceive (handle, &recv_item, portMAX_DELAY);
	return (recv_item);
}


//-------------------------------------------------------------------------------------
/** @brief   Return and remove the item at the head of the queue from within an ISR.
 *  @details This method removes and returns the item at the head of the queue from 
 *           within an interrupt service routine. This method must @b not be called 
 *           from within normal non-ISR code. 
 *  @return  The data retrieved from the queue
 */

template <class dataType>
inline dataType TaskQueue<dataType>::ISR_get (void)
{
	dataType recv_item;                     // Data item read from the queue
	portBASE_TYPE task_awakened;            // Checks if a context switch is needed

	// If xQueueReceive doesn't return pdTrue, nothing was found in the queue, so
	// we'll return the item as created by its default constructor
	xQueueReceiveFromISR (handle, &recv_item, &task_awakened);
	return (recv_item);
}


//-------------------------------------------------------------------------------------
/** @brief   Return the item at the queue head without removing it.
 *  @details This method returns the item at the head of the queue without removing 
 *           that item from the queue. If there's nothing in the queue, this method 
 *           waits, blocking the calling task, for for the number of RTOS ticks 
 *           specified in the @c wait_time parameter to the queue constructor (the 
 *           default is forever) or until something shows up. This method must \b not 
 *           be called from within an interrupt service routine. 
 *  @return  The data retrieved from the queue; the data is set to 0 in the correct
 *           data type if we couldn't get any data from the queue and the attempt
 *           timed out
 */

template <class dataType>
inline dataType TaskQueue<dataType>::look_at (void)
{
	dataType recv_item;						// Data item read from the queue

	// If xQueueReceive doesn't return pdTrue, nothing was found in the queue, so
	// we'll return the item as created by its default constructor
	xQueuePeek (handle, &recv_item, portMAX_DELAY);
	return (recv_item);
}


//-------------------------------------------------------------------------------------
/** @brief   Put an item into the queue from within an ISR.
 *  @details This method puts an item of data into the back of the queue from within 
 *           an interrupt service routine. It must \b not be used within non-ISR code. 
 *  @param   item Reference to the item which is going to be put into the queue
 *  @return  True if the item was successfully queued, false if not
 */

template <class dataType>
inline bool TaskQueue<dataType>::ISR_put (const dataType& item)
{
	// This value is set to true if a context switch should occur due to this data
	signed portBASE_TYPE shouldSwitch = pdFALSE;

	bool return_value;						// Value returned from this method

	// Call the FreeRTOS function and save its return value
	return_value = (bool)(xQueueSendToBackFromISR (handle, &item, &shouldSwitch));

	// If the queue said that putting something into it has un-blocked a higher
	// priority task than the one currently running, ask for a context switch
	// BUG: There doesn't seem to be a taskYIELD_FROM_ISR() for the AVR port
// 	if (shouldSwitch)
// 	{
// 		taskYIELD_FROM_ISR ();
// 	}

	// Return the return value saved from the call to xQueueSendToBackFromISR()
	return (return_value);
}



//-------------------------------------------------------------------------------------
/** @brief   Put an item into the front of the queue from within an ISR.
 *  @details This method puts an item into the front of the queue from within an ISR.
 *           It must \b not be used within normal, non-ISR code. 
 *  @param   item The item which is going to be (rudely) put into the front of the 
 *                queue
 *  @return  True if the item was successfully queued, false if not
 */

template <class dataType>
bool TaskQueue<dataType>::ISR_butt_in (const dataType& item)
{
	// This value is set to true if a context switch should occur due to this data
	signed portBASE_TYPE shouldSwitch = pdFALSE;

	bool return_value;						// Value returned from this method

	// Call the FreeRTOS function and save its return value
	return_value = (bool)(xQueueSendToFrontFromISR (handle, &item, &shouldSwitch));

	// If the queue said that putting something into it has un-blocked a higher
	// priority task than the one currently running, ask for a context switch
	// BUG: There doesn't seem to be a taskYIELD_FROM_ISR() for the AVR port
// 	if (shouldSwitch)
// 	{
// 		taskYIELD_FROM_ISR ();
// 	}

	// Return the return value saved from the call to xQueueSendToBackFromISR()
	return (return_value);
}


//-------------------------------------------------------------------------------------
/** @brief   Return the item at the front of the queue without deleting it, from within
 *           an ISR.
 *  @details This method returns the item at the head of the queue without removing 
 *           that item from the queue. If there's nothing in the queue, this method 
 *           returns the result of the default constructor for the data item, usually
 *           zero in the given data type. This method must \b not be called from within
 *           an interrupt service routine. 
 *  @return  The data retrieved from the queue; the data is set to 0 in the correct
 *           data type if we couldn't get any data from the queue
 */

template <class dataType>
inline dataType TaskQueue<dataType>::ISR_look_at (void)
{
	dataType recv_item;						// Data item read from the queue
	portBASE_TYPE task_awakened;				// Checks if a task will wake up

	// If xQueueReceive doesn't return pdTrue, nothing was found in the queue, so
	// we'll return the item as created by its default constructor
	xQueuePeekFromISR (handle, &recv_item, &task_awakened);

	return (recv_item);
}


//-------------------------------------------------------------------------------------
/** @brief   Print the queue's status to a serial device.
 *  @details This method makes a printout of the queue's status on the given serial 
 *           device, then calls this same method for the next item of thread-safe data
 *           in the linked list of items. 
 *  @param   p_ser_dev Pointer to the serial device on which to print
 */
template <class dataType>
void TaskQueue<dataType>::print_in_list (emstream* p_ser_dev)
{
	// Print this task's name and pad it to 16 characters
	*p_ser_dev << name;
	for (uint8_t cols = strlen (name); cols < 16; cols++)
	{
		p_ser_dev->putchar (' ');
	}

	// Print the type
	p_ser_dev->puts ("queue\t");

	// Print the free and total number of spaces in the queue
	*p_ser_dev << uxQueueSpacesAvailable (handle) << '/' << buf_size << '\t';

	// End the line
	*p_ser_dev << endl;

	// Call the next item
	if (p_next != NULL)
	{
		p_next->print_in_list (p_ser_dev);
	}
}


#endif  // _TASKQUEUE_H_
