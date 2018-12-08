//*************************************************************************************
/** \file mechutil.cpp
 *    This file contains a set of enhancements to the GCC-AVR system which support 
 *    some of the more helpful C++ features that aren't present or working well in the
 *    normal GCC-AVR distribution (though I have no idea why). The information came 
 *    from an AVR-Freaks posting at: 
 *    \li http://www.avrfreaks.net/index.php?name=PNphpBB2&file=viewtopic&p=410870
 * 
 *    Other stuff is added to streamline the connection between the ME405 library and
 *    FreeRTOS. Of particular importance are FreeRTOS versions of 'new' and 'delete.' 
 *
 *  Revisions
 *    \li 04-12-2008 JRR Original file, material from source above
 *    \li 09-30-2012 JRR Added code to make memory allocation work with FreeRTOS
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
//*************************************************************************************

#include "mechutil.h"


//-------------------------------------------------------------------------------------
// Stuff to make the new and delete operators work

#ifdef INC_FREERTOS_H
	//---------------------------------------------------------------------------------
	/** This operator maps 'new' to the FreeRTOS memory allocation function, which is 
	*  configurable and thread safe, because the regular malloc() shouldn't be used in
	*  a FreeRTOS program. 
	*  @param size The number of bytes which are to be allocated
	*  @return A pointer to the memory area which has just been allocated
	*/

	void* operator new (size_t size)
	{
		return pvPortMalloc (size);
	}


	//---------------------------------------------------------------------------------
	/** This operator maps 'delete' to the FreeRTOS memory deallocation function, 
	 *  which is configurable and thread safe.
	 *  @param ptr A pointer to the memory area whose contents are to be deleted
	 */

	void operator delete (void *ptr)
	{
		if (ptr) vPortFree (ptr);
	}


	//---------------------------------------------------------------------------------
	/** This operator maps the array version of 'new' to the FreeRTOS memory allocation 
	 *  function, which is configurable and thread safe, because the regular malloc() 
	 *  shouldn't be used in a FreeRTOS program. Note that memory deallocation is only
	 *  possible when using some (not all) versions of the memory allocator in FreeRTOS;
	 *  for example, heap_1.c doesn't allow deallocation at all. 
	 *  @param size The number of bytes which are to be allocated
	 *  @return A pointer to the memory area which has just been allocated
	 */

	void* operator new[] (size_t size)
	{
		return pvPortMalloc (size);
	}


	//---------------------------------------------------------------------------------
	/** This operator maps the array version of 'delete' to the FreeRTOS memory 
	 *  deallocation function, which is configurable and thread safe. Note that memory 
	 *  deallocation is only possible when using some (not all) versions of the memory 
	 *  allocator in FreeRTOS; for example, heap_1.c doesn't allow deallocation at all. 
	 *  @param ptr A pointer to the memory area whose contents are to be deleted
	 */

	void operator delete[] (void *ptr)
	{
		if (ptr) vPortFree (ptr);
	}

#else
	#warning Seems like a FreeRTOS project, but not using pvPortMalloc()

	// If we're not using FreeRTOS, we can use the regular malloc() and free()
	void* operator new (size_t size) 
		{ 
		return malloc (size); 
		} 
	
	void operator delete (void* ptr) 
		{ 
		if (ptr) free (ptr); 
		} 
		
	void* operator new[] (size_t size)
		{
		return malloc (size);
		}
	
	void operator delete[] (void* ptr) 
		{ 
		if (ptr) free (ptr); 
		}
#endif


//-------------------------------------------------------------------------------------
/** This function is used to help make templates and virtual methods work. The author
 *  isn't sure exactly how it works, but the Interwebs seem to have half a clue. 
 *  Information about this and related functions can be found at
 *  http://www.avrfreaks.net/index.php?name=PNphpBB2&file=printview&t=57466&start=0
 *  @param g A pointer to some random thing
 *  @return Something that's pointed to by g
 */
extern "C"
{
int __cxa_guard_acquire (__guard *g)
    {
    return !*(char *)(g);
    }
}


//-------------------------------------------------------------------------------------
/** This function is used to help make templates and virtual methods work. The author
 *  isn't sure exactly how it works, but the Interwebs seem to have half a clue. 
 *  Information about this and related functions can be found at 
 *  http://www.avrfreaks.net/index.php?name=PNphpBB2&file=printview&t=57466&start=0
 *  @param g A pointer to some random thing
 */
extern "C"
{
void __cxa_guard_release (__guard *g)
    {
    *(char *)g = 1;
    }
}


//-------------------------------------------------------------------------------------
/** This function is used to help make templates and virtual methods work. The author
 *  isn't sure exactly how it works, but the Interwebs seem to have half a clue. 
 *  Information about this and related functions can be found at
 *  http://www.avrfreaks.net/index.php?name=PNphpBB2&file=printview&t=57466&start=0
 *  @param g A pointer to some random thing
 */
extern "C"
{
void __cxa_guard_abort (__guard *g)
    {
		(void)g;							// Shuts up a dumb compiler warning
    }
}


//-------------------------------------------------------------------------------------
/** This function is used to help make pure virtual methods work. The author isn't sure 
 *  exactly how it works (OK, clueless), but it might be a function that fills in for 
 *  the body of a pure virtual method, \e i.e. a method which doesn't have any body in 
 *  the regular source files. Or maybe it's a secret note from space aliens, who knows. 
 *  Information about this and related functions can be found at
 *  http://www.avrfreaks.net/index.php?name=PNphpBB2&file=printview&t=57466&start=0
 */

extern "C"
{    
void __cxa_pure_virtual (void)
    {
    }
}


