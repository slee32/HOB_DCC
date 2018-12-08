//*************************************************************************************
/** @file    baseshare.h
 *  @brief   Source code of a base class for type-safe, thread-safe task data exchange 
 *           classes.
 *  @details This file contains a base class for classes which exchange data between
 *           tasks. Inter-task data must be exchanged in a thread-safe manner, so the
 *           classes which share the data use mutexes or mutual exclusion mechanisms
 *           to prevent corruption of data. A linked list of all inter-task data items
 *           is kept by the ME405 system, and this base class contains members that
 *           handle that linked list. 
 *
 *  Revised:
 *    \li 10-18-2014 JRR Created file
 *
 *  License:
 *		This file is copyright 2014 by JR Ridgely and released under the Lesser GNU 
 *		Public License, version 2. It intended for educational use only, but its use
 *		is not limited thereto. */
/*		THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 *		AND	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 *		IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 *		ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
 *		LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUEN-
 *		TIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
 *		OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
 *		CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
 *		OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 *		OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */
//*************************************************************************************

#include "string.h"                         // C style string manipulation functions
#include "baseshare.h"                      // Header for the base share class


// Set the pointer to the most recently created shared data item to initially be NULL
BaseShare* BaseShare::p_newest = NULL;


//-------------------------------------------------------------------------------------
/** @brief   Construct a base shared data item.
 *  @details This default constructor saves the name of the shared data item. It is
 *           not to be called by application code (nobody has any reason to create a
 *           base class object which can't do anything!) but instead by the 
 *           constructors of descendent classes. 
 *  @param   p_name The name for the shared data item, in a character string
 */

BaseShare::BaseShare (const char* p_name = NULL)
{
	// Allocate some memory and then save the share's name; trim it to 12 characters
	if (p_name != NULL)
	{
		uint8_t namelength = strlen (p_name);
		namelength = (namelength <= 12) ? namelength : 12;
		name = new char[namelength + 1];
		strncpy (name, p_name, namelength);
	}

	// Install this share in the linked list of shares
	p_next = p_newest;
	p_newest = this;
}


//-------------------------------------------------------------------------------------
/** @brief   Start the printout showing the status of all shared data items.
 *  @details This function begins printing out the status of all items in the system's 
 *           linked list of shared data items (queues, task shares, and so on). The 
 *           most recently created share's status is printed first, followed by the 
 *           status of other shares in reverse order of creation. 
 *  @param   p_ser_dev A pointer to the serial device on which shares are printed
 */

void print_all_shares (emstream* p_ser_dev)
{
	if (BaseShare::p_newest != NULL)
	{
		*p_ser_dev << PMS ("Share/Queue     Type    Free/Total") << endl;
		*p_ser_dev << PMS ("----            ----    ----------") << endl;

		BaseShare::p_newest->print_in_list (p_ser_dev);
	}
	else
	{
		*p_ser_dev << PMS ("No shared data items to print") << endl;
	}
}
