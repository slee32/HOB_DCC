//*************************************************************************************
/** \file circ_buffer.h
 *    This file implements a circular buffer (also called a queue). Data which is 
 *    placed in the buffer is saved for future reading. This class is implemented as a 
 *    template; this means that a buffer can be configured to store data of almost any
 *    type. NOTE: This is \b not a thread-safe RTOS queue for inter-task communication.
 *    It is designed for use within one thread, or for transmitting data without any
 *    synchronization between an ISR and a non-ISR task, or for use in a cooperative
 *    multitasking environment. 
 *
 *  Usage:
 *    The template specifies the following:
 *      \li The type of data being stored in the buffer is \c qType.
 *      \li The number of data items in the buffer is \c qSize.
 *    The type of data being stored may be simple data types such as characters or 
 *    integers, but it may also be something more complex such as objects of a class 
 *    if there's enough memory to hold them all. For example, a buffer holding 100 
 *    long integers (which would use 400 bytes for the data plus a few more bytes for 
 *    the indices) could be declared as \c circ_buffer<int32_t,\c 100> \c my_buffer. 
 *
 *  Revisions:
 *    \li 08-28-2003 JRR Reference: www.yureu.com/logo_plotter/queue_code.htm
 *    \li 08-28-2003 JRR Ported to C++ from C
 *    \li 12-14-2007 JRR Doxygen comments added
 *    \li 02-17-2008 JRR Changed index type from define to template parameter
 *    \li 08-12-2008 JRR Added overloaded array subscript operator
 *    \li 02-10-2011 JRR Fixed index moving bug in jam()
 *    \li 10-27-2012 JRR Changed name from \c queue to \c circ_buffer for FreeRTOS 
 *                       compatibility because FreeRTOS uses a file called \c queue.c
 *    \li 12-17-2012 JRR Removed fancy index size, replaced with simpler \c size_t
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
#ifndef _CIRC_BUFFER_H_
#define _CIRC_BUFFER_H_


//-------------------------------------------------------------------------------------
/** \brief This class implements a simple first-in, first-out circular buffer.
 *  \details As a template class, it can be used to create a buffer which holds any
 *  type of object whose data is copied with an assignment operator. The size and type 
 *  of object are determined at compile time for efficiency. This is \b not the same 
 *  as an RTOS queue which transmits data from one task to another; a 
 *  \c circ_buffer<qType,\c qSize> has no components to make it thread-safe nor to 
 *  interact with an RTOS scheduler in any way. 
 * 
 *  \section Usage
 *  To create a buffer of this type, declare an object of this type, specifying as
 *  template parameters the type of data in the buffer and the number of items in the 
 *  buffer. As an example, to create a buffer which holds 100 16-bit integers:
 *  \code 
 *  circ_buffer<int16_t, 100> my_queue;
 *  \endcode
 * 
 *  One can also create buffers which hold objects of a class. If one has created a
 *  class called \c data_item whose objects each hold a bunch of data, one can make
 *  a circular buffer which holds a bunch of \c data_item objects:
 *  \code
 *  circ_buffer<data_item, 320> my_BIG_buff;
 *  \endcode 
 *  One must be careful when making buffers not to use too much RAM, as small 
 *  microcontrollers usually don't have much of it. If a \c data_item object has 5
 *  \c uint16_t integers to store A/D readings, for example, the data storage for
 *  \c my_BIG_buff will be over 3KB in size, which is most of the memory of an
 *  ATmega128; not enough memory will be left for task stacks and the RTOS itself. 
 * 
 *  Entering data into the buffer is done with the \c put() method:
 *  \code
 *  uint16_t some_data;
 *  ...
 *  my_queue.put (some_data);
 *  \endcode
 *  The \c put() method will only put data into the buffer if there is empty space for
 *  the data; a full buffer will not allow data to be written into it unless the 
 *  \c jam() method is used instead. Using \c jam() will overwrite existing data. 
 * 
 *  Data is retrieved from the buffer with the \c get() method. It is usually a good
 *  idea to check if anything is in the buffer before attempting to retrieve data so 
 *  that an empty buffer can be handled gracefully: 
 *  \code
 *  uint16_t an_item;
 *  ...
 *  if (my_queue.is_empty ())
 *  {
 *      // Do something which handles an empty buffer without causing trouble.
 *      // Sometimes that means to just do something else until data arrives
 *  }
 *  else
 *  {
 *      an_item = my_queue.get ();
 *  }
 *  \endcode
 */

template <class qType, size_t qSize> 
class circ_buffer
{
	protected:
		qType buffer[qSize];            ///< This memory buffer holds the contents
		size_t i_put;                   ///< Index where newest data will be written
		size_t i_get;                   ///< Index where oldest data was written
		size_t how_full;                ///< How many elements are full at this time

	public:
		circ_buffer (void);             // Constructor
		bool put (qType);               // Adds one item into buffer
		bool jam (qType);               // Force entry even if buffer full
		qType get (void);               // Gets an item from the buffer
		void flush (void);              // Empty out the whole buffer

		// This operator returns an item at the given index in the buffer
		qType operator[] (size_t);

		/** This method returns the number of items in the buffer.
		 *  @return The number of items currently in the buffer
		 */
		size_t num_items (void)
		{
			return (how_full);
		}

		/** This method returns true if the buffer is empty.  Empty means that there 
		 *  isn't any data which hasn't previously been read.
		 *  @return True if the buffer has unread data, false if it doesn't
		 */
		bool is_empty (void)
		{
			return (how_full == 0);
		}

		/** This method returns true if the buffer is full.
		 *  @return True if the buffer is full, false if there is empty space left
		 */
		bool is_full (void)
		{
			return (how_full >= qSize);
		}

		/** This method returns a pointer to the buffer which holds the data in the 
		 *  queue. Normally it's not used, but in some cases (particularly queues of
		 *  characters) one may need to access the data buffer directly.
		 *  @return A pointer to the buffer holding the data in the queue
		 */
		qType* get_p_buffer (void)
		{
			return buffer;
		}
};


//-------------------------------------------------------------------------------------
/** This constructor creates a circular buffer object.  Note that it doesn't have to 
 *  allocate memory because that was already done statically by the template mechanism 
 *  at compile time. If no parameters are supplied, a 16-element buffer for characters 
 *  which uses 8-bit array indices (for a maximum of 255 elements) is created. 
 *  The template parameters are as follows: 
 *	\li qType: The type of data which will be stored in the buffer (default char)
 *	\li qSize: The number of items in the buffer's memory area (default 16). This is
 *             the number of items of the given data type, \b not the number of bytes
 *             needed to store all that data
 */ 

template <class qType, size_t qSize> 
circ_buffer<qType, qSize>::circ_buffer (void)
{
	flush ();
}


//-------------------------------------------------------------------------------------
/** This method empties the buffer. It doesn't actually erase everything; it just sets
 *  the indices and fill indicator all to zero as if the buffer contained nothing.
 */

template <class qType, size_t qSize> 
void circ_buffer<qType, qSize>::flush (void)
{
	i_put = 0;
	i_get = 0;
	how_full = 0;
}


//-------------------------------------------------------------------------------------
/** This method adds an item into the buffer. If the buffer is full then nothing is 
 *  written and false (meaning buffer is full) is returned. This allows the calling
 *  program to see that the data couldn't be written and try again if appropriate. 
 *  @param data The data to be written into the buffer 
 *  @return False if the buffer was full and data was not written, true otherwise
 */

template <class qType, size_t qSize> 
bool circ_buffer<qType, qSize>::put (qType data)
{
	// Check if the buffer is already full
	if (how_full >= qSize)
	{
		return (false);
	}

	// OK, there's room in the buffer so add the data in
	buffer[i_put] = data;
	if (++i_put >= qSize)
	{
		i_put = 0;
	}
	how_full++;

	return (true);
}


//-------------------------------------------------------------------------------------
/** This method jams an item into the buffer whether the buffer is empty or not.  This 
 *  can overwrite existing data, so it must be used with caution.
 *  @param data The data to be jammed into the buffer
 *  @return False if some data was lost, true otherwise
 */

template <class qType, size_t qSize> 
bool circ_buffer<qType, qSize>::jam (qType data)
{
	// Write the data and move the write pointer to the next element
	buffer[i_put] = data;
	if (++i_put >= qSize) 
	{
		i_put = 0;
	}

	// Check if the buffer is already full; if so, the read index has to be moved so
	// that it points to the oldest unread data, which isn't the data written now
	if (how_full >= qSize)
	{
		if (++i_get >= qSize)
		{
			i_get = 0;
		}
		return (false);
	}
	else
	{
		how_full++;                         // If we get here, the buffer isn't full
		return (true);
	}
}


//-------------------------------------------------------------------------------------
/** This method returns the oldest item in the buffer.  If the buffer was empty, this 
 *  is already-retreived data, and somebody should have checked if there was new data 
 *  available using the \c is_empty() method.
 *  @return The data which is pulled out from the buffer at the current location
 */

template <class qType, size_t qSize> 
qType circ_buffer<qType, qSize>::get (void)
{
	qType whatIgot;                         // Temporary storage for what was read

	whatIgot = buffer[i_get];               // Read and hold the data

	if (how_full > 0)                       // If the buffer's not empty,
	{                                       // move the read pointer to the next full
		if (++i_get >= qSize)               // element
		{
			i_get = 0;
		}
		how_full--;                         // There's now one less item in the buffer
	}

	return (whatIgot);
}


//-------------------------------------------------------------------------------------
/** This overloaded array subscript operator implements subscripts in the buffer. The 
 *  index is defined relative to the location at which the oldest data in the buffer
 *  was written. This operator returns the (index)-th item in the buffer, or (-1) 
 *  typecast to the data type if there aren't enough items in the buffer to get 
 *  to the given index. The retreived data is not removed from the buffer. 
 *  @param index The array index into the buffer at which to get a data item
 *  @return The data which is at the given index
 */

template <class qType, size_t qSize> 
qType circ_buffer<qType, qSize>::operator[] (size_t index)
{
	// Check if there's data written at the given location
	if (index > how_full)
	{
		return ((qType)(-1));
	}

	// Find an index pointing to the correct location in the buffer
	size_t getIndex = i_get + index;
	if (getIndex > qSize)
	{
		getIndex -= qSize;
	}

	// Get the data at that index location
	return (buffer[getIndex]);
}

#endif // _CIRC_BUFFER_H_
