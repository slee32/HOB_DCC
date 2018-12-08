//*************************************************************************************
/** \file shares.h
 *    This file contains extern declarations for queues and other inter-task data
 *    communication objects used in a ME405/507/FreeRTOS project. 
 *
 *  Revisions:
 *    \li 09-30-2012 JRR Original file was a one-file demonstration with two tasks
 *    \li 10-05-2012 JRR Split into multiple files, one for each task plus a main one
 *    \li 10-29-2012 JRR Reorganized with global queue and shared data references
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

/** \mainpage Task communication tests
 *  This program implements a set of tests to verify that the ME405 library's 
 *  inter-task communication works. It's also a usable example of how the queues and
 *  shared data items are used. In addition, it helps to show why non thread safe data
 *  transfer is a bad idea; there is a global variable used to pass data between two
 *  tasks, and as the program runs, the global variable sometimes gets corrupted.
 * 
 *  The task diagram for this program is shown below. 
 *  \dot
 *  digraph task_diagram {
 *  node [ shape = box, fontname = FreeSans, fontsize = 10 ];
 *    task_multi1 [ label = "MultiTask 1\nPri: 1\n10-137 ms" URL = "\ref task_multi" ];
 *    task_multi2 [ label = "MultiTask 2\nPri: 1\n10-137 ms" URL = "\ref task_multi" ];
 *    task_multiN [ label = "MultiTask N\nPri: 1\n10-137 ms" URL = "\ref task_multi" ];
 *    task_sink   [ label = "Data Sink\nPri: 2\n0-20 ms" URL = "\ref task_sink" ];
 *    task_source [ label = "Data Source\nPri: 2\n1 ms" URL = "\ref task_source" ];
 *    task_user   [ label = "User Interface\nPri: 0\n1 ms" URL = "\ref task_user" ];
 *  edge [ arrowhead = normal, style = solid, fontname = FreeSans, fontsize = 10 ];
 *    task_source -> task_sink [ label = "queue_1\nuint32_t" ];
 *    task_source -> task_sink [ label = "share_1\nuint32_t", style = dashed ];
 *    task_source -> task_sink [ label = "glob_of_probs\nuint32_t", style = dotted ];
 *    task_multi1 -> task_user [ label = "print_ser_queue\nchar" ];
 *    task_multi2 -> task_user [ label = "print_ser_queue\nchar" ];
 *    task_multiN -> task_user [ label = "print_ser_queue\nchar" ];
 *    task_sink   -> task_user [ label = "print_ser_queue\nchar" ];
 *  }
 *  \enddot
 *  There are a couple of unusual features on this task diagram. The number of 
 *  "multi-tasks" can be varied by changing \c N_MULTI_TASKS in \c main(). These
 *  multi-tasks are a bunch of tasks, all created from the same class, that are used
 *  to put some stress on the RTOS by running lots of tasks at the same time. The
 *  multi-task's timing includes a delay of random duration; this timing scheme causes
 *  tasks to pre-empt each other with random timing. The data sink task normally runs
 *  very quickly, looking for changed data; if it finds data, it delays by 20 ms to 
 *  give other tasks some time to run. The data sink therefore takes up a lot of CPU
 *  time, and one can see that the user interface runs slowly because of this. 
 * 
 *  When this program is running, one can press the "v" key in a terminal window to
 *  cause the program to display a task status report. A typical report is shown 
 *  below.
 *  \code
 *  Task                            Stack
 *  Name            Pri.    State   Free/Total      Runs
 *  ----            ----    -----   ----------      ----
 *  Multi3          1       0       63/120          0        153 runs
 *  Multi2          1       0       63/120          0        161 runs
 *  Multi1          1       0       63/120          0        157 runs
 *  Multi0          1       0       63/120          0        152 runs
 *  UserInt         1       0       66/240          0        4217 runs
 *  Sink            2       0       92/160          0       1136446 runs, errors in queue: 0, shared_data: 0, global data: 265
 *  Source          2       0       145/220         0        10446 runs
 *  IDLE            0       -       50/100
 *  Heap: 4000/5944, OCR5A=1999
 *  \endcode
 *  This report shows tasks and their current priorities and states. Also, for the
 *  data sink task, it shows a report of how accurately the data from the data source
 *  task has been received. One should see that there have been no errors in sending
 *  data through the queue or shared data item, but the use of a global variable has
 *  caused errors due to task switching while the data was being written or read. 
 */


// This define prevents this .h file from being included multiple times in a .cpp file
#ifndef _SHARES_H_
#define _SHARES_H_

//-------------------------------------------------------------------------------------
// Externs:  In this section, we declare variables and functions that are used in all
// (or at least two) of the files in the data acquisition project. Each of these items
// will also be declared exactly once, without the keyword 'extern', in one .cpp file
// as well as being declared extern here. 

// This queue allows tasks to send characters to the user interface task for display.
extern TextQueue* p_print_ser_queue;

/*  This queue sends data from the source task to the sink task.
 */
extern TaskQueue<uint32_t>* p_queue_1;

/*  This shared data item allows a value to be posted by the source task and read by
 *  the sink task.
 */
extern TaskShare<uint32_t>* p_share_1;

/*  This global variable will be written by the source task and read by the sink task.
 *  We expect the process to be corrupted by context switches now and then.
 */
extern uint32_t glob_of_probs;

/*  This shared data item is used to test the increment and decrement operators for
 *  shared data items. 
 */
extern TaskShare<int8_t>* p_share_counter;

#endif // _SHARES_H_
