//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Sep 21, 2005
//    Module Name               : SYNOBJ.CPP
//    Module Funciton           : 
//                                This module countains synchronization object's implementation
//                                code.
//                                The following synchronization object(s) is(are) defined
//                                in this file:
//                                  1. EVENT
//                                  2. MUTEX
//                                  3. SEMAPHORE
//                                  4. TIMER
//
//                                ************
//                                This file is the most important file of Hello Taiwan.
//                                ************
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include "stdafx.h"

//Timer handler's parameter.
typedef struct
{
	struct __COMMON_OBJECT*        lpSynObject;  //Synchronous object.
	struct __PRIORITY_QUEUE*       lpWaitingQueue; //Waiting queue of the synchronous object.
	struct __KERNEL_THREAD_OBJECT* lpKernelThread; //Kernel thread who want to wait.
}__TIMER_HANDLER_PARAM;

//Timer handler routine for all synchronous object.
static DWORD WaitingTimerHandler(LPVOID lpData)
{
	struct __TIMER_HANDLER_PARAM*   lpHandlerParam = (struct __TIMER_HANDLER_PARAM*)lpData;
	DWORD                    dwFlags;

	if(NULL == lpHandlerParam)
	{
		BUG();
		return 0L;
	}

	__ENTER_CRITICAL_SECTION(NULL,dwFlags);  //Acquire kernel thread object's spinlock.
	switch(lpHandlerParam->lpKernelThread->dwWaitingStatus & OBJECT_WAIT_MASK)
	{
		case OBJECT_WAIT_RESOURCE:
		case OBJECT_WAIT_DELETED:
			break;
		case OBJECT_WAIT_WAITING:
			lpHandlerParam->lpKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
			lpHandlerParam->lpKernelThread->dwWaitingStatus |= OBJECT_WAIT_TIMEOUT;
			//Delete the lpKernelThread from waiting queue.
			lpHandlerParam->lpWaitingQueue->DeleteFromQueue(
				(struct __COMMON_OBJECT*)lpHandlerParam->lpWaitingQueue,
				(struct __COMMON_OBJECT*)lpHandlerParam->lpKernelThread);
			//Add this kernel thread to ready queue.
			lpHandlerParam->lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_READY;
			KernelThreadManager.AddReadyKernelThread((struct __COMMON_OBJECT*)&KernelThreadManager,
				lpHandlerParam->lpKernelThread);
			break;
		default:
			__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
			BUG();
			return 0L;
	}
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	return 0L;
}

//
//TimeOutWaiting is a global routine used by any synchronous objects' WaitForThisObjectEx
//routine.
//
DWORD TimeOutWaiting(struct __COMMON_OBJECT* lpSynObject,      //Synchronous object.
					 struct __PRIORITY_QUEUE* lpWaitingQueue,  //Waiting queue.
					 struct __KERNEL_THREAD_OBJECT* lpKernelThread,  //Who want to wait.
					 DWORD dwMillionSecond)  //Time out value in millionsecond.
{
	__TIMER_OBJECT*           lpTimerObj;
	struct __TIMER_HANDLER_PARAM     HandlerParam;

	if((NULL == lpSynObject) || (NULL == lpWaitingQueue) ||
	   (NULL == lpKernelThread) || (0 == dwMillionSecond))  //Invalid parameters.
	{
		BUG();
		return OBJECT_WAIT_FAILED;
	}

	//Initialize HandlerParam.
	HandlerParam.lpKernelThread  = lpKernelThread;
	HandlerParam.lpSynObject     = lpSynObject;
	HandlerParam.lpWaitingQueue  = lpWaitingQueue;

	lpKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
	lpKernelThread->dwWaitingStatus |= OBJECT_WAIT_WAITING;

	//Set a one time timer.
	lpTimerObj = (__TIMER_OBJECT*)System.SetTimer((struct __COMMON_OBJECT*)&System,
		lpKernelThread,
		#define TIMEOUT_WAITING_TIMER_ID 2048
		TIMEOUT_WAITING_TIMER_ID,
		dwMillionSecond,
		WaitingTimerHandler,
		(LPVOID)&HandlerParam,
		TIMER_FLAGS_ONCE);
	if(NULL == lpTimerObj)
	{
		return OBJECT_WAIT_FAILED;
	}

	KernelThreadManager.ScheduleFromProc(NULL);  //Re-schedule.

	//Once reach here,it means the waiting kernel thread was waken up.
	switch(lpKernelThread->dwWaitingStatus & OBJECT_WAIT_MASK)
	{
		case OBJECT_WAIT_RESOURCE:  //Got resource.
			System.CancelTimer((struct __COMMON_OBJECT*)&System,
				(struct __COMMON_OBJECT*)lpTimerObj);  //Cancel timer.
			return OBJECT_WAIT_RESOURCE;

		case OBJECT_WAIT_DELETED:   //Synchronous object was deleted.
			System.CancelTimer((struct __COMMON_OBJECT*)&System,
				(struct __COMMON_OBJECT*)lpTimerObj);
			return OBJECT_WAIT_DELETED;

		case OBJECT_WAIT_TIMEOUT:   //Time out.
			return OBJECT_WAIT_TIMEOUT;
		default:
			break;
	}
	//Once reach here,it means error encountered.
	BUG();
	return OBJECT_WAIT_FAILED;
}

//
//Routines pre-declaration.
//
static DWORD WaitForEventObject(struct __COMMON_OBJECT*);
static DWORD SetEvent(struct __COMMON_OBJECT*);
static DWORD ResetEvent(struct __COMMON_OBJECT*);
static DWORD WaitForEventObjectEx(struct __COMMON_OBJECT*,DWORD);

//---------------------------------------------------------------------------------
//
//                SYNCHRONIZATION OBJECTS
//
//----------------------------------------------------------------------------------

//
//Event object's initializing routine.
//This routine initializes the members of an event object.
//

BOOL EventInitialize(struct __COMMON_OBJECT* lpThis)
{
	BOOL                  bResult          = FALSE;
	struct __EVENT*              lpEvent          = NULL;
	struct __PRIORITY_QUEUE*     lpPriorityQueue  = NULL;

	if(NULL == lpThis)
		goto __TERMINAL;

	lpEvent = (struct __EVENT*)lpThis;

	lpPriorityQueue = (struct __PRIORITY_QUEUE*)
		ObjectManager.CreateObject(&ObjectManager,NULL,
		OBJECT_TYPE_PRIORITY_QUEUE);
	if(NULL == lpPriorityQueue)
		goto __TERMINAL;

	bResult = lpPriorityQueue->Initialize((struct __COMMON_OBJECT*)lpPriorityQueue);
	if(!bResult)
		goto __TERMINAL;

	lpEvent->lpWaitingQueue      = lpPriorityQueue;
	lpEvent->dwEventStatus       = EVENT_STATUS_OCCUPIED;
	lpEvent->SetEvent            = SetEvent;
	lpEvent->ResetEvent          = ResetEvent;
	lpEvent->WaitForThisObjectEx = WaitForEventObjectEx;
	lpEvent->WaitForThisObject   = WaitForEventObject;
	bResult                      = TRUE;

__TERMINAL:
	if(!bResult)
	{
		if(NULL != lpPriorityQueue)    //Release the priority queue.
			ObjectManager.DestroyObject(&ObjectManager,
			(struct __COMMON_OBJECT*)lpPriorityQueue);
	}
	return bResult;
}

//
//Event object's uninitializing routine.
//Safety deleted is support by EVENT object,so in this routine,
//if there are kernel threads waiting for this object,then wakeup
//all kernel threads,and then destroy the event object.
//

VOID EventUninitialize(struct __COMMON_OBJECT* lpThis)
{
	struct __EVENT*                 lpEvent          = NULL;
	struct __PRIORITY_QUEUE*        lpPriorityQueue  = NULL;
	struct __KERNEL_THREAD_OBJECT*  lpKernelThread   = NULL;
	DWORD                    dwFlags;

	if(NULL == lpThis)
	{
		BUG();
		return;
	}

	lpEvent = (struct __EVENT*)lpThis;

	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	lpPriorityQueue = lpEvent->lpWaitingQueue;
	if(EVENT_STATUS_FREE != EVENT_STATUS_FREE)
	{
		//Should wake up all kernel thread(s) who waiting for this object.
		lpKernelThread = (struct __KERNEL_THREAD_OBJECT*)
			lpPriorityQueue->GetHeaderElement(
			(struct __COMMON_OBJECT*)lpPriorityQueue,
			NULL);
		while(lpKernelThread)
		{
			lpKernelThread->dwThreadStatus   = KERNEL_THREAD_STATUS_READY;
			lpKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
			lpKernelThread->dwWaitingStatus |= OBJECT_WAIT_DELETED;
			KernelThreadManager.AddReadyKernelThread((struct __COMMON_OBJECT*)&KernelThreadManager,
				lpKernelThread);
			lpKernelThread = (struct __KERNEL_THREAD_OBJECT*)
				lpPriorityQueue->GetHeaderElement(
				(struct __COMMON_OBJECT*)lpPriorityQueue,
				NULL);
		}
	}
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

	ObjectManager.DestroyObject(&ObjectManager,
		(struct __COMMON_OBJECT*)lpPriorityQueue);          //*******CAUTION!!!************
		//lpPriorityQueue->Uninitialize((struct __COMMON_OBJECT*)lpPriorityQueue);
	return;
}

//
//The implementation of SetEvent.
//This routine do the following:
// 1. Saves the previous status into a local variable;
// 2. Sets the current status of the event to EVENT_STATUS_FREE;
// 3. Wakes up all kernel thread(s) in it's waiting queue.
// 4. Returns the previous status.
//

static DWORD SetEvent(struct __COMMON_OBJECT* lpThis)
{
	DWORD                     dwPreviousStatus     = EVENT_STATUS_OCCUPIED;
	struct __EVENT*                  lpEvent              = NULL;
	struct __KERNEL_THREAD_OBJECT*   lpKernelThread       = NULL;
	DWORD                     dwFlags              = 0L;

	if(NULL == lpThis)
		return dwPreviousStatus;

	lpEvent = (struct __EVENT*)lpThis;

	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	dwPreviousStatus = lpEvent->dwEventStatus;
	lpEvent->dwEventStatus = EVENT_STATUS_FREE;    //Set the current status to free.

	//Wake up all kernel thread(s) waiting for this event.
	lpKernelThread = (struct __KERNEL_THREAD_OBJECT*)
		lpEvent->lpWaitingQueue->GetHeaderElement(
		(struct __COMMON_OBJECT*)lpEvent->lpWaitingQueue,
		NULL);
	while(lpKernelThread)                         //Remove all kernel thread(s) from
		                                          //waiting queue.
	{
		lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_READY;
		//Set waiting result bit.
		lpKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
		lpKernelThread->dwWaitingStatus |= OBJECT_WAIT_RESOURCE;
		KernelThreadManager.AddReadyKernelThread(
			(struct __COMMON_OBJECT*)&KernelThreadManager,
			lpKernelThread);  //Add to ready queue.
		lpKernelThread = (struct __KERNEL_THREAD_OBJECT*)
			lpEvent->lpWaitingQueue->GetHeaderElement(
			(struct __COMMON_OBJECT*)lpEvent->lpWaitingQueue,
			NULL);
	}
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

	if(IN_KERNELTHREAD())  //Current context is in process.
	{
		KernelThreadManager.ScheduleFromProc(NULL);  //Re-schedule.
	}
	return dwPreviousStatus;
}

//
//The implementation of ResetEvent.
//

static DWORD ResetEvent(struct __COMMON_OBJECT* lpThis)
{
	struct __EVENT*          lpEvent          = NULL;
	DWORD             dwPreviousStatus = 0L;
	DWORD             dwFlags          = 0L;

	if(NULL == lpThis)
		return dwPreviousStatus;

	lpEvent = (struct __EVENT*)lpThis;

	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	dwPreviousStatus = lpEvent->dwEventStatus;
	lpEvent->dwEventStatus = EVENT_STATUS_OCCUPIED;
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

	return dwPreviousStatus;
}

//
//The implementation of WaitForEventObject.
//

static DWORD WaitForEventObject(struct __COMMON_OBJECT* lpThis)
{
	struct __EVENT*                      lpEvent             = NULL;
	struct __KERNEL_THREAD_OBJECT*       lpKernelThread      = NULL;
	__KERNEL_THREAD_CONTEXT*      lpContext           = NULL;
	DWORD                         dwFlags             = 0L;

	if(NULL == lpThis)
		return OBJECT_WAIT_FAILED;

	lpEvent = (struct __EVENT*)lpThis;

	__ENTER_CRITICAL_SECTION(NULL,dwFlags);

	if(EVENT_STATUS_FREE == lpEvent->dwEventStatus)
	{
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		return OBJECT_WAIT_RESOURCE;
	}
	else
	{
		lpKernelThread = KernelThreadManager.lpCurrentKernelThread;
		lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_BLOCKED;
		lpEvent->lpWaitingQueue->InsertIntoQueue(
			(struct __COMMON_OBJECT*)lpEvent->lpWaitingQueue,
			(struct __COMMON_OBJECT*)lpKernelThread,
			0L);

		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);    //Leave critical section here is safety.

		//lpContext = &lpKernelThread->KernelThreadContext;
		KernelThreadManager.ScheduleFromProc(NULL);
	}
	return OBJECT_WAIT_RESOURCE;
}

//WaitForEventObjectEx's implementation.
static DWORD WaitForEventObjectEx(struct __COMMON_OBJECT* lpObject, DWORD dwMillionSecond)
{
	struct __EVENT*                      lpEvent         = (struct __EVENT*)lpObject;
	struct __KERNEL_THREAD_OBJECT*       lpKernelThread  = NULL;
	DWORD                         dwFlags;
	DWORD                         dwTimeOutTick;
	DWORD                         dwTimeSpan;

	if(NULL == lpObject)
	{
		BUG();
		return OBJECT_WAIT_FAILED;
	}

	dwTimeOutTick = (dwMillionSecond / SYSTEM_TIME_SLICE) ? 
		(dwMillionSecond / SYSTEM_TIME_SLICE) : 1;
	dwTimeOutTick += System.dwClockTickCounter;

	__ENTER_CRITICAL_SECTION(NULL,dwFlags);  //Acquire event object's spinlock.
	if(EVENT_STATUS_FREE == lpEvent->dwEventStatus)
	{
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		return OBJECT_WAIT_RESOURCE;
	}
	//Should waiting now.
	if(0 == dwMillionSecond)  //Waiting zero time.
	{
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		KernelThreadManager.ScheduleFromProc(NULL);
		return OBJECT_WAIT_TIMEOUT;
	}
	lpKernelThread = KernelThreadManager.lpCurrentKernelThread;
	while(EVENT_STATUS_FREE != lpEvent->dwEventStatus)
	{
		if(dwTimeOutTick <= System.dwClockTickCounter)
		{
			__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
			return OBJECT_WAIT_TIMEOUT;
		}
		dwTimeSpan = (dwTimeOutTick - System.dwClockTickCounter) * SYSTEM_TIME_SLICE;
		lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_BLOCKED;
		//Add to event object's waiting queue.
		lpEvent->lpWaitingQueue->InsertIntoQueue(
			(struct __COMMON_OBJECT*)lpEvent->lpWaitingQueue,
			(struct __COMMON_OBJECT*)lpKernelThread,
			0L);

		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		
		switch(TimeOutWaiting((struct __COMMON_OBJECT*)lpEvent,lpEvent->lpWaitingQueue,
			lpKernelThread,dwTimeSpan))
		{
 			//Should loop to while again to check the status.
			case OBJECT_WAIT_RESOURCE:  
				__ENTER_CRITICAL_SECTION(NULL,dwFlags);
				break;
			case OBJECT_WAIT_TIMEOUT:
				return OBJECT_WAIT_TIMEOUT;
			case OBJECT_WAIT_DELETED:
				return OBJECT_WAIT_DELETED;
			default:
				BUG();
				return OBJECT_WAIT_FAILED;
		}
	}

	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

	return OBJECT_WAIT_RESOURCE;
}

////////////////////////////////////////////////////////////////////////////////////
//
// ------------------ ** The implementation of MUTEX object ** ---------------------
//
///////////////////////////////////////////////////////////////////////////////////

//
//The implementation of ReleaseMutex.
//

static DWORD ReleaseMutex(struct __COMMON_OBJECT* lpThis)
{
	struct __KERNEL_THREAD_OBJECT*     lpKernelThread   = NULL;
	__MUTEX*                    lpMutex          = NULL;
	DWORD                       dwPreviousStatus = 0L;
	DWORD                       dwFlags          = 0L;

	if(NULL == lpThis)    //Parameter check.
		return 0L;

	lpMutex = (__MUTEX*)lpThis;

	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	if(lpMutex->dwWaitingNum > 0)    //If there are other kernel threads waiting for this object.
		lpMutex->dwWaitingNum --;    //Decrement the counter.
	if(0 == lpMutex->dwWaitingNum)   //There is not kernel thread waiting for the object.
	{
		dwPreviousStatus = lpMutex->dwMutexStatus;
		lpMutex->dwMutexStatus = MUTEX_STATUS_FREE;  //Set to free.
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		return 0L;
	}
	lpKernelThread = (struct __KERNEL_THREAD_OBJECT*)lpMutex->lpWaitingQueue->GetHeaderElement(
		(struct __COMMON_OBJECT*)lpMutex->lpWaitingQueue,
		0L);  //Get one waiting kernel thread to run.
	lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_READY;
	lpKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
	lpKernelThread->dwWaitingStatus |= OBJECT_WAIT_RESOURCE;
	KernelThreadManager.AddReadyKernelThread(
		(struct __COMMON_OBJECT*)&KernelThreadManager,
		lpKernelThread);  //Put the kernel thread to ready queue.
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

	KernelThreadManager.ScheduleFromProc(NULL);  //Re-schedule kernel thread.
	return dwPreviousStatus;
}

//
//The implementation of WaitForMutexObject.
//

static DWORD WaitForMutexObject(struct __COMMON_OBJECT* lpThis)
{
	struct __KERNEL_THREAD_OBJECT*        lpKernelThread   = NULL;
	__MUTEX*                       lpMutex          = NULL;
	DWORD                          dwFlags          = 0L;

	if(NULL == lpThis)    //Parameter check.
		return 0L;

	lpMutex = (__MUTEX*)lpThis;

	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	if(MUTEX_STATUS_FREE == lpMutex->dwMutexStatus)    //If the current mutex is free.
	{
		lpMutex->dwMutexStatus = MUTEX_STATUS_OCCUPIED;  //Modify the current status.
		lpMutex->dwWaitingNum  ++;    //Increment the counter.
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		return OBJECT_WAIT_RESOURCE;  //The current kernel thread successfully occupy
		                              //the mutex.
	}
	else    //The status of the mutex is occupied.
	{
		lpKernelThread = KernelThreadManager.lpCurrentKernelThread;
		lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_BLOCKED;
		lpMutex->dwWaitingNum          ++;    //Increment the waiting number.

		lpMutex->lpWaitingQueue->InsertIntoQueue(
			(struct __COMMON_OBJECT*)lpMutex->lpWaitingQueue,
			(struct __COMMON_OBJECT*)lpKernelThread,
			0L);

		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);  //Leave critical section here is safety.
		//Reschedule all kernel thread(s).
		KernelThreadManager.ScheduleFromProc(NULL);
	}
	return OBJECT_WAIT_RESOURCE;
}

//
//Implementation of WaitForThisObjectEx routine.
//This routine is a time out waiting routine,caller can give a time value
//to indicate how long want to wait,once exceed the time value,waiting operation
//will return,even in case of the resource is not released.
//If the time value is zero,then this routine will check the current status of
//mutex object,if free,then occupy the object and return RESOURCE,else return
//TIMEOUT,and a re-schedule is triggered.
//
static DWORD WaitForMutexObjectEx(struct __COMMON_OBJECT* lpThis,DWORD dwMillionSecond)
{
	__MUTEX*                      lpMutex        = (__MUTEX*)lpThis;
	struct __KERNEL_THREAD_OBJECT*       lpKernelThread = NULL;
	DWORD                         dwFlags;

	if(NULL == lpMutex)
	{
		BUG();
		return OBJECT_WAIT_FAILED;
	}
	
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	if(MUTEX_STATUS_FREE == lpMutex->dwMutexStatus)  //Free now.
	{
		lpMutex->dwMutexStatus = MUTEX_STATUS_OCCUPIED;
		lpMutex->dwWaitingNum ++;
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		//KernelThreadManager.ScheduleFromProc(NULL);  //Re-schedule here.
		return OBJECT_WAIT_RESOURCE;
	}
	else  //The mutex is not free now.
	{
		if(0 == dwMillionSecond)
		{
			__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
			KernelThreadManager.ScheduleFromProc(NULL); //Re-schedule here.
			return OBJECT_WAIT_TIMEOUT;
		}
		lpKernelThread = KernelThreadManager.lpCurrentKernelThread;
		//Waiting on mutex's waiting queue.
		lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_BLOCKED;
		lpMutex->lpWaitingQueue->InsertIntoQueue(
			(struct __COMMON_OBJECT*)lpMutex->lpWaitingQueue,
			(struct __COMMON_OBJECT*)lpKernelThread,
			0L);
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

		return TimeOutWaiting((struct __COMMON_OBJECT*)lpMutex,
			lpMutex->lpWaitingQueue,
			lpKernelThread,
			dwMillionSecond);
	}
}
//
//The implementation of MutexInitialize.
//

BOOL MutexInitialize(struct __COMMON_OBJECT* lpThis)
{
	__MUTEX*             lpMutex     = NULL;
	struct __PRIORITY_QUEUE*    lpQueue     = NULL;
	BOOL                 bResult     = FALSE;

	if(NULL == lpThis) //Parameter check.
		return bResult;

	lpQueue = (struct __PRIORITY_QUEUE*)ObjectManager.CreateObject(&ObjectManager,
		NULL,
		OBJECT_TYPE_PRIORITY_QUEUE);
	if(NULL == lpQueue)    //Failed to create priority queue.
		return bResult;

	if(!lpQueue->Initialize((struct __COMMON_OBJECT*)lpQueue))  //Initialize the queue object.
		goto __TERMINAL;

	lpMutex = (__MUTEX*)lpThis;
	lpMutex->dwMutexStatus     = MUTEX_STATUS_FREE;
	lpMutex->lpWaitingQueue    = lpQueue;
	lpMutex->WaitForThisObject = WaitForMutexObject;
	lpMutex->dwWaitingNum      = 0L;
	lpMutex->ReleaseMutex      = ReleaseMutex;
	lpMutex->WaitForThisObjectEx = WaitForMutexObjectEx;

	bResult = TRUE;    //Successful to initialize the mutex object.

__TERMINAL:
	if(!bResult)
	{
		if(NULL != lpQueue)    //Release the queue object.
			ObjectManager.DestroyObject(&ObjectManager,
			(struct __COMMON_OBJECT*)lpQueue);
	}
	return bResult;
}

//
//The implementation of MutexUninitialize.
//This object support safety deleted,so in this routine,all kernel thread(s)
//must be waken up before this object is destroyed.
//

VOID MutexUninitialize(struct __COMMON_OBJECT* lpThis)
{
	struct __PRIORITY_QUEUE*       lpWaitingQueue  = NULL;
	struct __KERNEL_THREAD_OBJECT* lpKernelThread  = NULL;
	DWORD                   dwFlags;

	if(NULL == lpThis) //parameter check.
	{
		BUG();
		return;
	}

	lpWaitingQueue = ((__MUTEX*)lpThis)->lpWaitingQueue;
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	lpKernelThread = (struct __KERNEL_THREAD_OBJECT*)lpWaitingQueue->GetHeaderElement(
		(struct __COMMON_OBJECT*)lpWaitingQueue,
		NULL);
	while(lpKernelThread)
	{
		lpKernelThread->dwThreadStatus   = KERNEL_THREAD_STATUS_READY;
		lpKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
		lpKernelThread->dwWaitingStatus |= OBJECT_WAIT_DELETED;
		KernelThreadManager.AddReadyKernelThread(
			(struct __COMMON_OBJECT*)&KernelThreadManager,
			lpKernelThread);
		lpKernelThread = (struct __KERNEL_THREAD_OBJECT*)lpWaitingQueue->GetHeaderElement(
			(struct __COMMON_OBJECT*)lpWaitingQueue,
			NULL);
	}
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

	ObjectManager.DestroyObject(&ObjectManager,
		(struct __COMMON_OBJECT*)lpWaitingQueue);
	return;
}

