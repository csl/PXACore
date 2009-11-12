//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Nov,06 2004
//    Module Name               : system.cpp
//    Module Funciton           : 
//                                This module countains system mechanism releated objects's
//                                implementation..
//                                Including the following aspect:
//                                1. Interrupt object and interrupt management code;
//                                2. Timer object and timer management code;
//                                3. System level parameters management coee,such as
//                                   physical memory,system time,etc;
//                                4. Other system mechanism releated objects.
//
//                                ************
//                                This file is one of the most important file of Hello Taiwan.
//                                ************
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include "stdafx.h"

__PERF_RECORDER  TimerIntPr = 
{
	U64_ZERO,
	U64_ZERO,
	U64_ZERO,
	U64_ZERO
};                  //Performance recorder object used to mesure
                                //the performance of timer interrupt.

//
//TimerInterruptHandler routine.
//The following routine is the most CRITICAL routine of kernel of Hello Taiwan.
//The routine does the following:
// 1. Schedule timer object;
// 2. Update the system level variables,such as dwClockTickCounter;
// 3. Schedule kernel thread(s).
//

static BOOL TimerInterruptHandler(LPVOID lpEsp,LPVOID)
{
	DWORD                     dwPriority        = 0L;
	struct __TIMER_OBJECT*    lpTimerObject     = 0L;
	__KERNEL_THREAD_MESSAGE   Msg;
	struct __PRIORITY_QUEUE*         lpTimerQueue      = NULL;
	struct __PRIORITY_QUEUE*         lpSleepingQueue   = NULL;
	struct __KERNEL_THREAD_OBJECT*   lpKernelThread    = NULL;
	DWORD                     dwFlags           = 0L;

	if(NULL == lpEsp)    //Parameter check.
		return TRUE;

	if(System.dwClockTickCounter == System.dwNextTimerTick)     //Should schedule timer.
	{
		lpTimerQueue = System.lpTimerQueue;
		lpTimerObject = (struct __TIMER_OBJECT*)lpTimerQueue->GetHeaderElement(
				(struct __COMMON_OBJECT*)lpTimerQueue,
				&dwPriority);
		if(NULL == lpTimerObject) goto __CONTINUE_1;

		dwPriority = MAX_DWORD_VALUE - dwPriority;
		while(dwPriority <= System.dwNextTimerTick)    //Strictly speaking,the dwPriority
			                                       //variable must EQUAL System.dw-
							       //NextTimerTick,but in the implement-
							      //ing of the current version,there
							      //may be some error exists,so we assume
							      //dwPriority equal or less than dwNext-
							      //TimerTic.
		{
			if(NULL == lpTimerObject->DirectTimerHandler)  //Send a message to the kernel thread.
			{
				Msg.wCommand = KERNEL_MESSAGE_TIMER;
				Msg.dwParam  = lpTimerObject->dwTimerID;
				KernelThreadManager.SendMessage(
					(struct __COMMON_OBJECT*)lpTimerObject->lpKernelThread,
					&Msg);
				//printf("Send a timer message to kernel thread.");
			}
			else
			{
				lpTimerObject->DirectTimerHandler(
					lpTimerObject->lpHandlerParam);    //Call the associated handler.
			}

			switch(lpTimerObject->dwTimerFlags)
			{
			case TIMER_FLAGS_ONCE:        //Delete the timer object processed just now.
				ObjectManager.DestroyObject(&ObjectManager,
					(struct __COMMON_OBJECT*)lpTimerObject);
				break;
			case TIMER_FLAGS_ALWAYS:    //Re-insert the timer object into timer queue.
				dwPriority  = lpTimerObject->dwTimeSpan;
				dwPriority /= SYSTEM_TIME_SLICE;
				dwPriority += System.dwClockTickCounter;
				dwPriority  = MAX_DWORD_VALUE - dwPriority;
				lpTimerQueue->InsertIntoQueue((struct __COMMON_OBJECT*)lpTimerQueue,
					(struct __COMMON_OBJECT*)lpTimerObject,
					dwPriority);
				break;
			default:
				break;
			}

			lpTimerObject = (struct __TIMER_OBJECT*)lpTimerQueue->GetHeaderElement(
				(struct __COMMON_OBJECT*)lpTimerQueue,
				&dwPriority);    //Check another timer object.

			if(NULL == lpTimerObject)
				break;

			dwPriority = MAX_DWORD_VALUE - dwPriority;
		}

		if(NULL == lpTimerObject)  //There is no timer object in queue.
		{
			__ENTER_CRITICAL_SECTION(NULL,dwFlags);
			System.dwNextTimerTick = 0L;
			__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		}
		else
		{
			__ENTER_CRITICAL_SECTION(NULL,dwFlags);
			System.dwNextTimerTick = dwPriority;    //Update the next timer tick counter.
			__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
			dwPriority = MAX_DWORD_VALUE - dwPriority;
			lpTimerQueue->InsertIntoQueue((struct __COMMON_OBJECT*)lpTimerQueue,
				(struct __COMMON_OBJECT*)lpTimerObject,
				dwPriority);
		}
	}

__CONTINUE_1:

	//
	//The following code wakes up all kernel thread(s) whose status is SLEEPING and
	//the time it(then) set is out.
	//
	if(System.dwClockTickCounter == KernelThreadManager.dwNextWakeupTick)  //There must existes
	 //kernel thread(s) to up.
	{
		lpSleepingQueue = KernelThreadManager.lpSleepingQueue;
		lpKernelThread  = (struct __KERNEL_THREAD_OBJECT*)lpSleepingQueue->GetHeaderElement(
			(struct __COMMON_OBJECT*)lpSleepingQueue,
			&dwPriority);
		while(lpKernelThread)
		{
			dwPriority = MAX_DWORD_VALUE - dwPriority;  //Now,dwPriority countains the tick
			                                            //counter value.
			if(dwPriority > System.dwClockTickCounter)
				break;    //This kernel thread should not be wake up.
			lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_READY;
			KernelThreadManager.AddReadyKernelThread(
				(struct __COMMON_OBJECT*)&KernelThreadManager,
				lpKernelThread);  //Insert the waked up kernel thread into ready queue.

			lpKernelThread = (struct __KERNEL_THREAD_OBJECT*)lpSleepingQueue->GetHeaderElement(
				(struct __COMMON_OBJECT*)lpSleepingQueue,
				&dwPriority);  //Check next kernel thread in sleeping queue.
		}
		if(NULL == lpKernelThread)
		{
			__ENTER_CRITICAL_SECTION(NULL,dwFlags);
			KernelThreadManager.dwNextWakeupTick = 0L;
			__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		}
		else
		{
			__ENTER_CRITICAL_SECTION(NULL,dwFlags);
			KernelThreadManager.dwNextWakeupTick = dwPriority;
			__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
			dwPriority = MAX_DWORD_VALUE - dwPriority;
			lpSleepingQueue->InsertIntoQueue((struct __COMMON_OBJECT*)lpSleepingQueue,
				(struct __COMMON_OBJECT*)lpKernelThread,
				dwPriority);
		}
	}

	goto __TERMINAL;

__TERMINAL:
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	System.dwClockTickCounter ++;    //Update the system clock interrupt counter.
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

	//KernelThreadManager.ScheduleFromInt((struct __COMMON_OBJECT*)&KernelThreadManager,
	//	lpEsp);

	return TRUE;
}


//
//The implementation of ConnectInterrupt routine of Interrupt Object.
//The routine do the following:
// 1. Insert the current object into interrupt object array(maintenanced by system object);
// 2. Set the object's data members correctly.
//

struct __COMMON_OBJECT* BOOL ConnectInterrupt(struct __COMMON_OBJECT*     lpThis,
							 __INTERRUPT_HANDLER  lpInterruptHandler,
							 LPVOID               lpHandlerParam,
							 UCHAR                ucVector,
							 UCHAR                ucReserved1,
							 UCHAR                ucReserved2,
							 UCHAR                ucInterruptMode,
							 BOOL                 bIfShared,
							 DWORD                dwCPUMask)
{
	__INTERRUPT_OBJECT*      lpInterrupt          = NULL;
	__INTERRUPT_OBJECT*      lpObjectRoot         = NULL;
	struct __SYSTEM*                lpSystem             = NULL;
	DWORD                    dwFlags              = 0L;

	if((NULL == lpThis) || (NULL == lpInterruptHandler))    //Parameters valid check.
		return NULL;

	if(ucVector >= MAX_INTERRUPT_VECTOR)                    //Impossible!!!
		return NULL;

	lpInterrupt = (__INTERRUPT_OBJECT*)
		ObjectManager.CreateObject(&ObjectManager,NULL,OBJECT_TYPE_INTERRUPT);
	if(NULL == lpInterrupt)    //Failed to create interrupt object.
		return FALSE;
	if(!lpInterrupt->Initialize((struct __COMMON_OBJECT*)lpInterrupt))  //Failed to initialize.
		return FALSE;

	lpInterrupt->lpPrevInterruptObject = NULL;
	lpInterrupt->lpNextInterruptObject = NULL;
	lpInterrupt->InterruptHandler      = lpInterruptHandler;
	lpInterrupt->lpHandlerParam        = lpHandlerParam;
	lpInterrupt->ucVector              = ucVector;

	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	lpObjectRoot = lpSystem->lpInterruptVector[ucVector];
	if(NULL == lpObjectRoot)    //If this is the first interrupt object of the vector.
	{
		System.lpInterruptVector[ucVector]  = lpInterrupt;
	}
	else
	{
		lpInterrupt->lpNextInterruptObject  = lpObjectRoot;
		lpObjectRoot->lpPrevInterruptObject = lpInterrupt;
		System.lpInterruptVector[ucVector]  = lpInterrupt;
	}
	//LEAVE_CRITICAL_SECTION();
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

	return (struct __COMMON_OBJECT*)lpInterrupt;
}

//
//The implementation of DisconnectInterrupt.
//

static VOID DisconnectInterrupt(struct __COMMON_OBJECT* lpThis,
								struct __COMMON_OBJECT* lpInterrupt)
{
	__INTERRUPT_OBJECT*   lpIntObject    = NULL;
	struct __SYSTEM*             lpSystem       = NULL;
	UCHAR                 ucVector       = NULL;
	DWORD                 dwFlags        = 0L;

	if((NULL == lpThis) || (NULL == lpInterrupt)) //Parameters check.
		return;

	lpSystem = (struct __SYSTEM*)lpThis;
	lpIntObject = (__INTERRUPT_OBJECT*)lpInterrupt;
	ucVector    = lpIntObject->ucVector;

	//ENTER_CRITICAL_SECTION();
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	if(NULL == lpIntObject->lpPrevInterruptObject)  //This is the first interrupt object.
	{
		lpSystem->lpInterruptVector[ucVector] = lpIntObject->lpNextInterruptObject;
		if(NULL != lpIntObject->lpNextInterruptObject) //Is not the last object.
		{
			lpIntObject->lpNextInterruptObject->lpPrevInterruptObject = NULL;
		}
	}
	else    //This is not the first object.
	{
		lpIntObject->lpPrevInterruptObject->lpNextInterruptObject = lpIntObject->lpNextInterruptObject;
		if(NULL != lpIntObject->lpNextInterruptObject)
		{
			lpIntObject->lpNextInterruptObject->lpPrevInterruptObject = lpIntObject->lpPrevInterruptObject;
		}
	}
	//LEAVE_CRITICAL_SECTION();
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	return;
}

//
//The implementation of Initialize routine of interrupt object.
//

BOOL InterruptInitialize(struct __COMMON_OBJECT* lpThis)
{
	__INTERRUPT_OBJECT*    lpInterrupt = NULL;

	if(NULL == lpThis)
		return FALSE;

	lpInterrupt = (__INTERRUPT_OBJECT*)lpThis;
	lpInterrupt->lpPrevInterruptObject = NULL;
	lpInterrupt->lpNextInterruptObject = NULL;
	lpInterrupt->InterruptHandler      = NULL;
	lpInterrupt->lpHandlerParam        = NULL;
	lpInterrupt->ucVector              = 0L;
	return TRUE;
}

//
//The implementation of Uninitialize of interrupt object.
//This routine does nothing.
//

VOID InterruptUninitialize(struct __COMMON_OBJECT* lpThis)
{
	return;
}


//
//The implementation of timer object.
//

BOOL TimerInitialize(struct __COMMON_OBJECT* lpThis)    //Initializing routine of timer object.
{
	struct __TIMER_OBJECT*     lpTimer  = NULL;
	
	if(NULL == lpThis)
		return FALSE;

	lpTimer = (struct __TIMER_OBJECT*)lpThis;
	lpTimer->dwTimerID    = 0L;
	lpTimer->dwTimeSpan   = 0L;
	lpTimer->lpKernelThread      = NULL;
	lpTimer->lpHandlerParam      = NULL;
	lpTimer->DirectTimerHandler  = NULL;

	return TRUE;
}

//
//Uninitializing routine of timer object.
//

VOID TimerUninitialize(struct __COMMON_OBJECT* lpThis)
{
	return;
}

//-----------------------------------------------------------------------------------
//
//              The implementation of system object.
//
//------------------------------------------------------------------------------------

//
//Initializing routine of system object.
//The routine do the following:
// 1. Create a priority queue,to be used as lpTimerQueue,countains the timer object;
// 2. Create an interrupt object,as TIMER interrupt object;
// 3. Initialize system level variables,such as dwPhysicalMemorySize,etc.
//

static BOOL SystemInitialize(struct __COMMON_OBJECT* lpThis)
{
	struct __SYSTEM*            lpSystem         = NULL;
	struct __PRIORITY_QUEUE*    lpPriorityQueue  = NULL;
	__INTERRUPT_OBJECT*  lpIntObject      = NULL;
	BOOL                 bResult          = FALSE;
	DWORD                dwFlags          = 0L;

	if(NULL == lpThis)
		return FALSE;

	lpSystem = (struct __SYSTEM*)lpThis;
	lpPriorityQueue = (struct __PRIORITY_QUEUE*)ObjectManager.CreateObject(&ObjectManager,
		NULL,
		OBJECT_TYPE_PRIORITY_QUEUE);

	if(NULL == lpPriorityQueue)  //Failed to create priority queue.
		return FALSE;

	if(!lpPriorityQueue->Initialize((struct __COMMON_OBJECT*)lpPriorityQueue))  //Failed to initialize
		                                                                 //priority queue.
		goto __TERMINAL;
	lpSystem->lpTimerQueue = lpPriorityQueue;

	lpIntObject = (__INTERRUPT_OBJECT*)ObjectManager.CreateObject(
		&ObjectManager,
		NULL,
		OBJECT_TYPE_INTERRUPT);
	if(NULL == lpIntObject)
		goto __TERMINAL;

	bResult = lpIntObject->Initialize((struct __COMMON_OBJECT*)lpIntObject);
	if(!bResult)
		goto __TERMINAL;

	lpIntObject->ucVector = INTERRUPT_VECTOR_TIMER;
	lpIntObject->lpHandlerParam = NULL;
	lpIntObject->InterruptHandler = TimerInterruptHandler;

	//ENTER_CRITICAL_SECTION();
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	lpSystem->lpInterruptVector[INTERRUPT_VECTOR_TIMER] = lpIntObject;
	//
	//Here,maybe some code initializes all other interrupt vector.
	//
	//LEAVE_CRITICAL_SECTION();
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	bResult = TRUE;

__TERMINAL:
	if(!bResult)
	{
		if(lpPriorityQueue != NULL)
		{
			ObjectManager.DestroyObject(&ObjectManager,
				(struct __COMMON_OBJECT*)lpPriorityQueue);
		}
		if(lpIntObject != NULL)
		{
			ObjectManager.DestroyObject(&ObjectManager,
				(struct __COMMON_OBJECT*)lpIntObject);
		}
	}
	return bResult;
}

//
//GetClockTickCounter routine.
//

static DWORD GetClockTickCounter(struct __COMMON_OBJECT* lpThis)
{
	struct __SYSTEM*    lpSystem = NULL;

	if(NULL == lpThis)
		return 0L;

	lpSystem = (struct __SYSTEM*)lpThis;

	return lpSystem->dwClockTickCounter;
}

//
//GetPhysicalMemorySize.
//

static DWORD GetPhysicalMemorySize(struct __COMMON_OBJECT* lpThis)
{
	if(NULL == lpThis)
		return 0L;

	return ((struct __SYSTEM*)lpThis)->dwPhysicalMemorySize;
}

//
//DispatchInterrupt.
//The routine calls another routine,DefaultIntHandler,to handle the interrupt
//without interrupt object.
//

//
//This routine is the default interrupt handler.
//If no entity(such as,kernel thread) install an interrupt handler,this handler
//will be called to handle the appropriate interrupt.
//

static VOID DefaultIntHandler(LPVOID lpEsp,UCHAR ucVector)
{
	BYTE          strBuffer[16] = {0};
	DWORD         dwTmp         = 0L;
	DWORD         dwLoop        = 0L;
	DWORD*        lpdwEsp       = NULL;
	WORD          wIsr;
	static DWORD  dwTotalNum    = 0L;

	dwTotalNum ++;    //Record this unhandled exception or interrupt.

	printf("  Unhandled interrupt or Exception!");  //Print out this message.
	printf("  Interrupt Vector:");

	dwTmp   = ucVector;
	lpdwEsp = (DWORD*)lpEsp;
	strBuffer[0] = ' ';
	strBuffer[1] = ' ';
	strBuffer[2] = ' ';
	strBuffer[3] = ' ';

	Hex2Str(dwTmp,&strBuffer[4]);
	printf("%s\n", strBuffer);  //Print out the interrupt or exception's vector.
	if(dwTmp == 0x2B)      //--------- ** debug ** ---------------
	{
		printf("NIC interrupt : -------------------------------");
		//WriteWordToPort(0xd03e,0x0000);
		//ReadWordFromPort(&wIsr,(WORD)0xD03E);
/*
		__asm
		{
			push eax
			push edx
			mov dx,0xd037
			mov al,0x10
			out dx,al      //Reset NIC
__REPEAT:
			in al,dx
			and al,0x10
			jnz __REPEAT   //Waiting the NIC to finished initialization.
			mov eax,0x00000100
			mov dx,0xd054
			out dx,eax
			mov dx,0xd03c
			mov ax,0xFFFF
			out dx,ax     //Set the IMR register.
			mov eax,0x00000010
			mov dx,0xd048
			out dx,eax    //Reset the timer register.
			pop edx
			pop eax
		}
*/
	}
	printf("  Context:");  //Print out system context information.
	for(dwLoop = 0;dwLoop < 12;dwLoop ++)
	{
		dwTmp = *lpdwEsp;
		Hex2Str(dwTmp,&strBuffer[4]);
		printf(strBuffer);
		lpdwEsp ++;
	}

	if(dwTmp <= 0x20)    //---------- ** debug ** --------------
	{
		Hex2Str(dwTotalNum,strBuffer);
		printf("Total unhandled exception or interrupt is:");
		printf("%s\n", strBuffer);
__LOOP:
	goto __LOOP;
	}

	/*
__DEADLOOP:    //If a unhandled interrupt or exception occurs,the system will halt.
	goto __DEADLOOP;*/
	/*
	__asm               //-------------- ** debug ** ------------
	{
		cli
	}
	*/
	return;
}


static VOID DispatchInterrupt(struct __COMMON_OBJECTstruct __COMMON_OBJECT* lpThis,
							  LPVOID           lpEsp,
							  UCHAR ucVector)
{
	__INTERRUPT_OBJECT*    lpIntObject  = NULL;
	struct __SYSTEM*              lpSystem     = NULL;

	if((NULL == lpThis) || (NULL == lpEsp))
		return;

	lpSystem = (struct __SYSTEM*)lpThis;
	
	lpSystem->ucIntNestLevel += 1;    //Increment nesting level.
	if(lpSystem->ucIntNestLevel <= 1)
	{
		//Call thread hook here,because current kernel thread is
		//interrupted.
		//If interrupt occurs before any kernel thread is scheduled,
		//lpCurrentKernelThread is NULL.
		if(KernelThreadManager.lpCurrentKernelThread)
		{
			KernelThreadManager.CallThreadHook(
				THREAD_HOOK_TYPE_ENDSCHEDULE,
				KernelThreadManager.lpCurrentKernelThread,
				NULL);
		}
	}
	lpIntObject = lpSystem->lpInterruptVector[ucVector];

	if(NULL == lpIntObject)  //The current interrupt vector has not handler object.
	{
		DefaultIntHandler(lpEsp,ucVector);
		goto __RETFROMINT;
	}

	while(lpIntObject)    //Travel the whole interrupt list of this vector.
	{
		if(lpIntObject->InterruptHandler(lpEsp,
			lpIntObject->lpHandlerParam))    //If an interrupt object handles the interrupt,then returns.
		{
			break;
		}
		lpIntObject = lpIntObject->lpNextInterruptObject;
	}

__RETFROMINT:
	lpSystem->ucIntNestLevel -= 1;    //Decrement interrupt nesting level.
	if(0 == lpSystem->ucIntNestLevel)  //The outmost interrupt.
	{
		KernelThreadManager.ScheduleFromInt((struct __COMMON_OBJECT*)&KernelThreadManager,
			lpEsp);  //Re-schedule kernel thread.
	}
	else
	{
		BUG();  //In current version(V1.5),interrupt nesting is not supportted yet.
	}
	return;
}

//
//SetTimer.
//The routine do the following:
// 1. Create a timer object;
// 2. Initialize the timer object;
// 3. Insert into the timer object into timer queue of system object;
// 4. Return the timer object's base address if all successfully.
//

static struct __COMMON_OBJECT* SetTimer(struct __COMMON_OBJECT* lpThis,
					struct __KERNEL_THREAD_OBJECT* lpKernelThread,
					DWORD  dwTimerID,
					DWORD  dwTimeSpan,
					__DIRECT_TIMER_HANDLER lpHandler,
					LPVOID lpHandlerParam,
					DWORD  dwTimerFlags)
{
	struct __PRIORITY_QUEUE*            lpPriorityQueue    = NULL;
	struct __SYSTEM*                    lpSystem           = NULL;
	struct __TIMER_OBJECT*              lpTimerObject      = NULL;
	BOOL                         bResult            = FALSE;
	DWORD                        dwPriority         = 0L;
	DWORD                        dwFlags            = 0L;

	if((NULL == lpThis) ||
	   (NULL == lpKernelThread))    //Parameters check.
	   return NULL;

	if(dwTimeSpan <= SYSTEM_TIME_SLICE)
		dwTimeSpan = SYSTEM_TIME_SLICE;

	lpSystem = (struct __SYSTEM*)lpThis;
	lpTimerObject = (struct __TIMER_OBJECT*)ObjectManager.CreateObject(&ObjectManager, NULL, OBJECT_TYPE_TIMER);
	if(NULL == lpTimerObject)    //Can not create timer object.
		goto __TERMINAL;
	bResult = lpTimerObject->Initialize((struct __COMMON_OBJECT*)lpTimerObject);  //Initialize.
	if(!bResult)
		goto __TERMINAL;

	lpTimerObject->dwTimerID           = dwTimerID;
	lpTimerObject->dwTimeSpan          = dwTimeSpan;
	lpTimerObject->lpKernelThread      = lpKernelThread;
	lpTimerObject->DirectTimerHandler  = lpHandler;
	lpTimerObject->lpHandlerParam      = lpHandlerParam;
	lpTimerObject->dwTimerFlags        = dwTimerFlags;

	//
	//The following code calculates the priority value of the timer object.
	//
	dwPriority     = dwTimeSpan;
	dwPriority    /= SYSTEM_TIME_SLICE;
	dwPriority    += lpSystem->dwClockTickCounter;    //Now,the dwPriority countains the
	                                                  //tick counter this timer must be
	                                                  //processed.
	dwPriority     = MAX_DWORD_VALUE - dwPriority;    //Final priority value.

	__ENTER_CRITICAL_SECTION(NULL,dwFlags);

	bResult = lpSystem->lpTimerQueue->InsertIntoQueue((struct __COMMON_OBJECT*)lpSystem->lpTimerQueue,
		(struct __COMMON_OBJECT*)lpTimerObject,
		dwPriority);
	if(!bResult)
	{
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		goto __TERMINAL;
	}

	dwPriority = MAX_DWORD_VALUE - dwPriority;    //Now,dwPriority countains the next timer
	                                              //tick value.
	//ENTER_CRITICAL_SECTION();
	//__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	if((System.dwNextTimerTick > dwPriority) ||
	   (System.dwNextTimerTick == 0))
		System.dwNextTimerTick = dwPriority;    //Update the next timer tick counter.
	//LEAVE_CRITICAL_SECTION();
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

__TERMINAL:
	if(!bResult)
	{
		if(lpTimerObject != NULL)
		{
			ObjectManager.DestroyObject(&ObjectManager,(struct __COMMON_OBJECT*)lpTimerObject);
			lpTimerObject = NULL;
		}
	}
	return (struct __COMMON_OBJECT*)lpTimerObject;
}

//
//CancelTimer implementation.
//This routine is used to cancel timer.
//

static VOID CancelTimer(struct __COMMON_OBJECT* lpThis,struct __COMMON_OBJECT* lpTimer)
{
	struct __SYSTEM*                  lpSystem       = NULL;
	DWORD                      dwPriority     = 0L;
	struct __TIMER_OBJECT*            lpTimerObject  = NULL;

	if((NULL == lpThis) || (NULL == lpTimer))
		return;

	lpSystem = (struct __SYSTEM*)lpThis;
	//if(((struct __TIMER_OBJECT*)lpTimer)->dwTimerFlags != TIMER_FLAGS_ALWAYS)
	//	return;
	lpSystem->lpTimerQueue->DeleteFromQueue((struct __COMMON_OBJECT*)lpSystem->lpTimerQueue,
		lpTimer);
	lpTimerObject = (struct __TIMER_OBJECT*)
		lpSystem->lpTimerQueue->GetHeaderElement(
		(struct __COMMON_OBJECT*)lpSystem->lpTimerQueue,
		&dwPriority);
	if(NULL == lpTimerObject)    //There is not any timer object to be processed.
		return;

	//
	//The following code updates the tick counter that timer object should be processed.
	//
	dwPriority = MAX_DWORD_VALUE - dwPriority;
	if(dwPriority > lpSystem->dwNextTimerTick)
		lpSystem->dwNextTimerTick = dwPriority;
	dwPriority = MAX_DWORD_VALUE - dwPriority;
	lpSystem->lpTimerQueue->InsertIntoQueue(
		(struct __COMMON_OBJECT*)lpSystem->lpTimerQueue,
		(struct __COMMON_OBJECT*)lpTimerObject,
		dwPriority);    //Insert into timer object queue.

	return;

	/*struct __SYSTEM*  lpSystem  = NULL;

	if((NULL == lpThis) || (NULL == lpTimer))
		return;

	lpSystem = (struct __SYSTEM*)lpThis;
	//if(((struct __TIMER_OBJECT*)lpTimer)->dwTimerFlags != TIMER_FLAGS_ALWAYS)
	//	return;
	if(lpSystem->lpTimerQueue->DeleteFromQueue((struct __COMMON_OBJECT*)lpSystem->lpTimerQueue,
		lpTimer))
	{
		ObjectManager.DestroyObject(&ObjectManager,
			lpTimer);
	}
	return;*/
}

/***************************************************************************************
****************************************************************************************
****************************************************************************************
****************************************************************************************
***************************************************************************************/

//The definition of system object.

struct __SYSTEM System = {
	{0},                      //lpInterruptVector[MAX_INTERRUPT_VECTOR].
	NULL,                     //lpTimerQueue.
	0L,                       //dwClockTickCounter,
	0L,                       //dwNextTimerTick,
	0,                        //ucIntNestLeve;
	0,
	0,
	0,                        //ucReserved3;
	0L,                       //dwPhysicalMemorySize,
    	SystemInitialize,         //Initialize routine.
	GetClockTickCounter,      //GetClockTickCounter routine.
	GetPhysicalMemorySize,    //GetPhysicalMemorySize routine.
	DispatchInterrupt,        //DispatchInterrupt routine.
	ConnectInterrupt,         //ConnectInterrupt.
	DisconnectInterrupt,      //DisconnectInterrupt.
	SetTimer,                 //SetTimerRoutine.
	CancelTimer
};

//***************************************************************************************
//
//             General Interrupt Handler
//
//***************************************************************************************

//
//GeneralIntHandler.
//This routine is the general handler of all interrupts.
//Once an interrupt occurs,the low layer code (resides in Mini-Kernel) calls this routine,
//this routine then calls DispatchInterrupt of system object.
//

VOID GeneralIntHandler(DWORD dwVector,LPVOID lpEsp)
{
	UCHAR    ucVector = LOBYTE(LOWORD(dwVector));

	System.DispatchInterrupt((struct __COMMON_OBJECT*)&System, lpEsp, ucVector);
}




