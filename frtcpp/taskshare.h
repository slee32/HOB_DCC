//*************************************************************************************
/** @file    taskshare.h
 *  @brief   Type-safe data which can be shared between tasks in a thread-safe manner.
 *  @details This file contains a template class for data which is to be shared between
 *           classes. The data must be protected against damage due to context 
 *           switches, so it is protected by a mutex or by causing transfers to take 
 *           place inside critical sections which are protected from being interrupted.
 *
 *  Revised:
 *    \li 10-29-2012 JRR Original file
 *    \li 08-26-2014 JRR Changed file names, class name to @c TaskShare, removed unused
 *                       version that uses semaphores, renamed @c put() and @c get()
 *    \li 10-18-2014 JRR Added linked list of all shares for tracking and debugging
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
#ifndef _TASKSHARE_H_
#define _TASKSHARE_H_

#include <string.h>                         // C language string handling functions
#include "FreeRTOS.h"                       // Main header for FreeRTOS
#include "baseshare.h"                      // Base class for shared data items


//-------------------------------------------------------------------------------------
/** @brief   Class for data to be shared in a thread-safe manner between tasks.
 *  @details This class implements an item of data which can be shared between tasks
 *           without risk of data corruption associated with global variables. Unlike
 *           queues, shares do not use a buffer for many data items; there is only a
 *           one-item buffer in which the most recent value of the data is kept. 
 *           Shares therefore do not provide the task synchronization or incur the 
 *           overhead associated with queues. 
 * 
 *           The data is protected by using critical code sections (see the FreeRTOS 
 *           documentation of @c portENTER_CRITICAL() ) so that tasks can't interrupt 
 *           each other when reading or writing the data is taking place. This prevents
 *           data corruption due to thread switching. The C++ template mechanism is 
 *           used to ensure that only data of the correct type is put into or taken 
 *           from a shared data item. A @c TaskShare<DataType> object keeps its own 
 *           separate copy of the data. This uses some memory, but it is necessary to 
 *           reliably prevent data corruption; it prevents possible side effects from 
 *           causing the sender's copy of the data from being inadvertently changed. 
 * 
 *  @section Usage
 *  The following bits of code show how to set up and use a share to transfer data of
 *  type @c uint16_t 
 *  from one hypothetical task called @c task_A to another called @c task_B.
 *  
 * In the file which contains @c main() we create a pointer to a share and use the 
 * @c new operator to create the share itself. The constructor of the 
 * @c TaskShare<uint16_t> class is given the name of the share; the name will be shown
 * on system diagnostic printouts:
 *  @code 
 *  TaskShare<uint16_t>* p_my_share;
 *  ...
 *  main ()
 *  {
 *      ...
 *      p_my_share = new TaskShare<uint16_t> ("Data_3");
 *  }
 *  @endcode
 *  In a header file which is read by all the source files in the project we re-declare
 *  the share pointer with the keyword @c extern to make it globally accessible in all
 *  files that @c \#include this header file:
 *  @code
 *  extern TaskShare<uint16_t>* p_my_share;
 *  \endcode
 *  In the sending task, data is put into the share:
 *  @code
 *  uint16_t a_data_item;
 *  ...
 *  p_my_share->put (a_data_item);
 *  \endcode
 *  In the receiving task, data is read from the share:
 *  @code
 *  uint16_t got_data;
 *  ...
 *  got_data = p_my_share->get ();
 *  \endcode
 * 
 *  @b Note: In the past, ME405 students have often used task shares to save data
 *  persistently within a task. This is @b not necessary. Just use variables declared 
 *  @c static if the data is only needed within one method. If data needs to be shared 
 *  between member functions of the same task, make such data member data of the task 
 *  class rather than local data within one method. This data will persist as long as 
 *  the owning task object exists, which is generally until the power is turned off. 
 */

template <class DataType> class TaskShare : public BaseShare
{
	protected:
		DataType the_data;					///< Holds the data to be shared

	public:
		/** @brief   Construct a shared data item.
		 *  @details This default constructor for a shared data item doesn't do much
		 *           besides allocate memory because there isn't any particular setup 
		 *           required. Note that the data is @b not initialized. 
		 *  @param   p_name A name to be shown in the list of task shares (default 
		 *           @c NULL)
		 */
		TaskShare<DataType> (const char* p_name) : BaseShare (p_name)
		{
		}

		// This method is used to write data into the shared data item
		void put (DataType);

		// This method is used to write data from within an ISR only
		void ISR_put (DataType);

		// This method is used to read data from the shared data item
		DataType get (void);

		// This method is used to read data from within an ISR only
		DataType ISR_get (void);

		// Print the share's status within a list of all shares' statuses (statae?)
		void print_in_list (emstream* p_ser_dev);

		/**   @brief   The prefix increment causes the shared data to increase by one.
		 *    @details This operator just increases by one the variable held by the 
		 *             shared data item. BUG: It should return a reference to this
		 *             shared data item, but for some reason the compiler insists it
		 *             must return a reference to the data. Why is unknown. 
		 */
		DataType& operator ++ (void)
		{
			portENTER_CRITICAL ();
			the_data++;
			portEXIT_CRITICAL ();

			return (the_data);
		}

		/**   @brief The postfix increment causes the shared data to increase by one.
		 */
		DataType operator ++ (int)
		{
			DataType result = the_data;
			portENTER_CRITICAL ();
			the_data++;
			portEXIT_CRITICAL ();

			return (result);
		}

		/**   @brief   The prefix decrement causes the shared data to decrease by one.
		 *    @details This operator just decreases by one the variable held by the 
		 *             shared data item. BUG: It should return a reference to this
		 *             shared data item, but for some reason the compiler insists it
		 *             must return a reference to the data. Why is unknown. 
		 */
		DataType& operator -- (void)
		{
			portENTER_CRITICAL ();
			the_data--;
			portEXIT_CRITICAL ();

			return (the_data); //// *this);  The BUG
		}

		/**   @brief The postfix decrement causes the shared data to decrease by one.
		 */
		DataType operator -- (int)
		{
			DataType result = the_data;
			portENTER_CRITICAL ();
			the_data--;
			portEXIT_CRITICAL ();

			return (result);
		}
}; // class TaskShare<DataType>


//-------------------------------------------------------------------------------------
/** @brief   Put data into the shared data item.
 *  @details This method is used to write data into the shared data item. It's 
 *           declared @c inline so that instead of a regular function call at the 
 *           assembly language level, <tt>an_object.put (x);</tt> will result in 
 *           the code within this function being inserted directly into the calling
 *           function. This is faster than doing a regular function call, which
 *           involves pushing the program counter on the stack, pushing parameters, 
 *           jumping, making space for local variables, jumping back and popping the 
 *           program counter, yawn, zzz...
 *  @param   new_data The data which is to be written
 */

template <class DataType>
inline void TaskShare<DataType>::put (DataType new_data)
{
	portENTER_CRITICAL ();
	the_data = new_data;
	portEXIT_CRITICAL ();
}


//-------------------------------------------------------------------------------------
/** @brief   Put data into the shared data item from within an ISR.
 *  @details This method writes data from an ISR into the shared data item. It must 
 *           only be called from within a hardware interrupt, not a normal task. This 
 *           is because critical section protection isn't used here, which is OK, 
 *           assuming that an interrupt can't be interrupted by another interrupt, 
 *           which is the case on most small microcontrollers. 
 *  @param   new_data The data which is to be written into the shared data item
 */

template <class DataType>
void TaskShare<DataType>::ISR_put (DataType new_data)
{
	the_data = new_data;
}


//-------------------------------------------------------------------------------------
/** @brief   Read data from the shared data item.
 *  @details This method is used to read data from the shared data item with critical
 *           section protection to ensure that the data cannot be corrupted by a task
 *           switch. 
 *  @return  The current value of the shared data item
 */

template <class DataType>
DataType TaskShare<DataType>::get (void)
{
	// It's necessary to make an extra, temporary copy of the data so that the
	// temporary copy can be returned. We can't call return() from within the
	// critical section for reasons that are obvious if you think about it
	DataType temporary_copy;

	portENTER_CRITICAL ();
	temporary_copy = the_data;
	portEXIT_CRITICAL ();

	return (temporary_copy);
}


//-------------------------------------------------------------------------------------
/** @brief   Read data from the shared data item, from within an ISR.
 *  @details This method is used to enable code within an ISR to read data from the 
 *           shared data item. It must only be called from within an interrupt service
 *           routine, not a normal task. This is because critical section protection 
 *           isn't used here, which is OK, assuming that an interrupt can't be 
 *           interrupted by another interrupt, which is the case on most small 
 *           microcontrollers. 
 *  @return  The current value of the shared data item
 */

template <class DataType>
DataType TaskShare<DataType>::ISR_get (void)
{
	return (the_data);
}


//-------------------------------------------------------------------------------------
/** @brief   Read data from the shared data item, from within an ISR.
 *  @details This method is used to enable code within an ISR to read data from the 
 *           shared data item. It must only be called from within an interrupt service
 *           routine, not a normal task. This is because critical section protection 
 *           isn't used here, which is OK, assuming that an interrupt can't be 
 *           interrupted by another interrupt, which is the case on most small 
 *           microcontrollers. 
 *  @param   p_ser_dev Pointer to a serial device on which to print the status
 */

template <class DataType>
void TaskShare<DataType>::print_in_list (emstream* p_ser_dev)
{
	// Print this task's name and pad it to 16 characters
	*p_ser_dev << name;
	for (uint8_t cols = strlen (name); cols < 16; cols++)
	{
		p_ser_dev->putchar (' ');
	}

	p_ser_dev->puts ("share\t");

	// End the line
	*p_ser_dev << endl;

	// Call the next item
	if (p_next != NULL)
	{
		p_next->print_in_list (p_ser_dev);
	}
}



#endif  // _TASKSHARE_H_
