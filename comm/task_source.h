//**************************************************************************************
/** \file task_source.h
 *    This file contains the header for a task which sends test data to another task,
 *    the data sink task, using a queue, a shared data class object, and a dangerous
 *    global variable. The data will be checked to test the reliability of each method
 *    of conveying the information. 
 *
 *  Revisions:
 *    \li 09-30-2012 JRR Original file was a one-file demonstration with two tasks
 *    \li 10-05-2012 JRR Split into multiple files, one for each task
 *    \li 11-04-2012 JRR Changed into the test data source task
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
#ifndef _TASK_SOURCE_H_
#define _TASK_SOURCE_H_

#include <stdlib.h>                         // Prototype declarations for I/O functions

#include "FreeRTOS.h"                       // Primary header for FreeRTOS
#include "task.h"                           // Header for FreeRTOS task functions
#include "queue.h"                          // FreeRTOS inter-task communication queues

#include "rs232int.h"                       // ME405/507 library for serial comm.
#include "time_stamp.h"                     // Class to implement a microsecond timer
#include "taskbase.h"                       // Header of wrapper for FreeRTOS tasks
#include "taskqueue.h"                      // Header for generic RTOS queue class
#include "textqueue.h"                      // Header for RTOS text queue class
#include "taskshare.h"                      // Header for thread-safe shared data

#include "shares.h"                         // Global ('extern') queue declarations


/** This is the default number of RTOS ticks per run of this task. If this constant 
 *  is set to 100, for example, and the RTOS clock runs at 1000 Hz, then this task 
 *  will run 10 times per second. This is the default value used at program startup; 
 *  it's copied into a variable so that the rate of this task can be varied as needed.
 */
const TickType_t DEF_TICKS_PER_RUN = configTICK_RATE_HZ;         // For 1 Hz updates


//-------------------------------------------------------------------------------------
// Externs:  In this section, we declare variables and functions that are used in all
// (or at least two) of the files in the data acquisition project. Each of these items
// will also be declared exactly once, without the keyword 'extern', in one .cpp file
// as well as being declared extern here. 



//-------------------------------------------------------------------------------------
/** This task class creates some data to send to a sink task. We will check if the
 *  data got there safely, having made it through the dangerous waters of RTOS
 *  context switches and interrupts and sharks with lasers. 
 */

class task_source : public TaskBase
{
protected:

public:
	// This constructor creates a data acquisition task object
	task_source (const char*, unsigned portBASE_TYPE, size_t, emstream*);

	/** This run method is called by the RTOS and contains a loop in which the data
	 *  acquisition takes place.
	 */
	void run (void);
};


#endif // _TASK_SOURCE_H_
