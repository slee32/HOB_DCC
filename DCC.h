//*************************************************************************************
/** \file DCC.h
 *    This is a file with the C++ code in order to control DCC trains according to the
 *    NMRA standards.
 *
 *  Revised:
 *    12-04-2018 SBL Original File 
 * 
 *  Usage:
 *    This file is intended to be compiled and run on some sort of desktop or laptop 
 *    computer using the GCC compiler on Windows Vista/7/8/10(tm) or better (such as 
 *    Linux, Unix, or Mac) in Atmel Studio, built, and then programmed onto an AtMega.
 *
 * 
 *  License:
 *    This file is copyright 2018 by SBL and released under the Lesser GNU 
 *    Public License, version 3. It intended for educational use only, but its use
 *    is not limited thereto, except by the fact that it's not really useful for
 *    anything else, so nobody in his right (or left) mind would try to use it. 
 *    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
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

#include <avr/io.h>
#include <avr/interrupt.h>
#include "rs232int.h"
#include "emstream.h"
#include "base232.h"
#include <stdlib.h>
#include "taskbase.h"
#include "external_references.h"


// pg. 195  OCR2A=99 creates pulse at f=5000Hz (200 us), OCR2A=49 creates pulse at 10000Hz (100 us)
// MAY HAVE TO CHANGE FOR FAST PWM MODE
/* FAST PWM SETTINGS FOR DCC++ METHOD

For no prescaling: pg. 195
49 ---> 12.5 us (80kHz)

With prescaling of 8:
49 ---> 100 us (10 kHz)
56 ---> 114 us (8.772 kHz)
99 ---> 200 us (5 kHz)

*/

// Logic zero
#define DCC_ZERO_BIT_TOTAL_DURATION_TIMER2 99;  
#define DCC_ZERO_BIT_PULSE_DURATION_TIMER2 49;

// Logic one
#define DCC_ONE_BIT_TOTAL_DURATION_TIMER2 49;
#define DCC_ONE_BIT_PULSE_DURATION_TIMER2 24;

//-------------------------------------------------------------------------------------
/** @brief   DCC controller, child of TaskBase
 *  @details This class is for an RTOS task for controlling DCC trains, making them run,
 *           and in the future possibly to schedule them automatically.
 * 
 *  \section Usage
 *  In the file which contains @c main() the initializing of the task looks like as follows:
 *  @code
 *  
 *  ...
 *  main ()
 *  {
 *      ...
 *      DCC* DCC_1 = new DCC ("DCC 1", tskIDLE_PRIORITY + 5, 500, &ser_port);
 *      ...
 *  }
 *  \endcode
 *  In a header file which is read by all the source files in the project we re-declare
 *  the queue pointer with the keyword @c extern to make it globally accessible in all
 *  files that @c \#include this file. This task does not require a shared variable
 *  but in the next iteration it should include them.
 */
class DCC : public TaskBase
{
protected:
    bool train_a_on;     // Boolean for seeing if train A is on or off
    bool train_b_on;     // Boolean for seeing if train B is on or off
    
public:
    DCC(const char*, unsigned portBASE_TYPE, size_t, emstream*);
    // Constructor function for creating a task for DCC RTOS
    void run(void);     // RTOS run method for DCC task
};
