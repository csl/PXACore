//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Nov 04, 2004
//    Module Name               : synobj.cpp
//    Module Funciton           : 
//                                This module countains synchronization object's implementation
//                                code.
//                                The following synchronization object(s) is(are) implemented
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

#include "StdAfx.h"

//
//Previous declare of static routine.
//
static DWORD WaitForEventObject(__COMMON_OBJECT*);
static DWORD SetEvent(__COMMON_OBJECT*);
static DWORD ResetEvent(__COMMON_OBJECT*);

//
//Helper functions.
//

__declspec(naked) static VOID SwitchTo(__KERNEL_THREAD_CONTEXT* lpContext)
{
#ifdef __I386__                           //Intel's x86 CPU implementation.
	__asm{
		cli
		push ebp
		mov ebp,esp                                          //Build the stack frame.
		mov eax,dword ptr [ebp + 0x08]                       //Now,eax countains the lpContext value.

		mov esp,dword ptr [eax + CONTEXT_OFFSET_ESP]         //Restore the ESP register first.
		push dword ptr [eax + CONTEXT_OFFSET_EFLAGS]         //Push EFlags register to stack.
		xor ebx,ebx
		mov bx,word ptr [eax + CONTEXT_OFFSET_CS]
		push ebx                                             //Extend and push CS to stack.
		push dword ptr [eax + CONTEXT_OFFSET_EIP]            //Push EIP to stack.
		                                                     //Now,we have built the correct 
															 //stack frame.

	    push dword ptr [eax + CONTEXT_OFFSET_EAX]            //Save eax to stack.
		push dword ptr [eax + CONTEXT_OFFSET_EBX]            //Save ebx to stack.
		push dword ptr [eax + CONTEXT_OFFSET_ECX]            //ecx
		push dword ptr [eax + CONTEXT_OFFSET_EDX]            //edx
		push dword ptr [eax + CONTEXT_OFFSET_ESI]            //esi
		push dword ptr [eax + CONTEXT_OFFSET_EDI]            //edi
		push dword ptr [eax + CONTEXT_OFFSET_EBP]            //ebp

		mov ebp,dword ptr [esp]                 //Restore the ebp register.
		add esp,0x04

		mov edi,dword ptr [esp]                 //Restore edi
		add esp,0x04

		mov esi,dword ptr [esp]                 //Restore esi
		add esp,0x04

		mov edx,dword ptr [esp]                 //Restore edx
		add esp,0x04

		mov ecx,dword ptr [esp]                 //Restore ecx
		add esp,0x04

		mov ebx,dword ptr [esp]                 //Restore ebx
		add esp,0x04                            
		
		mov al,0x20
		out 0x20,al
		out 0xa0,al

		mov eax,dword ptr [esp]                 
		add esp,0x04                            //Now,the esp pointer points to the current
		                                        //position.
		sti
		iretd
		retn                                   //This instruction will never be reached.
	}
#else
#endif
}

//
//Event object's initializing routine.
//This routine initializes the members of an event object.
//

BOOL EventInitialize(__COMMON_OBJECT* lpThis)
{
	BOOL                  bResult          = FALSE;
	__EVENT*              lpEvent          = NULL;
	__PRIORITY_QUEUE*     lpPriorityQueue  = NULL;

	if(NULL == lpThis)
		goto __TERMINAL;

	lpEvent = (__EVENT*)lpThis;

	lpPriorityQueue = (__PRIORITY_QUEUE*)
		ObjectManager.CreateObject(&ObjectManager,NULL,
		OBJECT_TYPE_PRIORITY_QUEUE);
	if(NULL == lpPriorityQueue)
		goto __TERMINAL;

	bResult = lpPriorityQueue->Initialize((__COMMON_OBJECT*)lpPriorityQueue);
	if(!bResult)
		goto __TERMINAL;

	lpEvent->lpWaitingQueue     = lpPriorityQueue;
	lpEvent->dwEventStatus      = EVENT_STATUS_OCCUPIED;
	lpEvent->SetEvent           = SetEvent;
	lpEvent->ResetEvent         = ResetEvent;
	lpEvent->WaitForThisObject  = WaitForEventObject;
	bResult                     = TRUE;

__TERMINAL:
	return bResult;
}

//
//Event object's uninitializing routine.
//This routine calles priority queue's uninitialize routine to release
//priority queue's resource.
//

VOID EventUninitialize(__COMMON_OBJECT* lpThis)
{
	__EVENT*            lpEvent          = NULL;
	__PRIORITY_QUEUE*   lpPriorityQueue  = NULL;

	if(NULL == lpThis)
		return;

	lpEvent = (__EVENT*)lpThis;
	lpPriorityQueue = lpEvent->lpWaitingQueue;
	if(NULL != lpPriorityQueue)
		lpPriorityQueue->Uninitialize((__COMMON_OBJECT*)lpPriorityQueue);
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

static DWORD SetEvent(__COMMON_OBJECT* lpThis)
{
	DWORD                     dwPreviousStatus     = EVENT_STATUS_OCCUPIED;
	__EVENT*                  lpEvent              = NULL;
	__KERNEL_THREAD_OBJECT*   lpKernelThread       = NULL;

	if(NULL == lpThis)
		return dwPreviousStatus;

	lpEvent = (__EVENT*)lpThis;
	dwPreviousStatus = lpEvent->dwEventStatus;
	DisableInterrupt();
	lpEvent->dwEventStatus = EVENT_STATUS_FREE;    //Set the current status to free.
	EnableInterrupt();
	lpKernelThread = (__KERNEL_THREAD_OBJECT*)
		lpEvent->lpWaitingQueue->GetHeaderElement(
		(__COMMON_OBJECT*)lpEvent->lpWaitingQueue,
		NULL);
	while(lpKernelThread)                         //Remove all kernel thread(s) from
		                                          //waiting queue,and insert them into
												  //ready queue of kernel thread,so,they
												  //would be scheduled in appropriate
												  //time.
	{
		lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_READY;
		KernelThreadManager.lpReadyQueue->InsertIntoQueue(
			(__COMMON_OBJECT*)KernelThreadManager.lpReadyQueue,
			(__COMMON_OBJECT*)lpKernelThread,
			lpKernelThread->dwScheduleCounter
			);
		lpKernelThread = (__KERNEL_THREAD_OBJECT*)
			lpEvent->lpWaitingQueue->GetHeaderElement(
			(__COMMON_OBJECT*)lpEvent->lpWaitingQueue,
			NULL);
	}
	return dwPreviousStatus;
}

//
//The implementation of ResetEvent.
//

static DWORD ResetEvent(__COMMON_OBJECT* lpThis)
{
	__EVENT*          lpEvent          = NULL;
	DWORD             dwPreviousStatus = 0L;

	if(NULL == lpThis)
		return dwPreviousStatus;

	lpEvent = (__EVENT*)lpThis;
	dwPreviousStatus = lpEvent->dwEventStatus;
	DisableInterrupt();
	lpEvent->dwEventStatus = EVENT_STATUS_OCCUPIED;
	EnableInterrupt();

	return dwPreviousStatus;
}

//
//The implementation of WaitForEventObject.
//

static DWORD WaitForEventObject(__COMMON_OBJECT* lpThis)
{
	__EVENT*                      lpEvent             = NULL;
	__KERNEL_THREAD_OBJECT*       lpKernelThread      = NULL;
	__KERNEL_THREAD_CONTEXT*      lpContext           = NULL;

	if(NULL == lpThis)
		return 0L;

	lpEvent = (__EVENT*)lpThis;
	if(EVENT_STATUS_FREE == lpEvent->dwEventStatus)
		return 1L;
	else
	{
		lpKernelThread = KernelThreadManager.lpCurrentKernelThread;
		DisableInterrupt();
		lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_BLOCKED;
		EnableInterrupt();
		lpEvent->lpWaitingQueue->InsertIntoQueue(
			(__COMMON_OBJECT*)lpEvent->lpWaitingQueue,
			(__COMMON_OBJECT*)lpKernelThread,
			0L);
		lpContext = &lpKernelThread->KernelThreadContext;
		//SwitchTo(lpContext);
		KernelThreadManager.ScheduleFromProc(lpContext);
		//__asm{                  //-------- ** debug ** ------------
		//	mov eax,0x09099090
		//	mov ebx,0x09099090
		//}
	}
	return 0L;
}

//
//One kernel thread can use the following steps to create an event object:
// 1. Create an event object by calling CreateObject;
// 2. Initialize the event object by calling the event object's initialize routine;
// 3. Set the event object's initializing status by calling SetEvent or ResetEvent.
//In order to make this easy,we define a wrap routine,CreatEvent,to wrap these steps
//as one step.
//

__COMMON_OBJECT* CreateEvent(BOOL bInitialState,LPVOID lpReserved)
{
	__COMMON_OBJECT*         lpCommonObject    = NULL;
	__EVENT*                 lpEvent           = NULL;
	DWORD                    dwInitialState    = 0L;

	lpCommonObject = ObjectManager.CreateObject(&ObjectManager,
		NULL,
		OBJECT_TYPE_EVENT);
	if(NULL == lpCommonObject)
		goto __TERMINAL;

	if(!lpCommonObject->Initialize(lpCommonObject))
	{
		ObjectManager.DestroyObject(&ObjectManager,lpCommonObject);
		lpCommonObject = NULL;
		goto __TERMINAL;
	}

	lpEvent = (__EVENT*)lpCommonObject;
	if(bInitialState)
		lpEvent->SetEvent((__COMMON_OBJECT*)lpEvent);

__TERMINAL:
	return lpCommonObject;
}

VOID DestroyEvent(__COMMON_OBJECT* lpThis)
{
	ObjectManager.DestroyObject(&ObjectManager,lpThis);
}

////////////////////////////////////////////////////////////////////////////////////
//
//*******************************************************************************//
//
///////////////////////////////////////////////////////////////////////////////////

//
//The implementation of ReleaseMutex.
//

static DWORD ReleaseMutex(__COMMON_OBJECT* lpThis)
{
	return 0L;
}

//
//The implementation of WaitForMutexObject.
//

static DWORD WaitForMutexObject(__COMMON_OBJECT* lpThis)
{
	return 0L;
}

//
//The implementation of MutexInitialize.
//

BOOL MutexInitialize(__COMMON_OBJECT* lpThis)
{
	return FALSE;
}

//
//The implementation of MutexUninitialize.
//

VOID MutexUninitialize(__COMMON_OBJECT* lpThis)
{
	return;
}

//
//The implementation of CreateMutex.
//

__COMMON_OBJECT* CreateMutex(LPVOID lpReserved)
{
	return NULL;
}

//
//The implementation of DestroyMutex.
//

VOID DestroyMutex(__COMMON_OBJECT* lpThis)
{
	return;
}

