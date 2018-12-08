//*************************************************************************************
/** @file flip_flap.cpp
 *    This is a file with the C++ code in order to control DCC trains according to the
 *    NMRA standards. 
 *
 *
 *  @b Revisions:
 *    12-04-2018 SBL Original file
 * 
 *  @b Usage:
 *    This file is intended to be compiled and run on some sort of desktop or laptop
 *    computer using the GCC compiler on Windows Vista/7/8/10(tm) or better (such as
 *    Linux, Unix, or Mac) in Atmel Studio, built, and then programmed onto an AtMega.
 * 
 *  @b License:
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

// Only include the corresponding "*.h" file in the "*.cpp" file
#include "dcc.h"

//-------------------------------------------------------------------------------------
/** This constructor creates a DCC train controller task. Its main job is to call the
 *  parent class's constructor which does most of the work.
 *  @param a_name A character string which will be the name of this task
 *  @param a_priority The priority at which this task will initially run (default: 0)
 *  @param a_stack_size The size of this task's stack in bytes 
 *                      (default: configMINIMAL_STACK_SIZE)
 *  @param p_serial_dev Pointer to a serial device (port, radio, SD card, etc.) which can
 *                   be used by this task to communicate (default: NULL)
 */

DCC::DCC(const char* a_name,
        unsigned portBASE_TYPE a_priority,
        size_t a_stack_size,
        emstream* p_serial_dev) : TaskBase(a_name, a_priority, a_stack_size, p_serial_dev)
{
    /* 
    
    THIS IS OUR EFFORTS TO FOLLOW DCC COMMANDER
    @li https://github.com/fergul/DCCCommander
    
    */
    
    // Initializes motor driver pins for track
    // Set PC4, PC5, and PC6 on microcontroller as output pins
    // (PC4 goes to ENA, PC5 goes to ENB, PC6 goes to INA, PC7 goes to INB)
    // PC6 (INA) and PC7 (INB) set direction of motor rotation
    DDRC |= (1<<PC4) | (1<<PC5) | (1<<PC6) | (1<<PC7); // Sets data direction register to output
    // PC7 is going to IN1A, PC6 is IN2A, PC4 -> ENA, PC4 ENB
    PORTC |= (1<<PC4) | (1<<PC5) | (1<<PC6);           // Sets these pins high
    PORTC &= ~(1<<PC7);                                // Sets this pin low
    
    /*
    
    THIS IS OUR EFFORTS TO FOLLOW DCC++. THIS METHOD USES FAST PWM AND COMPARE MATCH (INVERTING).
    @li https://github.com/DccPlusPlus
    
    
    // Initializes motor driver pins for track
    // Set PC4, PC5, and PC6 on microcontroller as output pins
    // (PC4 goes to ENA, PC5 goes to ENB, PC6 goes to INA, PC7 goes to INB)
    // PC6 (INA) and PC7 (INB) set direction of motor rotation
    DDRC |= (1<<PC4) | (1<<PC5) | (1<<PC6) | (1<<PC7); // Sets data direction register to output
    // PC7 is going to IN1A, PC6 is IN2A, PC4 -> ENA, PC4 ENB
    PORTC |= (1<<PC4) | (1<<PC5) | (1<<PC6);           // Sets these pins high
    PORTC &= ~(1<<PC7);                                // Sets this pin low
    
    // Setting PWM to Fast PWM with TOP being the flag and being set by OCR2A pg. 204
    TCCR2A |= (1<<WGM21) | (1<<WGM20);
    TCCR2B |= (1<<WGM22);
    
    // Setting Compare Output Mode to Compare Match, clear OC2A at BOTTOM (inverting) pg. 202
    TCCR2A |= (1<<COM2A0);
    TCCR2A &= ~(1<<COM2A1);
    
    // Setting up timer prescaling to 1 or none pg. 205
    TCCR2B |= (1<<CS20);
    TCCR2B &= ~(1<<CS21);
    TCCR2B &= ~(1<<CS22); 
    
    // NOT SURE OF OCR2B
    OCR2A = DCC_ONE_BIT_TOTAL_DURATION_TIMER2; // We know this is making the wave
    OCR2B = DCC_ONE_BIT_PULSE_DURATION_TIMER2; // We know this is making the interrupt
    
    TIMSK2 |= (1<<2); // Enabling Output Compare Match Interrupt Enable
    DDRD |= (1<<7); // Setting bit 7 of PORTD as an output (data direction register)    
    */
    
    /* 
    THIS IS OUR EFFORT WE TRIED BY USING OUTPUT COMPARE MATCH AND SWITCHING INA AND INB
    BUT WITH THE VNH5019 THIS IS NOT POSSIBLE. WE NEED PERIODS OF 112.5 AND 200 us, BUT
    THE MINIMUM t_del OR TIME DELAY OF SWITCHING OPERATING MODES IS 200 us. 
    
    // Initializes motor driver pins for track
    // Set PC4, PC5, and PC6 on microcontroller as output pins
    // (PC4 goes to ENA, PC5 goes to ENB, PC6 goes to INA, PC7 goes to INB)
    // PC6 (INA) and PC7 (INB) set direction of motor rotation
    DDRC |= (1<<PC4) | (1<<PC5) | (1<<PC6) | (1<<PC7); // Sets data direction register to output
    PORTC |= (1<<PC4) | (1<<PC5) | (1<<PC6);           // Sets these pins high
    PORTC &= ~(1<<PC7);                                // Sets this pin low

    // Enables the motor driver for Timer Output Compare Match
    DDRD |= (1<<7); // Setting bit 7 of PORTD as an output (data direction register)
    TIMSK2 |= (1<<1); // Enabling Output Compare Match Interrupt Enable
    
    // Setting up Timer/Counter Control Register A
    // Sets Compare Output Mode
    TCCR2A |= (1<<COM2A0) | (1<<WGM21); // ***THIS IS DIFFERENT FROM FLIP FLAP!!!!!*** // Sets up Toggle OC2A on compare match
    TCCR2B |= (1<<WGM22);    // Fast PWM mode
    //TCCR2B |= (1<<CS20);   // Set prescaling to none
    TCCR2B |= (1<<CS21);     // Set pre-scaling to 8

    OCR2A = 49; // pg. 195  OCR2A=99 creates pulse at f=5000Hz (200 us), OCR2A=49 creates pulse at 10000Hz (100 us)
    /* Flip flap data
    TCCR2A |= (1<<COM2B1) | (1<<WGM20); // Clear OC2B on compare match, pg. 203
    TCCR2B |= (1<<CS20); // No pre-scaling pg. 205-206
    */
}

void DCC::run(void)
{
    portTickType LastWakeTime;                 // For scheduling how often task runs
    // Initializes the LastWakeTime variable with the current time. This happens just
    // once, before the infinite loop is entered
    LastWakeTime = get_tick_count ();
    // This is the task loop. Once the task has been initialized in the code just
    // above, the task loop runs, and it keeps running until the power is shut off
    uint16_t DCC_ticks = 0;  // Ticks counter for debugging purposes
    for (;;)
    {
        // Run the task's state machine here
        switch (state)
        {
            // Initialization of DCC interrupts and trains.
            case 0: // Already initialized move to state 1
                transition_to(1);
                break;
            case 1: // Switching between state 1, 2, and 3 to test switching signals
                DCC_ticks++;
                if (DCC_ticks == 2000)
                {
                    DCC_ticks = 0;
                    transition_to(2);
                }                    
                break;
            case 2:
                DCC_ticks++;
                if (DCC_ticks == 2000)
                {
                    DCC_ticks = 0;
                    transition_to(3);
                }
                break;
            case 3:
                DCC_ticks++;
                if (DCC_ticks == 2000)
                {
                    DCC_ticks = 0;
                    transition_to(1);
                }
                break;

        };
        // Tell the RTOS to delay until the given number of RTOS timer ticks have
        // elapsed. This means the code after this line runs every ticks_per_run
        // milliseconds if the RTOS interrupt is set to go off every 1 millisecond
        delay_from_for (LastWakeTime, 1);
    }
    
}
