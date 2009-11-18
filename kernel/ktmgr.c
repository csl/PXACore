//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct,18 2004
//    Module Name               : ktmgr.cpp
//    Module Funciton           : 
//                                This module countains kernel thread and kernel thread 
//                                manager's implementation code.
//
//                                ************
//                                This file is the most important file of Hello Taiwan.
//                                ************
//    Last modified Author      : Garry
//    Last modified Date        : Sep,30,2006
//    Last modified Content     :
//                                1. 
//                                2.
//    Lines number              :
//***********************************************************************/

#include "stdafx.h"

//#include "archstd.h"

//
//Pre-declare for extern global routines,these routines may
//be implemented in KTMGRx.CPP file,where x is 2,3,etc.
//

extern __THREAD_HOOK_ROUTINE SetThreadHook(DWORD dwHookType, __THREAD_HOOK_ROUTINE lpRoutine);
extern VOID CallThreadHook(DWORD dwHookType, struct __KERNEL_THREAD_OBJECT* lpPrev, 			
								struct __KERNEL_THREAD_OBJECT* lpNext);
extern struct __KERNEL_THREAD_OBJECT* GetScheduleKernelThread(struct __COMMON_OBJECT* lpThis, DWORD dwPriority);
extern VOID AddReadyKernelThread(struct __COMMON_OBJECT* lpThis, struct __KERNEL_THREAD_OBJECT* lpKernelThread);
extern VOID KernelThreadWrapper(struct __COMMON_OBJECT*);
extern DWORD WaitForKernelThreadObject(struct __COMMON_OBJECT* lpThis);
//extern void __SwitchTo(LPVOID);
//
//Static global varaibles.
//

char* lpszCriticalMsg = "CRITICAL ERROR : The internal data structure is not consecutive,\
                         please restart the system!!";

//
//The initialization routine of Kernel Thread Object.
//In the current implementation of Hello Taiwan,we do the following task:
// 1. Create the waiting queue object of the kernel thread object;
// 2. Initialize the waiting queue object;
// 3. Set appropriaty value of the member functions,such as WaitForThisObject.
//

BOOL KernelThreadInitialize(struct __COMMON_OBJECT* lpThis)
{
	BOOL                    bResult           = FALSE;
	struct __PRIORITY_QUEUE*       lpWaitingQueue    = NULL;
	struct __KERNEL_THREAD_OBJECT* lpKernelThread    = NULL;
	struct __PRIORITY_QUEUE*       lpMsgWaitingQueue = NULL;

	if(NULL == lpThis)  //Parameter check.
		goto __TERMINAL;

	lpKernelThread = (struct __KERNEL_THREAD_OBJECT*) lpThis;
	lpWaitingQueue = (struct __PRIORITY_QUEUE*) ObjectManager.CreateObject(
						&ObjectManager,
						NULL,
						OBJECT_TYPE_PRIORITY_QUEUE
					      );
	if(NULL == lpWaitingQueue)  //Failed to create the waiting queue object.
		goto __TERMINAL;

	if(!lpWaitingQueue->Initialize((struct __COMMON_OBJECT*)lpWaitingQueue))
		goto __TERMINAL;    //Failed to initialize the waiting queue object.

	lpMsgWaitingQueue = (struct __PRIORITY_QUEUE*) ObjectManager.CreateObject(
						&ObjectManager,
						NULL,
						OBJECT_TYPE_PRIORITY_QUEUE);
	if(NULL == lpMsgWaitingQueue)  //Can not create message waiting queue.
		goto __TERMINAL;
	if(!lpMsgWaitingQueue->Initialize((struct __COMMON_OBJECT*)lpMsgWaitingQueue))
		goto __TERMINAL;

	lpKernelThread->lpWaitingQueue    = lpWaitingQueue;
	lpKernelThread->lpMsgWaitingQueue = lpMsgWaitingQueue;
	lpKernelThread->WaitForThisObject = WaitForKernelThreadObject;

	//Depend on Arch
	//lpKernelThread->lpKernelThreadContext = NULL;

	bResult = TRUE;


__TERMINAL:
	if(!bResult)        //Initialize failed.
	{
	  if(lpWaitingQueue != NULL)
		ObjectManager.DestroyObject(&ObjectManager,(struct __COMMON_OBJECT*)lpWaitingQueue);
	  if(lpMsgWaitingQueue != NULL)
		ObjectManager.DestroyObject(&ObjectManager,(struct __COMMON_OBJECT*)lpMsgWaitingQueue);
	}
	return bResult;
}

//
//The Uninitialize routine of kernel thread object.
//

VOID KernelThreadUninitialize(struct __COMMON_OBJECT* lpThis)
{
	struct __KERNEL_THREAD_OBJECT*   lpKernelThread = NULL;
	struct __PRIORITY_QUEUE*         lpWaitingQueue = NULL;
	struct __EVENT*                  lpMsgEvent     = NULL;

	if(NULL == lpThis)    //Parameter check.
		return;

	lpKernelThread = (struct __KERNEL_THREAD_OBJECT*)lpThis;
	//Destroy waiting queue.
	ObjectManager.DestroyObject(&ObjectManager,
		(struct __COMMON_OBJECT*)lpKernelThread->lpWaitingQueue);

	//Destroy message waiting queue.
	ObjectManager.DestroyObject(&ObjectManager,
		(struct __COMMON_OBJECT*)lpKernelThread->lpMsgWaitingQueue);

	return;
}

//
//The implementation of Kernel Thread Manager.
//

//Initializing routine of Kernel Thread Manager.

static BOOL KernelThreadMgrInit(struct __COMMON_OBJECT* lpThis)
{
	BOOL    bResult          = FALSE;
	struct __KERNEL_THREAD_MANAGER*   lpMgr            = NULL;
	struct __PRIORITY_QUEUE*          lpRunningQueue   = NULL;
	struct __PRIORITY_QUEUE*          lpReadyQueue     = NULL;
	struct __PRIORITY_QUEUE*          lpSuspendedQueue = NULL;
	struct __PRIORITY_QUEUE*          lpSleepingQueue  = NULL;
	struct __PRIORITY_QUEUE*          lpTerminalQueue  = NULL;
	DWORD i;

	if(NULL == lpThis)
		return bResult;

	lpMgr = (struct __KERNEL_THREAD_MANAGER*) lpThis;

	//
	//The following code creates all objects required by Kernel Thread Manager.
	//If any error occurs,the initializing process is terminaled.
	//

	lpRunningQueue = (struct __PRIORITY_QUEUE*)ObjectManager.CreateObject			
						(&ObjectManager, NULL ,OBJECT_TYPE_PRIORITY_QUEUE);

	if(NULL == lpRunningQueue) {
		  printf("NULL = lpRunningQueue\n");
		  goto __TERMINAL;
	}

	if(FALSE == lpRunningQueue->Initialize((struct __COMMON_OBJECT*)lpRunningQueue))
	{
		  printf("lpRunningQueue initialize error\n");
		  goto __TERMINAL;
	}

	lpReadyQueue = (struct __PRIORITY_QUEUE*)ObjectManager.CreateObject
						(&ObjectManager,NULL,OBJECT_TYPE_PRIORITY_QUEUE);
	if(NULL == lpReadyQueue) 
	{
		printf("NULL = lpReadyQueue\n");
		goto __TERMINAL;
	}
	if(FALSE == lpReadyQueue->Initialize((struct __COMMON_OBJECT*) lpReadyQueue))
	{
		printf("lpReadyQueue initialize error\n");
		goto __TERMINAL;
	}

	lpSuspendedQueue = (struct __PRIORITY_QUEUE*)ObjectManager.CreateObject
						(&ObjectManager,NULL,OBJECT_TYPE_PRIORITY_QUEUE);
	if(NULL == lpSuspendedQueue)
		goto __TERMINAL;
	if(FALSE == lpSuspendedQueue->Initialize((struct __COMMON_OBJECT*)lpSuspendedQueue))
		goto __TERMINAL;

	lpSleepingQueue = (struct __PRIORITY_QUEUE*)ObjectManager.CreateObject
						(&ObjectManager,NULL,OBJECT_TYPE_PRIORITY_QUEUE);
	if(NULL == lpSleepingQueue)
		goto __TERMINAL;
	if(FALSE == lpSleepingQueue->Initialize((struct __COMMON_OBJECT*) lpSleepingQueue))
		goto __TERMINAL;

	lpTerminalQueue = (struct __PRIORITY_QUEUE*)ObjectManager.CreateObject
						(&ObjectManager,NULL,OBJECT_TYPE_PRIORITY_QUEUE);
	if(NULL == lpTerminalQueue)
		goto __TERMINAL;
	if(FALSE == lpTerminalQueue->Initialize((struct __COMMON_OBJECT*)lpTerminalQueue))
		goto __TERMINAL;


	//
	//Now,the objects required by Kernel Thread Manager are created and initialized success-
	//fully,initialize the kernel thread manager itself now.
	//
	lpMgr->lpRunningQueue    = lpRunningQueue;
	//lpMgr->lpReadyQueue      = lpReadyQueue;
	lpMgr->lpSuspendedQueue  = lpSuspendedQueue;
	lpMgr->lpSleepingQueue   = lpSleepingQueue;
	lpMgr->lpTerminalQueue   = lpTerminalQueue;

	//Initializes the ready queue array.Any element failure can cause the whole
	//process to fail.
	for(i = 0;i < MAX_KERNEL_THREAD_PRIORITY + 1;i ++)
	{
		lpReadyQueue = (struct __PRIORITY_QUEUE*)ObjectManager.CreateObject(&ObjectManager,
								NULL,
								OBJECT_TYPE_PRIORITY_QUEUE);
		if(NULL == lpReadyQueue)
		{
			goto __TERMINAL;
		}
		//Can not initialize it.
		if(!lpReadyQueue->Initialize((struct __COMMON_OBJECT*)lpReadyQueue))  
		{
			goto __TERMINAL;
		}
		lpMgr->ReadyQueue[i] = lpReadyQueue;
	}

	lpMgr->lpCurrentKernelThread = NULL;

	bResult = TRUE;

__TERMINAL:


	if(!bResult)  //If failed to initialize the Kernel Thread Manager.
	{
	   printf("failed to initialize the Kernel Thread Manager\n");

	   if(NULL != lpRunningQueue)  //Destroy the objects created just now.
		ObjectManager.DestroyObject(&ObjectManager,(struct __COMMON_OBJECT*)lpRunningQueue);
   	   if(NULL != lpReadyQueue)
		ObjectManager.DestroyObject(&ObjectManager,(struct __COMMON_OBJECT*)lpReadyQueue);
	   if(NULL != lpSuspendedQueue)
		ObjectManager.DestroyObject(&ObjectManager,(struct __COMMON_OBJECT*)lpSuspendedQueue);
	   if(NULL != lpSleepingQueue)
	   	ObjectManager.DestroyObject(&ObjectManager,(struct __COMMON_OBJECT*)lpSleepingQueue);
	   if(NULL != lpTerminalQueue)
		ObjectManager.DestroyObject(&ObjectManager,(struct __COMMON_OBJECT*)lpTerminalQueue);
	}

	return bResult;
}

//
//CreateKernelThread's implementation.
//This routine do the following:
// 1. Create a kernel thread object by calling CreateObject;
// 2. Initializes the kernel thread object;
// 3. Create the kernel thread's stack by calling KMemAlloc;
// 4. Insert the kernel thread object into proper queue.
//

static struct __KERNEL_THREAD_OBJECT* CreateKernelThread(struct __COMMON_OBJECT*  lpThis, 
						  DWORD dwStackSize, 
						  DWORD dwStatus, 
						  DWORD dwPriority, 
						  __KERNEL_THREAD_ROUTINE lpStartRoutine, 
					           LPVOID lpRoutineParam, 
					           LPVOID lpReserved, 
						  LPSTR lpszName)
{
	struct __KERNEL_THREAD_OBJECT* lpKernelThread = NULL;
	struct __KERNEL_THREAD_MANAGER* lpMgr = NULL;
	LPVOID lpStack = NULL;
	BOOL bSuccess = FALSE;
	DWORD* lpStackPtr = NULL;
	DWORD i;

	if((NULL == lpThis) || (NULL == lpStartRoutine))    //Parameter check.
		goto __TERMINAL;

	if((KERNEL_THREAD_STATUS_READY != dwStatus) && 
					(KERNEL_THREAD_STATUS_SUSPENDED != dwStatus))
		goto __TERMINAL;

	lpMgr = (struct __KERNEL_THREAD_MANAGER*)lpThis;

	lpKernelThread = (struct __KERNEL_THREAD_OBJECT*)
		ObjectManager.CreateObject(&ObjectManager, NULL, OBJECT_TYPE_KERNEL_THREAD);

	if(NULL == lpKernelThread)    //If failed to create the kernel thread object.
		goto __TERMINAL;

	if(!lpKernelThread->Initialize((struct __COMMON_OBJECT*) lpKernelThread))    
		//Failed to initialize.
		goto __TERMINAL;

	if(0 == dwStackSize)
	//If the dwStackSize is zero,then allocate the default size's stack.
	{
		dwStackSize = DEFAULT_STACK_SIZE;
	}
	else			//If dwStackSize is too small.
	{
		if(dwStackSize < MIN_STACK_SIZE)    
		{
			dwStackSize = MIN_STACK_SIZE;
		}
	}

	lpStack = KMemAlloc(dwStackSize, KMEM_SIZE_TYPE_ANY);

	if(NULL == lpStack)    //Failed to create kernel thread stack.
	{
		goto __TERMINAL;
	}

	//The following code initializes the kernel thread object created just now.
	lpKernelThread->dwThreadID            = lpKernelThread->dwObjectID;
	lpKernelThread->dwThreadStatus        = dwStatus;
	lpKernelThread->dwThreadPriority      = dwPriority;
	lpKernelThread->dwScheduleCounter     = dwPriority;  //***** CAUTION!!! *****
	lpKernelThread->dwReturnValue         = 0L;
	lpKernelThread->dwTotalRunTime        = 0L;
	lpKernelThread->dwTotalMemSize        = 0L;

	lpKernelThread->bUsedMath             = FALSE;      //May be updated in the future.
	lpKernelThread->dwStackSize           = dwStackSize ? dwStackSize : DEFAULT_STACK_SIZE;

	lpKernelThread->lpInitStackPointer    = (LPVOID)((DWORD) lpStack + dwStackSize);

	lpKernelThread->KernelThreadRoutine   = lpStartRoutine;       //Will be updated.
	lpKernelThread->lpRoutineParam        = lpRoutineParam;

	lpKernelThread->ucMsgQueueHeader      = 0;
	lpKernelThread->ucMsgQueueTrial       = 0;
	lpKernelThread->ucCurrentMsgNum       = 0;

	lpKernelThread->dwLastError           = 0L;
	lpKernelThread->dwWaitingStatus       = OBJECT_WAIT_WAITING;

	//Copy kernel thread name.
	if(lpszName)
	{
		for(i = 0;i < MAX_THREAD_NAME - 1;i ++)
		{
			if(lpszName[i] == 0)  //End.
			{
				break;
			}
			lpKernelThread->KernelThreadName[i] = lpszName[i];
		}
	}

	lpKernelThread->KernelThreadName[i] = 0;  //Set string's terminator.

	//
	//The following routine initializes the hardware context
	//of the kernel thread.
	//It's implementation depends on the hardware platform,so
	//this routine is implemented in ARCH directory.
	//

	//Depend on ARCH
	lpKernelThread->lpInitStackPointer = InitKernelThreadContext(lpKernelThread);

	if(KERNEL_THREAD_STATUS_READY == dwStatus)         //Add into Ready Queue.
	{
		lpMgr->AddReadyKernelThread((struct __COMMON_OBJECT*) lpMgr, lpKernelThread);
	}
	else                                               //Add into Suspended Queue.
	{
		if(!lpMgr->lpSuspendedQueue->InsertIntoQueue(
		(struct __COMMON_OBJECT*)lpMgr->lpSuspendedQueue, (struct __COMMON_OBJECT*) lpKernelThread,dwPriority))
			goto __TERMINAL;
	}

	//Call the create hook.
	//lpMgr->CallThreadHook(THREAD_HOOK_TYPE_CREATE, lpKernelThread, NULL);
	bSuccess = TRUE;  //Now,the TRANSACTION of create a kernel thread is successfully.

__TERMINAL:

	if(!bSuccess)
	{
		//First,release the resources created successfully.
		if(NULL != lpKernelThread)
			ObjectManager.DestroyObject(&ObjectManager,(struct __COMMON_OBJECT*) lpKernelThread);
		if(NULL != lpStack)
			KMemFree(lpStack,KMEM_SIZE_TYPE_ANY,0L);
		return NULL;
	}
	else
		return lpKernelThread;
}

//
//DestroyKernelThread's implementation.
//The routine do the following:
// 1. Check the status of the kernel thread object will be destroyed,if the
//    status is KERNEL_THREAD_STATUS_TERMINAL,then does the rest steps,else,
//    simple return;
// 2. Delete the kernel thread object from Terminal Queue;
// 3. Destroy the kernel thread object by calling DestroyObject.
//

static VOID DestroyKernelThread(struct __COMMON_OBJECT* lpThis,struct __COMMON_OBJECT* lpKernel)
{
	struct __KERNEL_THREAD_OBJECT*     lpKernelThread   = NULL;
	struct __KERNEL_THREAD_MANAGER*    lpMgr            = NULL;
	struct __PRIORITY_QUEUE*           lpTerminalQueue  = NULL;
	LPVOID                      lpStack          = NULL;

	if((NULL == lpThis) || (NULL == lpKernel))    //Parameter check.
		return;

	lpKernelThread = (struct __KERNEL_THREAD_OBJECT*)lpKernel;
	lpMgr          = (struct __KERNEL_THREAD_MANAGER*)lpThis;

	if(KERNEL_THREAD_STATUS_TERMINAL != lpKernelThread->dwThreadStatus)
		return;

	lpTerminalQueue = lpMgr->lpTerminalQueue;
	lpTerminalQueue->DeleteFromQueue((struct __COMMON_OBJECT*)lpTerminalQueue,
		                             (struct __COMMON_OBJECT*)lpKernelThread);  //Delete from terminal queue.

	//Call terminal hook routine.
	lpMgr->CallThreadHook(THREAD_HOOK_TYPE_TERMINAL,
		lpKernelThread,NULL);

	lpStack = lpKernelThread->lpInitStackPointer;
	lpStack = (LPVOID)((DWORD)lpStack - lpKernelThread->dwStackSize);
	KMemFree(lpStack,KMEM_SIZE_TYPE_ANY,0L);    //Free the stack of the kernel thread.

	ObjectManager.DestroyObject(&ObjectManager,
		                        (struct __COMMON_OBJECT*)lpKernelThread);

}

//SuspendKernelThread's implementation.
static BOOL SuspendKernelThread(struct __COMMON_OBJECT* lpThis,struct __COMMON_OBJECT* lpKernelThread)
{
	return FALSE;
}

//ResumeKernelThread's implementation.
static BOOL ResumeKernelThread(struct __COMMON_OBJECT* lpThis,struct __COMMON_OBJECT* lpKernelThread)
{
	return FALSE;
}

//
//ScheduleFromProc's implementation.
//This routine can be called anywhere that re-schedule is required.
//

static VOID ScheduleFromProc(void)
//__KERNEL_THREAD_CONTEXT* lpContext
{
	struct __KERNEL_THREAD_OBJECT*          lpKernelThread     = NULL;
	struct __KERNEL_THREAD_OBJECT*          lpCurrent          = NULL;
	struct __KERNEL_THREAD_OBJECT*          lpNew              = NULL;
	//__KERNEL_THREAD_CONTEXT**        	lppOldContext      = NULL;
	//__KERNEL_THREAD_CONTEXT**        	lppNewContext      = NULL;
	DWORD                            dwFlags            = 0L;


	__ENTER_CRITICAL_SECTION(NULL,dwFlags);

	//get CurrentKernelThread
	lpCurrent = KernelThreadManager.lpCurrentKernelThread;

	switch(lpCurrent->dwThreadStatus)  //Do different actions according to status.
	{

	case KERNEL_THREAD_STATUS_RUNNING:

		lpNew = KernelThreadManager.GetScheduleKernelThread(
			(struct __COMMON_OBJECT*)&KernelThreadManager,
			lpCurrent->dwThreadPriority);    //Get a ready kernel thread.

		if(NULL == lpNew)  //Current one is most priority whose status is READY.
		{
			lpCurrent->dwTotalRunTime += SYSTEM_TIME_SLICE;

			__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
			return;  //Let current kernel thread continue to run.
		}
		else  //Should swap out current kernel thread and run next ready one.
		{
			lpCurrent->dwThreadStatus = KERNEL_THREAD_STATUS_READY;
			KernelThreadManager.AddReadyKernelThread(
				(struct __COMMON_OBJECT*)&KernelThreadManager,
				lpCurrent);  //Insert into ready queue.

			lpNew->dwThreadStatus = KERNEL_THREAD_STATUS_RUNNING;
			lpNew->dwTotalRunTime += SYSTEM_TIME_SLICE;
			KernelThreadManager.lpCurrentKernelThread = lpNew;
		
			//Call schedule hook before swich.
			KernelThreadManager.CallThreadHook(
				THREAD_HOOK_TYPE_ENDSCHEDULE | THREAD_HOOK_TYPE_BEGINSCHEDULE,
				lpCurrent,lpNew);

			//Depend on ARCH
			//__SaveAndSwitch(&lpCurrent->lpKernelThreadContext, &lpNew->lpKernelThreadContext);  //Switch.
			__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
			return;
		}
		break;

	case KERNEL_THREAD_STATUS_READY:

		lpNew = KernelThreadManager.GetScheduleKernelThread(
			(struct __COMMON_OBJECT*)&KernelThreadManager,
			lpCurrent->dwThreadPriority);  //Get a ready thread.

		if(NULL == lpNew)  //Should not occur.
		{
			__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
			BUG();
			return;
		}

		if(lpNew == lpCurrent)  //The same one.
		{
			lpCurrent->dwTotalRunTime += SYSTEM_TIME_SLICE;
			lpCurrent->dwThreadStatus = KERNEL_THREAD_STATUS_RUNNING;
			__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
			return;
		}
		else  //Not the same one.
		{
			lpNew->dwTotalRunTime += SYSTEM_TIME_SLICE;
			lpNew->dwThreadStatus = KERNEL_THREAD_STATUS_RUNNING;
			KernelThreadManager.lpCurrentKernelThread = lpNew;

			//Call schedule hook routine.
			KernelThreadManager.CallThreadHook(
				THREAD_HOOK_TYPE_ENDSCHEDULE | THREAD_HOOK_TYPE_BEGINSCHEDULE,
				lpCurrent,lpNew);

			//Depend on ARCH
			//__SaveAndSwitch(&lpCurrent->lpKernelThreadContext,&lpNew->lpKernelThreadContext);
			__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
			return;
		}
		break;

	case KERNEL_THREAD_STATUS_BLOCKED:
	case KERNEL_THREAD_STATUS_SUSPENDED:
	case KERNEL_THREAD_STATUS_TERMINAL:
	case KERNEL_THREAD_STATUS_SLEEPING:

		lpNew = KernelThreadManager.GetScheduleKernelThread(
			(struct __COMMON_OBJECT*)&KernelThreadManager,
			0L);  //Get a ready thread to run.

		if(NULL == lpNew)
		{
			__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
			BUG();
			printf("Fatal error: in ScheduleFromProc,lpNew == NULL.");
			return;
		}
		lpNew->dwThreadStatus = KERNEL_THREAD_STATUS_RUNNING;
		lpNew->dwTotalRunTime += SYSTEM_TIME_SLICE;
		KernelThreadManager.lpCurrentKernelThread = lpNew;

		//Call schedule hook.
		KernelThreadManager.CallThreadHook(
			THREAD_HOOK_TYPE_ENDSCHEDULE | THREAD_HOOK_TYPE_BEGINSCHEDULE,
			lpCurrent,lpNew);
		//Depend on ARCH
		//__SaveAndSwitch(&lpCurrent->lpKernelThreadContext, &lpNew->lpKernelThreadContext);
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

		return;
		break;
	default:
		BUG();
		break;
	}
}

//ScheduleFromInt's implementation.
static VOID ScheduleFromInt(void)
//struct __COMMON_OBJECT* lpThis, LPVOID lpESP
{

	struct __KERNEL_THREAD_OBJECT*         lpNextThread    = NULL;
	struct __KERNEL_THREAD_OBJECT*         lpCurrentThread = NULL;

	//struct __KERNEL_THREAD_MANAGER*        lpMgr           = NULL;

	//if((NULL == lpThis) || (NULL == lpESP))    //Parameters check.
	//	return;

	//lpMgr = (struct __KERNEL_THREAD_MANAGER*)lpThis;
/*
	if(NULL == lpMgr->lpCurrentKernelThread)   //The routine is called first time.
	{

		lpNextThread = KernelThreadManager.GetScheduleKernelThread(
			(struct __COMMON_OBJECT*)&KernelThreadManager,
			0L);

		//If this case is occurs,the system is crash.
		if(NULL == lpNextThread)               
		{
			BUG();
			return;
		}

		KernelThreadManager.lpCurrentKernelThread = lpNextThread;
		lpNextThread->dwThreadStatus = KERNEL_THREAD_STATUS_RUNNING;
		lpNextThread->dwTotalRunTime += SYSTEM_TIME_SLICE;
		//Call schedule hook.
		KernelThreadManager.CallThreadHook(
			THREAD_HOOK_TYPE_BEGINSCHEDULE,NULL,lpNextThread);

		//Depend or ARCH
		//__SwitchTo(lpNextThread->lpKernelThreadContext);  //Switch to this thread.
	}
	else  //Not the first time be called.
*/

	lpCurrentThread = KernelThreadManager.lpCurrentKernelThread;
	//This code line saves the context of current kernel thread.

	//Depend or ARCH
	//lpCurrentThread->lpKernelThreadContext = (struct __KERNEL_THREAD_CONTEXT*)lpESP;

	switch(lpCurrentThread->dwThreadStatus)
	{
	case KERNEL_THREAD_STATUS_BLOCKED:     //Waiting shared object in process.
	case KERNEL_THREAD_STATUS_TERMINAL:    //In process of termination.
	case KERNEL_THREAD_STATUS_SLEEPING:    //In process of falling in sleep.
	case KERNEL_THREAD_STATUS_SUSPENDED:   //In process of being suspended.
	case KERNEL_THREAD_STATUS_READY:       //Wakeup immediately in another interrupt.
		lpCurrentThread->dwTotalRunTime += SYSTEM_TIME_SLICE;
		//lpContext = lpCurrentThread->lpKernelThreadContext;
		KernelThreadManager.CallThreadHook(
			THREAD_HOOK_TYPE_BEGINSCHEDULE,NULL,lpCurrentThread);

		//Depend or ARCH
		LPVOID pointer = lpCurrentThread->lpInitStackPointer;
		printf("lpCurrentThread->lpInitStackPointer = %x\n", pointer);
		__SwitchTo(pointer);
		break;                             //This instruction will never reach.

	case KERNEL_THREAD_STATUS_RUNNING:
		lpNextThread = KernelThreadManager.GetScheduleKernelThread(
			(struct __COMMON_OBJECT*)&KernelThreadManager,
			lpCurrentThread->dwThreadPriority);

		printf("lpInitStackPointer %x %x\n",lpCurrentThread->lpInitStackPointer, lpNextThread->lpInitStackPointer);
		if(NULL == lpNextThread || lpNextThread == lpCurrentThread)  //Current is most priority.
		{
			lpCurrentThread->dwTotalRunTime += SYSTEM_TIME_SLICE;
			printf("lpCurrentThread->dwTotalRunTime = %d\n", lpCurrentThread->dwTotalRunTime);
			//KernelThreadManager.CallThreadHook(
			//	THREAD_HOOK_TYPE_BEGINSCHEDULE,NULL,lpCurrentThread);
			//Depend or ARCH
			//LPVOID pointer = lpCurrentThread->lpInitStackPointer;
			//printf("lpCurrentThread->lpInitStackPointer = %x\n", pointer);
			//__SwitchTo(pointer);
			return;
		}
		else
		{

			lpCurrentThread->dwThreadStatus = KERNEL_THREAD_STATUS_READY;
			KernelThreadManager.AddReadyKernelThread(
				(struct __COMMON_OBJECT*)&KernelThreadManager,
				lpCurrentThread);  //Add to ready queue.

			lpNextThread->dwTotalRunTime += SYSTEM_TIME_SLICE;
			lpNextThread->dwThreadStatus = KERNEL_THREAD_STATUS_RUNNING;
			KernelThreadManager.lpCurrentKernelThread = lpNextThread;
			
			//KernelThreadManager.CallThreadHook(
			//	THREAD_HOOK_TYPE_BEGINSCHEDULE,NULL,lpNextThread);
			
			//Depend or ARCH
			LPVOID pointer = lpNextThread->lpInitStackPointer;
#ifdef  DEBUG
			printf("lpNextThread->lpInitStackPointer = %x\n", pointer);
#endif			
			if (pointer>0xB0000000) while (1) ;
			__SwitchTo(pointer);
			return;
		}
	default:
		BUG();
		break;
	}
}

//SetThreadPriority.
static DWORD SetThreadPriority(struct __COMMON_OBJECT* lpKernelThread,DWORD dwPriority)
{
	struct __KERNEL_THREAD_OBJECT*    lpThread = NULL;
	DWORD                      dwOldPri = PRIORITY_LEVEL_IDLE;
	DWORD                      dwFlags  = 0L;

	if(NULL == lpKernelThread)
		return PRIORITY_LEVEL_IDLE;
	
	lpThread = (struct __KERNEL_THREAD_OBJECT*)lpKernelThread;
	dwOldPri = lpThread->dwThreadPriority;
	//ENTER_CRITICAL_SECTION();
	__ENTER_CRITICAL_SECTION(NULL,dwFlags)
	lpThread->dwThreadPriority = dwPriority;
	//LEAVE_CRITICAL_SECTION();
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags)

	return dwOldPri;
}

//GetThreadPriority.
static DWORD GetThreadPriority(struct __COMMON_OBJECT* lpKernelThread)
{
	struct __KERNEL_THREAD_OBJECT*    lpThread = NULL;

	if(NULL == lpKernelThread)
		return PRIORITY_LEVEL_IDLE;

	return ((struct __KERNEL_THREAD_OBJECT*)lpKernelThread)->dwThreadPriority;
}

//TerminalKernelThread.
static DWORD TerminalKernelThread(struct __COMMON_OBJECT* lpThis,struct __COMMON_OBJECT* lpKernelThread)
{
	return 0L;
}

//
//Sleep Routine.
//This routine do the following:
// 1. Updates the dwNextWakeupTick value of kernel thread manager;
// 2. Modifies the current kernel thread's status to SLEEPING;
// 3. Puts the current kernel thread into sleeping queue of kernel thread manager;
// 4. Schedules another kernel thread to.
//

static BOOL Sleep(struct __COMMON_OBJECT* lpThis,//struct __COMMON_OBJECT* lpKernelThread,
						DWORD dwMillisecond)
{
	struct __KERNEL_THREAD_MANAGER*           lpManager      = NULL;
	struct __KERNEL_THREAD_OBJECT*            lpKernelThread = NULL;
	DWORD                              dwTmpCounter   = 0L;
	//struct __KERNEL_THREAD_CONTEXT*           lpContext      = NULL;
	DWORD                              dwFlags        = 0L;

	if((NULL == lpThis) ||
	  (dwMillisecond < SYSTEM_TIME_SLICE))    //Parameters check.
	  return FALSE;

	lpManager = (struct __KERNEL_THREAD_MANAGER*) lpThis;

	lpKernelThread = lpManager->lpCurrentKernelThread;

	if(NULL == lpKernelThread)    //The routine is called in system initializing process.
	{
		//__ERROR_HANDLER(ERROR_LEVEL_CRITICAL,0L,NULL);
		return FALSE;
	}

	dwTmpCounter =  dwMillisecond / SYSTEM_TIME_SLICE;
	//dwTmpCounter += System.dwClockTickCounter;   //Now,dwTmpCounter countains the 
	                                                   //tick counter value when this
	                                                   //kernel thread who calls this routine
	                                                   //must be waken up.
	//ENTER_CRITICAL_SECTION();
	__ENTER_CRITICAL_SECTION(NULL,dwFlags)

	if((0 == lpManager->dwNextWakeupTick) ||  (lpManager->dwNextWakeupTick > dwTmpCounter))
	   lpManager->dwNextWakeupTick = dwTmpCounter;     //Update dwNextWakeupTick value.

	lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_SLEEPING;

	dwTmpCounter = MAX_DWORD_VALUE - dwTmpCounter;     //Calculates the priority of the
	                                                   //current kernel thread in the sleeping
	                                                   //queue.

	lpManager->lpSleepingQueue->InsertIntoQueue((struct __COMMON_OBJECT*)lpManager->lpSleepingQueue,
		(struct __COMMON_OBJECT*)lpKernelThread,
		dwTmpCounter);

	__LEAVE_CRITICAL_SECTION(NULL,dwFlags)

	//lpContext = &lpKernelThread->KernelThreadContext;

	lpManager->ScheduleFromProc();

	return TRUE;
}

//CancelSleep Routine.
static BOOL CancelSleep(struct __COMMON_OBJECT* lpThis,struct __COMMON_OBJECT* lpKernelThread)
{
	return FALSE;
}

//SetCurrentIRQL.
static DWORD SetCurrentIRQL(struct __COMMON_OBJECT* lpThis,DWORD dwNewIRQL)
{
	return 0L;
}

//GetCurrentIRQL.
static DWORD GetCurrentIRQL(struct __COMMON_OBJECT* lpThis)
{
	return 0L;
}

//SetLastError.
static DWORD SetLastError(/*struct __COMMON_OBJECT* lpKernelThread,*/DWORD dwNewError)
{
	DWORD  dwOldError = 0L;

	dwOldError = KernelThreadManager.lpCurrentKernelThread->dwLastError;
	KernelThreadManager.lpCurrentKernelThread->dwLastError = dwNewError;
	return dwOldError;
}

//GetLastError.
static DWORD GetLastError(/*struct __COMMON_OBJECT* lpKernelThread*/)
{
	return KernelThreadManager.lpCurrentKernelThread->dwLastError;
}

//GetThreadID.
static DWORD GetThreadID(struct __COMMON_OBJECT* lpKernelThread)
{
	if(NULL == lpKernelThread)
		return 0L;

	return ((struct __KERNEL_THREAD_OBJECT*)lpKernelThread)->dwThreadID;
}

//GetThreadStatus.
static DWORD GetThreadStatus(struct __COMMON_OBJECT* lpKernelThread)
{
	if(NULL == lpKernelThread)
		return 0L;

	return ((struct __KERNEL_THREAD_OBJECT*)lpKernelThread)->dwThreadStatus;
}

//SetThreadStatus.
static DWORD SetThreadStatus(struct __COMMON_OBJECT* lpKernelThread,DWORD dwStatus)
{
	return 0L;
}

/*
//MsgQueueFull.
static BOOL MsgQueueFull(struct __COMMON_OBJECT* lpThread)
{
	struct __KERNEL_THREAD_OBJECT*     lpKernelThread  = NULL;

	if(NULL == lpThread)    //Parameter check.
		return FALSE;

	lpKernelThread = (struct __KERNEL_THREAD_OBJECT*)lpThread;

	return MAX_KTHREAD_MSG_NUM == lpKernelThread->ucCurrentMsgNum ? TRUE : FALSE;
}
*/

//MsgQueueEmpty.
static BOOL MsgQueueEmpty(struct __COMMON_OBJECT* lpThread)
{
	struct __KERNEL_THREAD_OBJECT*     lpKernelThread = NULL;

	if(NULL == lpThread)   //Parameter check.
		return FALSE;

	lpKernelThread = (struct __KERNEL_THREAD_OBJECT*)  lpThread;

	return 0 == lpKernelThread->ucCurrentMsgNum ? TRUE : FALSE;
}

/*
//SendMessage.
static BOOL MgrSendMessage(struct __COMMON_OBJECT* lpThread,struct __KERNEL_THREAD_MESSAGE* lpMsg)
{
	struct __KERNEL_THREAD_OBJECT*     lpKernelThread = NULL;
	struct __KERNEL_THREAD_OBJECT*     lpNewThread    = NULL;
	BOOL                        bResult        = FALSE;
	DWORD                       dwFlags        = 0L;

	if((NULL == lpThread) || (NULL == lpMsg)) //Parameters check.
		return bResult;
	lpKernelThread = (struct __KERNEL_THREAD_OBJECT*)lpThread;

	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	if(MsgQueueFull(lpThread))             //Message queue is full.
	{
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		return bResult;
	}
	//Message queue not full,put the message to the queue.
	lpKernelThread->KernelThreadMsg[lpKernelThread->ucMsgQueueTrial].wCommand
		= lpMsg->wCommand;
	lpKernelThread->KernelThreadMsg[lpKernelThread->ucMsgQueueTrial].wParam
		= lpMsg->wParam;
	lpKernelThread->KernelThreadMsg[lpKernelThread->ucMsgQueueTrial].dwParam
		= lpMsg->dwParam;
	lpKernelThread->ucMsgQueueTrial ++;
	if(MAX_KTHREAD_MSG_NUM == lpKernelThread->ucMsgQueueTrial)
		lpKernelThread->ucMsgQueueTrial = 0;
	lpKernelThread->ucCurrentMsgNum ++;

	lpNewThread = (struct __KERNEL_THREAD_OBJECT*)lpKernelThread->lpMsgWaitingQueue->GetHeaderElement(
		(struct __COMMON_OBJECT*)lpKernelThread->lpMsgWaitingQueue,
		NULL);
	if(lpNewThread)  //Should wakeup the target kernel thread.
	{
		lpNewThread->dwThreadStatus = KERNEL_THREAD_STATUS_READY;
		KernelThreadManager.AddReadyKernelThread(
			(struct __COMMON_OBJECT*)&KernelThreadManager,
			lpNewThread);  //Add to ready queue.
	}
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	//
	//If in kernel thread context,then re-schedule kernel thread.
	//
	if(IN_KERNELTHREAD())  //---- !!!!!!!! PROBLEM CAUSED !!!!!!!! ----
	{
		KernelThreadManager.ScheduleFromProc(NULL);
	}
	bResult = TRUE;
	return bResult;
}
*/

//GetMessage.
static BOOL MgrGetMessage(struct __COMMON_OBJECT* lpThread,struct __KERNEL_THREAD_MESSAGE* lpMsg)
{
	struct __KERNEL_THREAD_OBJECT*     lpKernelThread  = NULL;
	DWORD                       dwFlags         = 0L;

	if((NULL == lpThread) || (NULL == lpMsg))   //Parameters check.
		return FALSE;

	lpKernelThread = (struct __KERNEL_THREAD_OBJECT*) lpThread;

	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	if(MsgQueueEmpty(lpThread))  //Current message queue is empty,should waiting.
	{
		lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_BLOCKED;
		lpKernelThread->lpMsgWaitingQueue->InsertIntoQueue(
			(struct __COMMON_OBJECT*)lpKernelThread->lpMsgWaitingQueue,
			(struct __COMMON_OBJECT*)lpKernelThread, 0L);
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

		KernelThreadManager.ScheduleFromProc();  //Re-schedule.
		//lpKernelThread->lpMsgEvent->WaitForThisObject(
		//	(struct __COMMON_OBJECT*)(lpKernelThread->lpMsgEvent));  //Block the current thread.
	}
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

	__ENTER_CRITICAL_SECTION(NULL,dwFlags)
	lpMsg->wCommand     = lpKernelThread->KernelThreadMsg[lpKernelThread->ucMsgQueueHeader].wCommand;
	lpMsg->wParam       = lpKernelThread->KernelThreadMsg[lpKernelThread->ucMsgQueueHeader].wParam;
	lpMsg->dwParam      = lpKernelThread->KernelThreadMsg[lpKernelThread->ucMsgQueueHeader].dwParam;

	lpKernelThread->ucMsgQueueHeader ++;
	if(MAX_KTHREAD_MSG_NUM == lpKernelThread->ucMsgQueueHeader)
		lpKernelThread->ucMsgQueueHeader = 0x0000;
	lpKernelThread->ucCurrentMsgNum --;

	//if(0 == lpKernelThread->ucCurrentMsgNum)     //The message queue is empty.
	//	lpKernelThread->lpMsgEvent->ResetEvent((struct __COMMON_OBJECT*)(lpKernelThread->lpMsgEvent));
	//LEAVE_CRITICAL_SECTION();                    //Enable the interrupt.
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	return TRUE;
}

//
//The following routine is used to lock a kernel thread,especially the current kernel thread.
//When a kernel thread is locked,it can not be interrupted,even a kernel thread with high priority
//ready to run.The different between lock kernel thread and lock interrupt is,when interrupt is 
//locked,the system will never be interrupted by hardware,and the schedule will never occur,because
//timer interrupt also be locked,but when lock a kernel thread,only disables the schedule of the
//system,interrupt is not locked,so,hardware interrupt can also be processed by system.
//CAUTION: When lock a kernel thread,you must unlock it by calling UnlockKernelThread routine to
//unlock the kernel thread,otherwise,others kernel thread(s) will never be scheduled.
//

static BOOL LockKernelThread(struct __COMMON_OBJECT* lpThis,struct __COMMON_OBJECT* lpThread)
{
	struct __KERNEL_THREAD_MANAGER*                   lpManager        = NULL;
	struct __KERNEL_THREAD_OBJECT*                    lpKernelThread   = NULL;
	DWORD                                      dwFlags          = 0L;

	if(NULL == lpThis)    //Parameter check.
		return FALSE;

	lpManager = (struct __KERNEL_THREAD_MANAGER*)lpThis;

	//ENTER_CRITICAL_SECTION();
	__ENTER_CRITICAL_SECTION(NULL,dwFlags)
	lpKernelThread = (NULL == lpThread) ? lpManager->lpCurrentKernelThread : 
	(struct __KERNEL_THREAD_OBJECT*)lpThread;    //If lpThread is NULL,then lock the current kernel thread.

	if(KERNEL_THREAD_STATUS_RUNNING != lpKernelThread->dwThreadStatus)
	{
		//LEAVE_CRITICAL_SECTION();
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags)
		return FALSE;
	}

	lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_BLOCKED;    //Once mark the status of
	                                                                  //the target thread to
	                                                                  //BLOCKED,it will never be
	                                                                  //switched out.
	//LEAVE_CRITICAL_SECTION();
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags)
	return TRUE;
}

//
//The following routine unlockes a kernel thread who is locked by LockKernelThread routine.
//

static VOID UnlockKernelThread(struct __COMMON_OBJECT* lpThis,struct __COMMON_OBJECT* lpThread)
{
	struct __KERNEL_THREAD_MANAGER*               lpManager       = NULL;
	struct __KERNEL_THREAD_OBJECT*                lpKernelThread  = NULL;
	DWORD                                  dwFlags         = 0L;

	if(NULL == lpThis)    //Parameter check.
		return;

	lpManager = (struct __KERNEL_THREAD_MANAGER*)lpThis;

	//ENTER_CRITICAL_SECTION();
	__ENTER_CRITICAL_SECTION(NULL,dwFlags)
	lpKernelThread = (NULL == lpThread) ? lpManager->lpCurrentKernelThread : 
	(struct __KERNEL_THREAD_OBJECT*)lpThread;
	if(KERNEL_THREAD_STATUS_BLOCKED != lpKernelThread->dwThreadStatus)  //If not be locked.
	{
		//LEAVE_CRITICAL_SECTION();
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		return;
	}
	lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_RUNNING;
	//LEAVE_CRITICAL_SECTION();
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags)
	return;
}

/**************************************************************
***************************************************************
***************************************************************
**************************************************************/
//
//The definition of Kernel Thread Manager.
//

struct __KERNEL_THREAD_MANAGER KernelThreadManager = 
{
	0L,                                              //dwCurrentIRQL.
	NULL,                                            //CurrentKernelThread.

	NULL,                                            //lpRunningQueue.
	//NULL,                                            //lpReadyQueue.
	NULL,                                            //lpSuspendedQueue.
	NULL,                                            //lpSleepingQueue.
	NULL,                                            //lpTerminalQueue.

	{0},                                             //Ready queue array.
	//0L,                                              //dwClockTickCounter.
	0L,                                              //dwNextWakeupTick.

	NULL,                                            //lpCreateHook.
	NULL,                                            //lpEndScheduleHook.
	NULL,                                            //lpBeginScheduleHook.
	NULL,                                            //lpTerminalHook.
	SetThreadHook,                                   //SetThreadHook.
	CallThreadHook,                                  //CallThreadHook.

	GetScheduleKernelThread,                         //GetScheduleKernelThread.
	AddReadyKernelThread,                            //AddReadyKernelThread.
	KernelThreadMgrInit,                             //Initialize routine.

	CreateKernelThread,                              //CreateKernelThread routine.
	DestroyKernelThread,                             //DestroyKernelThread routine.

	SuspendKernelThread,                             //SuspendKernelThread routine.
	ResumeKernelThread,                              //ResumeKernelThread routine.

	ScheduleFromProc,                                //ScheduleFromProc routine.
	ScheduleFromInt,                                 //ScheduleFromInt routine.

	SetThreadPriority,                               //SetThreadPriority routine.
	GetThreadPriority,                               //GetThreadPriority routine.

	TerminalKernelThread,                            //TerminalKernelThread routine.
	//Sleep,                                           //Sleep routine.
	//CancelSleep,                                     //CancelSleep routine.

	SetCurrentIRQL,                                  //SetCurrentIRQL routine.
	GetCurrentIRQL,                                  //GetCurrentIRQL routine.

	GetLastError,                                    //GetLastError routine.
	SetLastError,                                    //SetLastError routine.

	GetThreadID,                                     //GetThreadID routine.

	GetThreadStatus,                                 //GetThreadStatus routine.
	SetThreadStatus,                                 //SetThreadStatus routine.

	//MgrSendMessage,                                  //SendMessage routine.
	//MgrGetMessage,                                   //GetMessage routine.
	//MsgQueueFull,                                    //MsgQueueFull routine.
	//MsgQueueEmpty,                                   //MsgQueueEmpty routine.
	LockKernelThread,                                //LockKernelThread routine.
	UnlockKernelThread                               //UnlockKernelThread routine.
};

//
/**************************************************************************************
****************************************************************************************
****************************************************************************************
****************************************************************************************
***************************************************************************************/
//
//Dispatch a message to an message(event) handler.
//

DWORD DispatchMessage(struct __KERNEL_THREAD_MESSAGE* lpMsg, __KERNEL_THREAD_MESSAGE_HANDLER lpHandler)
{
	return lpHandler(lpMsg->wCommand, lpMsg->wParam, lpMsg->dwParam);
}


