//***********************************************************************/
//    Author                    : Garry
//    Original Date             : May,31 2004
//    Module Name               : timer.h
//    Module Funciton           : 
//                                This module countains time handle code
//                                defination and structures.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __TIMER__
#define __TIMER__

#define MAX_TIME_SLOT_OBJECT 512

#define SYS_CLOCK_SPAN  100      //System clock frequency.

//
//Timer handler data structures.
//

//Time slot object,when a task set a timer,master add one object
//to the time slot.
//When a time interrupt occured,system decrease dwTimeSpan of all the objects
//in the time slot,and if the dwTimeSpan's value equal to zero,send a message
//to the task identified by pTCB.
typedef struct __TSO{
	DWORD              dwTimeSpan;   //Time span
	__TASK_CTRL_BLOCK* pTCB;         //Task control block.
	WORD               wTimerID;
}__TIME_SLOT_OBJECT;

//Time handle procedures.
VOID TimerHandler(DWORD);

BOOL SetTimer(__TASK_CTRL_BLOCK*,DWORD,WORD);

DWORD GetRunTime();

#endif //timer.h