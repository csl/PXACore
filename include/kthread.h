//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jun,28 2004
//    Module Name               : kthread.h
//    Module Funciton           : 
//                                This module countains kernal thread
//                                implementation code.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __KTHREAD_H__
#define __KTHREAD_H__

//
//Kernal thread scheduled time.
//Hello Taiwan maintaince a timer counter,
//and increase this counter every
//system clock interrupt,once this counter
//reachs KTHREAD_SCHEDULE_TIME,Hello Taiwan
//will schedule the kernal thread.
//
#define KTHREAD_SCHEDULE_TIME  20

//
//Kernal thread status code.
//
#define  KTHREAD_STATUS_READY       0x00000001  //Read to run.
#define  KTHREAD_STATUS_RUNNING     0x00000002  //Running now.
#define  KTHREAD_STATUS_BLOCKED     0x00000003  //Blocked,if this thread is 
                                                //waiting for a object's
                                                //signal status,or waiting 
                                                //for some resource,it resides
                                                //this status.

#define KTHREAD_STATUS_SUSPEND      0x00000004  //The thread is suspended.

//
//Kernal thread control block.
//It countains the thread's status data,such as
//the thread's general registers,FPU registers,
//and other status data.
//
typedef VOID (*LPKTHREAD_ROUTINE)(LPVOID);

struct __KTHREAD_CONTROL_BLOCK{
	DWORD               dwKThreadID;           //Kernal thread ID.
	DWORD               dwKThreadStatus;       //Kernal thread status.
	DWORD               dwKThreadPriority;     //Priority.
	DWORD               dwStackSize;           //Stack size.
	LPKTHREAD_ROUTINE   pKThreadRoutine;       //The start address of the kernal thread.
	LPVOID              pData;                 //The parameter of kernal thread routine.

	__KTHREAD_CONTROL_BLOCK*  pNext;           //Points to the next kernal thread.
	__KTHREAD_CONTROL_BLOCK*  pPrev;           //Points to the previous kernal thread.

	__KTHREAD_MSG       ktmsg[MAX_KTHREAD_MSG_NUM];  //Kernal thread message operation 
	                                                 //members.
	WORD                wHeader;                     //Message queue header index.
	WORD                wTrial;                      //Message queue trial index.
	WORD                wCurrentMsgNum;              //Current message number.
	WORD                wReserved;                   //Aligment to 4 byte boundry.

#ifdef __I386__                                //The following variables is used to save     
	                                           //all general registers and flag register.
	DWORD               dwEAX;
	DWORD               dwEBX;
	DWORD               dwECX;
	DWORD               dwEDX;
	DWORD               dwESI;
	DWORD               dwEDI;
	DWORD               dwEBP;
	DWORD               dwESP;

	DWORD               dwEIP;

	DWORD               dwEFlags;
#else
#endif
};

//
//Kernal thread message operations.
//

BOOL KtMsgQueueFull(__KTHREAD_CONTROL_BLOCK*);  //If the kernal thread's message queue is full.

BOOL KtMsgQueueEmpty(__KTHREAD_CONTROL_BLOCK*); //If the kernal thread's message queue is empty.

BOOL KtSendMessage(__KTHREAD_CONTROL_BLOCK*,__KTHREAD_MSG*);  //Add a message to the msg queue.

BOOL KtGetMessage(__KTHREAD_CONTROL_BLOCK*,__KTHREAD_MSG*);   //Get a message from the queue.

typedef BOOL (*__KTHREAD_MSG_HANDLER)(WORD,DWORD,DWORD);      //Kernal thread message handler type.

BOOL KtDispatchMessage(__KTHREAD_MSG*,__KTHREAD_MSG_HANDLER);  //Dispatch a message.

//
//Max kernal thread number.
//
#define MAX_KTHREAD_NUM  128


//
//Global functions.
//

//
//The following function create a kernal thread,and returns the thread's ID.
//

DWORD CreateKThread(DWORD dwStackSize,DWORD dwFlags,
					DWORD dwPriority,LPKTHREAD_ROUTINE pStartAddress,
					LPVOID pData,LPVOID pReserved);

//
//The following function terminal a kernal thread.
//The proper method to terminal a thread is termialed naturaly,
//but if you call this procedure to terminal a thread,the
//kernal thread will be terminaled roughly,and all resource
//this kernal thread allocated will NOT be released.
//
VOID  TerminalKThread(DWORD dwThreadID);

//
//The following function returns the current running kernal thread's ID.
//
DWORD GetCurrentKThreadID();

//
//Get kernal thread's control block,by the kernal thread ID.
//
__KTHREAD_CONTROL_BLOCK* GetKThreadControlBlock(DWORD);

//
//The following function re-schedule the kernal thread.
//
VOID ScheduleKThread(DWORD dwEsp);

//
//Set kernal thread's priority.
//
VOID SetKThreadPriority(DWORD,DWORD);

#endif //kthread.h