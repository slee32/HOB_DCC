//**************************************************************************************
/** \file task_multi.h
 *    This file contains the header for a task class which is will be instantiated many
 *    times to test how many tasks can be safely run on various AVR processors using
 *    the ME507/FreeRTOS software. 
 *
 *  Revisions:
 *    \li 09-30-2012 JRR Original file was a one-file demonstration with two tasks
 *    \li 10-05-2012 JRR Split into multiple files, one for each task
 *    \li 10-25-2012 JRR Changed to a more fully C++ version with class task_sender
 *    \li 10-27-2012 JRR Altered from data sending task into LED blinking class
 *    \li 11-04-2012 JRR Altered again into the multi-task monstrosity
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
#ifndef _TASK_MULTI_H_
#define _TASK_MULTI_H_

#include <stdlib.h>                         // Prototype declarations for I/O functions

#include "FreeRTOS.h"                       // Primary header for FreeRTOS
#include "task.h"                           // Header for FreeRTOS task functions
#include "queue.h"                          // FreeRTOS inter-task communication queues

#include "taskbase.h"                       // ME405/507 base task class
#include "rs232int.h"                       // ME405/507 library for serial comm.
#include "time_stamp.h"                     // Class to implement a microsecond timer
#include "taskqueue.h"                      // Header of wrapper for FreeRTOS queues
#include "taskshare.h"                      // Header for thread-safe shared data


//-------------------------------------------------------------------------------------
/** This task doesn't do much except take up space and processor time to test the
 *  ME507/FreeRTOS software. The idea is to instantiate a whole bunch of these tasks
 *  (it has been tested with over 50 of them on an ATmega1284p) and run them all to 
 *  make sure the processor can handle allocating memory for and running so many
 *  tasks. Each task object will run for a very brief time, then call its delay()
 *  method for a semi-pseudo-random period of time. 
 */

class task_multi : public TaskBase
{
private:

protected:

public:
// 	uint32_t runs;                          ///< How many times through the task loop
// 
	// This constructor creates a generic task of which many copies can be made
	task_multi (const char*, unsigned portBASE_TYPE, size_t, emstream*);

	/** This run method is called by the RTOS and contains a loop in which the task
	 *  checks for data and sends it if appropriate.
	 */
	void run (void);
};

#endif // _TASK_MULTI_H_
