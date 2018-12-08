#define F_CPU 8000000   // AVR clock frequency in Hz, used by util/delay.h
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "rs232int.h"   // Include for serial communication through an RS232 channel
#include "emstream.h"   // Including for emstream, mostly '<<' operator
#include "base232.h"
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"       // RTOS task header
#include "taskshare.h"  // Class for data to be shared in a thread safe manner between tasks
#include "taskqueue.h"  // Implements a queue to transmit from one RTOS task to another
#include "taskbase.h"   // Has base class for a task
#include "flip_flap.h"  // Flip flap task
#include "dcc.h"        // DCC task
#include "task_user.h"  // User task to get serial input

// Creating task share variables
TaskShare<uint16_t>* enc_position; // This is a task share variable for the encoder position of flip-flap
TaskShare<uint16_t>* flap_num_com; // The flap number that is desired

rs232 ser_port (9600, 0); // Opening and starting an RS232 serial port at a baud rate of 9600

uint16_t mode = 0; // Mode 0 for flip flap, mode 1 for DCC

uint16_t ticks_1 = 0; // Debugging for ISR FF
uint16_t ticks = 0; // Debugging for ISR DCC


//-------------------------------------------------------------------------------------
/** This is the main method of the main file.
 */
int main() {
    
    if (mode == 0) // Flip flap
    {
        ser_port << "For flip flap" << endl;
        
        //task_user* user_task = new task_user("Task User 1", tskIDLE_PRIORITY + 2, 255, &ser_port);
        
        //flap_num_com = new TaskShare<uint16_t>  ("Flap Number");
        flap_num_com->put(0);
        
        // Create an a task. It runs at priority 4 and has a 300 byte stack
        Flip_Flap* flip_flap_1 = new Flip_Flap ("Flip Flap 1", tskIDLE_PRIORITY + 4, 300, &ser_port);
        
        // New shared variable of Encoder Position
        enc_position = new TaskShare<uint16_t>  ("Encoder_Position");
        // Initialize as zero
        enc_position->put(0);
        
    }
    else if (mode == 1) // DCC
    {
        ser_port << "For DCC" << endl;
        // Create an a task for DCC. It runs at priority 5, has a 500 byte stack, serial port ser_port
        DCC* DCC_1 = new DCC ("DCC 1", tskIDLE_PRIORITY + 5, 500, &ser_port);
    }
    
    // Enable interrupts
    sei();
    // Start up the scheduler, causing all tasks to be run
    vTaskStartScheduler();
}

ISR(PCINT0_vect)
{
    /* Interrupts subroutine for updating the encoder position for the flip-flap.
    Prints the position (for now)
    Increments the position. There is no fancy tracking of the channel A and B of the encoder
    because the motor for the flip-flap will only spin in one direction.
    */

    enc_position->ISR_put(enc_position->ISR_get()+1);
    //enc_position->print_in_list(&ser_port);
    
    // Changed desired flap and flap num from uint8_t to uint16_t**
    
    
    ticks_1++;
    
    if (ticks_1 >= 1000)
    {
        ser_port << enc_position->ISR_get() << endl;
        ticks_1 = 0;
    }
    
    if ((!(!!(PINA & _BV(PA5)))) && (enc_position->ISR_get() > 600))
    {
        //flip_flap_1->rev_enc_count = flip_flap_1->position;
        //ser_port << flip_flap_1->position << endl;
        //ser_port << "ZERO" << endl;
        enc_position->ISR_put(0);
    }
}    

ISR(TIMER2_COMPB_vect)
{
    /* Interrupts subroutine for generating the DCC signal. It works to some extent.
    */
    ticks++;
    
    /* Trying to control the length and frequency of interrupts to generate the logic
    1 and 0 for DCC signals.
    */
    if (ticks == 1)
    {
         OCR2A = DCC_ZERO_BIT_TOTAL_DURATION_TIMER2;
         OCR2B = DCC_ZERO_BIT_PULSE_DURATION_TIMER2;
    }
    else if(ticks == 3)
    {
        OCR2A = DCC_ONE_BIT_TOTAL_DURATION_TIMER2;
        OCR2B = DCC_ONE_BIT_PULSE_DURATION_TIMER2;
    }
    else if(ticks == 2)
    {
        OCR2A = DCC_ZERO_BIT_TOTAL_DURATION_TIMER2;
        OCR2B = DCC_ZERO_BIT_PULSE_DURATION_TIMER2;
    }
    else if(ticks == 4)
    {
        OCR2A = DCC_ONE_BIT_TOTAL_DURATION_TIMER2;
        OCR2B = DCC_ONE_BIT_PULSE_DURATION_TIMER2;
        ticks = 0;
    }
        
    /*
    OCR2A = DCC_ZERO_BIT_TOTAL_DURATION_TIMER2;
    OCR2B = DCC_ZERO_BIT_PULSE_DURATION_TIMER2;
    ticks++;
    if (ticks == 1000)
    {
        //ser_port << "h";
        ticks = 0;
    }
    */
    
}