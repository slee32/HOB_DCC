//*************************************************************************************
/** @file flip_flap.cpp
 *    This is a file with the C++ code for a flip flap in a RTOS environment.
 *    The flip flap can be controlled to display the flap needed.
 *    
 *
 *  @b Revisions:
 *    12-01-2018 SBL Original File 
 * 
 *  @b Usage:
 *    This file is intended to be compiled and run on some sort of desktop or laptop 
 *    computer using the GCC compiler on Windows Vista/7/8/10(tm) or better (such as 
 *    Linux, Unix, or Mac) in Atmel Studio, built, and then programmed onto an AtMega.
 *
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
#include "flip_flap.h"

// This class includes functionality for a flip-flap display
// It sets up the encoder and the limit switch for a flip-flap display
//-------------------------------------------------------------------------------------
/** This constructor creates a flip flap controller task. Its main job is to call the
 *  parent class's constructor which does most of the work.
 *  @param a_name A character string which will be the name of this task
 *  @param a_priority The priority at which this task will initially run (default: 0)
 *  @param a_stack_size The size of this task's stack in bytes 
 *                      (default: configMINIMAL_STACK_SIZE)
 *  @param p_serial_dev Pointer to a serial device (port, radio, SD card, etc.) which can
 *                   be used by this task to communicate (default: NULL)
 */
Flip_Flap::Flip_Flap(const char* a_name, 
    unsigned portBASE_TYPE a_priority, 
    size_t a_stack_size, 
    emstream* p_serial_dev) : TaskBase(a_name, a_priority, a_stack_size, p_serial_dev)
{
    // Initializes limit switch pin PA5
    DDRA &= ~(1<<PA5);          // Setting up PA5 as an input for limit switch
    PORTA |= (1<<5);            // Enabling PA5 as pull-up
    
    // Initializes pins for encoder interrupt
    PCICR |= (1<<0);            // Enable Pin Change interrupts for PCINT 0:7 pg 89
    PCMSK0 |= (0b11000000);     // Enable masks for Bit 6 and Bit 7 pg 91
    //sei();                    // Turn on interrupts, not needed here
    
    // Initializes motor driver pins for flip-flap display
    // Set PB4, PB3, and PB1 on microcontroller as output pins
    // (PB4 goes to ENA, PB3 goes to ENB, PB2 goes to INA, PB1 goes to INB)
    // PB2 (INA) and PB1 (INB) set direction of motor rotation
    DDRB |= (1<<PB4) | (1<<PB3) | (1<<PB2) | (1<<PB1); // Sets data direction register to output
    PORTB |= (1<<PB4) | (1<<PB3) | (1<<PB1);           // Sets these pins high
    PORTB &= ~(1<<PB2);                                // Sets this pin low
    
    // Enables the motor driver PWM
    DDRD |= (1<<6); // Setting bit 6 of PORTD as an output (data direction register)
    TCCR2A |= (1<<COM2B1) | (1<<WGM20); // Clear OC2B on compare match, pg. 203
    TCCR2B |= (1<<CS20); // No pre-scaling pg. 205-206
    
    // Starting the encoder ticks and total number of flap
    rev_enc_count = 1200;               // Number of encoder ticks per one rev of the flip-flap
    total_flap_count = 18;              // Total number of flaps
}


//-------------------------------------------------------------------------------------
/** This method checks whether the limit switch is closed (pressed).
 *  @return Returns a boolean that is true if the limit switch is closed.
 */
bool Flip_Flap::LS_closed(void)
{
    // Returns the state of the limit switch. Returns true if closed, false if open
    return (!(!!(PINA & _BV(PA5))));
}

//-------------------------------------------------------------------------------------
/** This method calculates the encoder position for a desired flap number and sets
 *  the setpoint of the motor for the flip flap to that calculated encoder position.
 *  @param  flap_num This is an unsigned 16 bit number that corresponds to the flap number desired.
 */
void Flip_Flap::setDesiredFlap(uint16_t flap_num)
{
    uint16_t flap_enc_count = rev_enc_count/total_flap_count;
    set_point = flap_enc_count*flap_num;
}


//-------------------------------------------------------------------------------------
/** This method is to run function for motor control. This is where the control
 *  constants are controlled and saturation limits of the actuation is set as well.
 */
void Flip_Flap::runFlipFlap()
{
    uint16_t error = 0;                 // Initializing an error variable
    float KP = .3;                      // Proportional control constant
    float KI = 0;                       // Integral control constant
    uint16_t error_sum = 0;             // Error sum for integral control
    float actuation = 0;                // Actuation for motor
    
    // Position is found through enc_position->get() because it is a shared variable.
    
    error = abs(set_point - enc_position->get());
    // Calculating the error

    error_sum += error; // Calculating the sum of error
    actuation = error*KP + error_sum*KI; // Calculating actuation
    actuation = actuation*255/rev_enc_count; // Converting the actuation into units for the motor driver
    if (actuation > 20) // Saturation limit
    {
        actuation = 20;
    }
    OCR2B = (uint8_t)actuation; // Setting the register for the actuation of the motors
}


//-------------------------------------------------------------------------------------
/** This method is the run function for RTOS.
 */
void Flip_Flap::run(void)
{
    portTickType LastWakeTime;                 // For scheduling how often task runs
    // Initializes the LastWakeTime variable with the current time. This happens just
    // once, before the infinite loop is entered
    LastWakeTime = get_tick_count ();
    // This is the task loop. Once the task has been initialized in the code just
    // above, the task loop runs, and it keeps running until the power is shut off
    uint16_t ticks_enc = 0; // Initializing a tick counter for debugging.
    bool first_cycle = true; // First cycle for properly zeroing and initializing encoder.
    for (;;)
    {
        // Run the task's state machine here
        switch (state)
        {
            // Initialization state: zeros flip-flap display and gets ready to move to a desired flap
            case 0:
                if (LS_closed() && first_cycle)
                // If the limit switch is already closed on startup and in the first cycle
                {
                    OCR2B = 15*255/100; // Setting the duty cycle to 15% for initializing
                }
                else if (!LS_closed())
                // Checking if the limit switch is open, set duty cycle of motor
                {
                    OCR2B = 15*255/100; // Setting the duty cycle to 15%
                    first_cycle = false; // Not in first cycle anymore
                }
                else if (LS_closed())
                // When the limit switch is closed (again), turn the motor off
                {
                    OCR2B = 0; // Turn the motor off
                    transition_to(1);
                }
                break;
            case 1:
                // Control flip-flap display
                runFlipFlap();
                ticks_enc++;
                //setDesiredFlap(flap_num_com->get()); // If using serial input use this!
                if (ticks_enc >= 1000)
                {
                    setDesiredFlap(1);
                    ticks_enc = 0;
                    transition_to(2);
                }
                       
                break;
            case 2:
                runFlipFlap();
                ticks_enc++;
                if (ticks_enc >= 1000)
                {
                    setDesiredFlap(6);
                    ticks_enc = 0;
                    transition_to(3);
                }
                break;
            case 3:
                runFlipFlap();
                ticks_enc++;
                if (ticks_enc >= 1000)
                {
                    setDesiredFlap(6);
                    ticks_enc = 0;
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

