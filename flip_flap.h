//*************************************************************************************
/** \file flip_flap.h
 *           Headers for a child class of TaskBase for controlling a flip flap. 
 *           This file contains the child class for a flip flap which can be used
 *           to control the flip flap in RTOS by either setting it in a loop of going
 *           to the desired flaps or going to a desired flap based on serial input.
 *
 *  Revised:
 *    12-02-2018 SBL Original file
 * 
 *  Usage:
 *    This file is intended to be compiled and run on some sort of desktop or laptop
 *    computer using the GCC compiler on Windows Vista/7/8/10(tm) or better (such as
 *    Linux, Unix, or Mac) in Atmel Studio, built, and then programmed onto an AtMega.
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
#include <util/delay.h>
#include <math.h>
#include <avr/interrupt.h>
#include "rs232int.h"
#include "emstream.h"
#include "base232.h"
#include <stdlib.h>
#include "taskbase.h"

#include "external_references.h"
// We modified the "configENABLE_BACKWARD_COMPATIBILITY" value from a zero to a 1 in "FreeRTOSConfig.h" on line 120

//-------------------------------------------------------------------------------------
/** @brief   Flip flap controller class, child of TaskBase
 *  @details This class is for an RTOS task for controlling a flip flap, going to set 
 *           flap numbers (serial) or running a loop of flaps.
 * 
 *  \section Usage
 *  In the file which contains @c main() the initializing of the task looks like as follows:
 *  @code
 *  // Create an example task. It runs at priority 4, has a 300 byte stack, and a serial port named ser_port.
 *  TaskShare<uint16_t>* enc_position; // This is a task share variable for the encoder position of flip-flap
 *  TaskShare<uint16_t>* flap_num_com; // The flap number that is desired
 *  ...
 *  main ()
 *  {
 *      ...
 *      Flip_Flap* flip_flap_1 = new Flip_Flap ("Flip Flap 1", tskIDLE_PRIORITY + 4, 300, &ser_port);
 *      ...
 *  }
 *  \endcode
 *  In a header file which is read by all the source files in the project we re-declare
 *  the queue pointer with the keyword @c extern to make it globally accessible in all
 *  files that @c \#include this header file:
 *  @code
 *  extern TaskShare<uint16_t>* enc_position; // This is a task share variable for the encoder position of flip-flap
 *  extern TaskShare<uint16_t>* flap_num_com; // The flap number that is desired
 *  \endcode
 *  The encoder position or flap number can be obtained or set in ISRs or in this code as follows:
 *  @code
 *  flap_num_com->get()
 *  enc_position->ISR_put(0) // In ISR
 *  \endcode
 */
class Flip_Flap : public TaskBase
{
protected:
    uint16_t position;              // The position of the encoder (not used)
    uint16_t desiredFlap;           // Desired flap number (not used, shared used instead)
    uint16_t rev_enc_count;         // Total number of encoder ticks per one revolution
    uint16_t total_flap_count;      // Total number of flaps
    uint16_t set_point;             // The setpoint of the encoder needed to achieve the desired flap
public:    
    Flip_Flap(const char*, unsigned portBASE_TYPE, size_t, emstream*);
    // Constructor for creating a task for the flip flap
    bool LS_closed(void);                       // Method to find out whether the limit switch is closed/open
    void setDesiredFlap(uint16_t flap_num);     // Method that calculates the needed encoder tick/setpoint for a deired flap
    void runFlipFlap(void);                     // Method for the motor controller of the flip flap
    void run(void);                             // Run method for RTOS of flip flap
};