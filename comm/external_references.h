//*************************************************************************************
/*  File:  external_references.h
 *    This is a file with all the external references needed.
 *
 *  Revisions:
 *    12-03-2018 SBL Original file
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

#include "taskshare.h"

extern TaskShare<uint16_t>* enc_position; // This is a task share variable for the encoder position of flip-flap
extern TaskShare<uint16_t>* flap_num_com; // The flap number that is desired
