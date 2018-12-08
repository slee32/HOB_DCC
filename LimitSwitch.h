//*************************************************************************************
/** @file LimitSwitch.h
*		This code defines a limit switch class.
*
*	@b Revisions:
*		12.02.2018 SRW Original File
*
*	@ Usage:
*		This file is intended to be used on an Atmega 644A microcontroller and
*		compiled with Atmel Studio.
*
*	@b License:
*		This file is copyright 2018 by SR Waal and released under the Lesser GNU 
*   	Public License, version 3.
*		THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
*		AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
*		IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
*		ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
*		LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUEN-
*		TIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
*		OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
*		CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
*		OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
*		OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/
//*************************************************************************************

// //Initializing
//     /*
//     Flip-flap is connected to Timer/Counter 2 and Pin OC2B (PORTD bit 6 PD6)
//     */
//     DDRA &= ~(1<<PA5); // Setting up PA5 as an input for limit switch
//     PORTA |= (1<<5); // Enabling PA5 as pull-up
//  while (init == true) // Main initialization loop for flip flap
//     {
//         if ((!(!!(PINA & _BV(PA5)))) && first_cycle) 
//         // If the limit switch is already tripped on startup and in the first cycle
//         {
//             OCR2B = 15*255/100; // Setting the duty cycle to 15%
//         }
//         else if ((!!(PINA & _BV(PA5))))
//         // Checking if the limit switch is not triggered, set duty cycle of motor
//         {
//             OCR2B = 15*255/100; // Setting the duty cycle to 15%
//             first_cycle = false; // Not in first cycle anymore
//         }
//         else if (!(!!(PINA & _BV(PA5))))
//         // When the limit switch is triggered (again) then turn the motor off
//         {
//             OCR2B = 0; // Turn the motor off
//             init = false;
//         }



//-------------------------------------------------------------------------------------
/** @brief   	A class for a limit switch configured as "normally open"
 *  @details 	This class encapsulates the basic functionality for a limit switch.
 */
class LimitSwitch
{
protected:
	bool is_closed;						// Open if false, closed if true
public:
	LimitSwitch(void);// Default constructor
	bool getState(void);				// Returns "is_closed" value
    void run(void);
};

