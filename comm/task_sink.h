//**************************************************************************************
/** \file task_sink.h
 *    This file contains the source code for a task which receives data via a queue, a
 *    shared data item, and a dangerous global variable so that the data may be checked
 *    for errors in transmission. 
 *
 *  Revisions:
 *    \li 09-30-2012 JRR Original file was a one-file demonstration with two tasks
 *    \li 10-05-2012 JRR Split into multiple files, one for each task
 *    \li 10-25-2012 JRR Changed to a more fully C++ version with class task_sender
 *    \li 11-03-2012 JRR Morphed again into a data sink task with error checks
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

// This define prevents this .h file from being included multiple times in a .cpp file
#ifndef _TASK_SINK_H_
#define _TASK_SINK_H_

#include <stdlib.h>                         // Prototype declarations for I/O functions

#include "FreeRTOS.h"                       // Primary header for FreeRTOS
#include "task.h"                           // Header for FreeRTOS task functions
#include "queue.h"                          // FreeRTOS inter-task communication queues

#include "rs232int.h"                       // ME405/507 library for serial comm.
#include "time_stamp.h"                     // Class to implement a microsecond timer
#include "taskqueue.h"                      // Header of wrapper for FreeRTOS queues
#include "textqueue.h"                      // Header for text queue class
#include "taskshare.h"                      // Header for thread-safe shared data
#include "shares.h"                         // Global ('extern') queue declarations


//-------------------------------------------------------------------------------------
/** This task reads measurements that were taken by the data acquisition task in
 *  task_daq.* and puts those measurements into a queue. 
 */

class task_sink : public TaskBase
{
private:

protected:
	/// This is the number of errors in the queued data.
	uint32_t queue_errors;

	/// This is the number of errors in the data shared by frt_shared_data.
	uint32_t share_errors;

	/// This is the number of errors in the data shared by a global variable.
	uint32_t global_errors;

public:
	// This constructor creates a data sender task object
	task_sink (const char*, unsigned portBASE_TYPE, size_t, emstream*);

	/** This run method is called by the RTOS and contains a loop in which the task
	 *  checks for data and sends it if appropriate.
	 */
	void run (void);

	// Show how many times each transmission method messed up
	void show_errors (void);

	// Print how this task is doing on its tests
	void print_status (emstream&);
};

#endif // _TASK_SINK_H_
