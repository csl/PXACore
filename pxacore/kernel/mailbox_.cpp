//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jul,20 2005
//    Module Name               : MAILBOX.CPP
//    Module Funciton           : 
//                                This module countains the mailbox's implementation data
//                                structures and data types definition.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __STDAFX_H__	
#include "StdAfx.h"
#endif

//
//Pre-declaration's for member routines.
//

static DWORD SendMail(__COMMON_OBJECT*,LPVOID);
static DWORD GetMail(__COMMON_OBJECT*,LPVOID*,DWORD);
static BOOL  CancelWaiting(__COMMON_OBJECT*,__COMMON_OBJECT*);
static DWORD TimerHandler(LPVOID);

#define MAILBOX_TIMEOUT_TIMER_ID     1024    //Used by set timer.

//
//Mailbox's Initialize routine.
//

BOOL MailboxInitialize(__COMMON_OBJECT* lpThis)
{
	__MAILBOX*            lpMailBox          = NULL;
	DWORD                 dwLoop             = 0L;
	__PRIORITY_QUEUE*     lpWaitingQueue     = NULL;
	BOOL                  bResult            = FALSE;

	if(NULL == lpThis)  //Parameter check.
		return bResult;

	lpMailBox = (__MAILBOX*)lpThis;
	for(dwLoop = 0;dwLoop < MAILBOX_MSG_NUM;dwLoop ++)
		lpMailBox->MsgArray[dwLoop] = NULL;
	lpMailBox->dwMsgNum = 0L;

	//
	//The following code creates the priority queue,which is used to countain the
	//kernel thread(s) waiting for this mailbox.
	//
	lpWaitingQueue = (__PRIORITY_QUEUE*)ObjectManager.CreateObject(&ObjectManager,
		NULL,
		OBJECT_TYPE_PRIORITY_QUEUE);
	if(NULL == lpWaitingQueue)
	{
		bResult = FALSE;
		goto __TERMINAL;
	}
	if(!lpWaitingQueue->Initialize((__COMMON_OBJECT*)lpWaitingQueue)) //Initialize the priority queue.
	{
		bResult = FALSE;
		goto __TERMINAL;
	}

	lpMailBox->lpWaitingQueue = lpWaitingQueue;
	lpMailBox->SendMail       = SendMail;
	lpMailBox->GetMail        = GetMail;
	lpMailBox->CancelWaiting  = CancelWaiting;

	bResult                   = TRUE;  //Initialize the priority queue object successfully.

__TERMINAL:
	if((!bResult) && (lpWaitingQueue))    //Failed to initialize this object.
	{
		ObjectManager.DestroyObject(&ObjectManager,
			(__COMMON_OBJECT*)lpWaitingQueue);        //Destroy the priority queue object.
	}
	return bResult;
}

//
//Mailbox's Uninitialize routine.
//

VOID MailboxUninitialize(__COMMON_OBJECT* lpThis)
{
	__MAILBOX*               lpMailBox   = NULL;

	if(NULL == lpThis)    //Check the parameter.
		return;
	lpMailBox = (__MAILBOX*)lpThis;

	ObjectManager.DestroyObject(&ObjectManager,
		(__COMMON_OBJECT*)(lpMailBox->lpWaitingQueue));    //Destroy the kernel thread queue.

	return;
}


//
//The implementation of SendMail routine.
//This routine does the following:
// 1. Check if the mailbox has a free message slot;
// 2. If there is not a free message slot,then return MAILBOX_NO_SLOT;
// 3. If there is a free slot,then insert the message to this slot;
// 4. Check if there are kernel threads waiting for message;
// 5. If there is not any kernel thread waiting for message,then return SUCCESS;
// 6. If there is kernel thread waiting for message,then wake up the first one,
//    insert it into READY queue of KernelThreadManager,and return success.
//

static DWORD SendMail(__COMMON_OBJECT* lpThis,LPVOID lpMsg)
{
	__MAILBOX*                lpMailBox             = NULL;
	DWORD                     dwLoop                = 0L;
	BOOL                      bFind                 = FALSE;
	__PRIORITY_QUEUE*         lpWaitingQueue        = NULL;
	__KERNEL_THREAD_OBJECT*   lpKernelThread        = NULL;

	if((NULL == lpThis) || (NULL == lpMsg)) //Parameter check.
		return MAILBOX_FAILED;

	lpMailBox = (__MAILBOX*)lpThis;

	ENTER_CRITICAL_SECTION();

	for(dwLoop = 0;dwLoop < MAILBOX_MSG_NUM;dwLoop ++)  //Find a free mail slot.
	{
		if(NULL == lpMailBox->MsgArray[dwLoop])    //Find a free slot.
		{
			bFind  = TRUE;
			break;
		}
	}

	if(!bFind)    //Can not find a free slot.
	{
		LEAVE_CRITICAL_SECTION();
		return MAILBOX_NO_SLOT;
	}

	lpMailBox->MsgArray[dwLoop] = lpMsg;    //Send the mail.
	lpMailBox->dwMsgNum += 1;
	/*if(lpMailBox->dwMsgNum > 1)     //There is not any kernel thread waiting fo a message.
	{
		LEAVE_CRITICAL_SECTION();
		return MAILBOX_SUCCESS;
	}*/

	//
	//Determine if there are some kernel threads waiting for message.
	//
	KernelThreadManager.LockKernelThread((__COMMON_OBJECT*)&KernelThreadManager,
		NULL);    //Lock the current kernel thread.
	//LEAVE_CRITICAL_SECTION();

	lpWaitingQueue = lpMailBox->lpWaitingQueue;
	lpKernelThread = (__KERNEL_THREAD_OBJECT*)lpWaitingQueue->GetHeaderElement(
		(__COMMON_OBJECT*)lpWaitingQueue,
		NULL);
	if(NULL == lpKernelThread)    //There is not any kernel thread waiting for message.
	{
		KernelThreadManager.UnlockKernelThread((__COMMON_OBJECT*)&KernelThreadManager,
			NULL);    //Unlock the current kernel thread.
		LEAVE_CRITICAL_SECTION();
		return MAILBOX_SUCCESS;
	}
	lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_READY;    //Wake up the thread.
	KernelThreadManager.lpReadyQueue->InsertIntoQueue(
		(__COMMON_OBJECT*)KernelThreadManager.lpReadyQueue,
		(__COMMON_OBJECT*)lpKernelThread,
		lpKernelThread->dwScheduleCounter);    //Insert into ready queue.

	LEAVE_CRITICAL_SECTION();
	KernelThreadManager.UnlockKernelThread((__COMMON_OBJECT*)&KernelThreadManager,
		NULL);    //Unlock the current kernel thread.

	return MAILBOX_SUCCESS;
}

//
//The implementation of GetMail.
//This routine does the following:
// 1. Check if there is a message in the mail box;
// 2. If there is a message,then get it;
// 3. Else,block the current kernel thread;
// 4. If the caller set a timeout value,then set a timer,
//    to submit a timeout operation.
//

static DWORD GetMail(__COMMON_OBJECT* lpThis,LPVOID* llpMsg,DWORD dwTimeOut)
{
	__MAILBOX*                              lpMailBox               = NULL;
	__KERNEL_THREAD_OBJECT*                 lpKernelThread          = NULL;
	DWORD                                   dwLoop                  = 0L;
	__TIMER_HANDLER_PARAM*                  lpTimerHandlerParam     = NULL;
	__TIMER_OBJECT*                         lpTimerObject           = NULL;

	if((NULL == lpThis)  || (NULL == llpMsg)) //Parameters check.
		return MAILBOX_FAILED;

	lpMailBox = (__MAILBOX*)lpThis;
	ENTER_CRITICAL_SECTION();
	if(lpMailBox->dwMsgNum > 0)    //There at least a message in the mail box.
	{
		for(dwLoop = 0;dwLoop < MAILBOX_MSG_NUM;dwLoop ++)
		{
			if(lpMailBox->MsgArray[dwLoop])
			{
				*llpMsg = lpMailBox->MsgArray[dwLoop];    //Get the message.
				lpMailBox->MsgArray[dwLoop] = NULL;
				lpMailBox->dwMsgNum --;                   //Decrement the message number.
				LEAVE_CRITICAL_SECTION();
				return MAILBOX_SUCCESS;
			}
		}
		LEAVE_CRITICAL_SECTION();    //The following instructions can never be reached normallly.
		return MAILBOX_FAILED;
	}
	else    //There is not any message in the mailbox.
	{
		lpKernelThread = KernelThreadManager.lpCurrentKernelThread;
		lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_BLOCKED;    //Block the current thread.
		LEAVE_CRITICAL_SECTION();
		if(dwTimeOut)    //Should set a timer to process the timeout operation.
		{
			lpTimerHandlerParam = (__TIMER_HANDLER_PARAM*)KMemAlloc(
				sizeof(__TIMER_HANDLER_PARAM),
				KMEM_SIZE_TYPE_ANY);    //Allocate a memory block,this block is released by timer
			                            //handler.
			if(NULL == lpTimerHandlerParam)    //Can not allocate memory.
			{
				lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_RUNNING;
				return MAILBOX_FAILED;
			}
			lpTimerHandlerParam->lpMailBox       = lpMailBox;
			lpTimerHandlerParam->lpKernelThread  = lpKernelThread;
			lpTimerHandlerParam->bCanceled       = FALSE;
			lpTimerHandlerParam->llpHandlerParam = (LPVOID*)&lpTimerHandlerParam;
			lpTimerHandlerParam->dwWakenupReason = WAKEN_UP_REASON_HAVE_MESSAGE;

			lpTimerObject = (__TIMER_OBJECT*)System.SetTimer((__COMMON_OBJECT*)&System,
				lpKernelThread,
				MAILBOX_TIMEOUT_TIMER_ID,
				dwTimeOut,
				TimerHandler,
				lpTimerHandlerParam,
				TIMER_FLAGS_ONCE);    //Set a timer to process the timeout.
			                          //CAUTION!!! : Currently,we do not process the potential error.
		}
		lpMailBox->lpWaitingQueue->InsertIntoQueue(
			(__COMMON_OBJECT*)lpMailBox->lpWaitingQueue,
			(__COMMON_OBJECT*)lpKernelThread,
			0L);    //Insert into mailbox's waiting queue.
		KernelThreadManager.ScheduleFromProc(&lpKernelThread->KernelThreadContext);    //Re-schedule.
	}

		//
		//The following code is executed when the kernel thread is waken up.
		//Two cases exist:
		// 1. One kernel thread have sent a message to the mailbox,and wake up the current kernel
		//    thread;
		// 2. The waiting is canceled by a timer,in case of timeout not equal zero.
		//In the first case,the kernel thread get a message from mailbox,and check if a timer
		//has been set,if set,then cancel the timer by set bCanceled to	TRUE.
		//In the second case,the kernel thread only returns a MAILBOX_TIME out value.
		//
		ENTER_CRITICAL_SECTION();

		if(dwTimeOut == 0)    //Have not set a timer.
		{
			for(dwLoop = 0;dwLoop < MAILBOX_MSG_NUM;dwLoop ++)
			{
				if(NULL != lpMailBox->MsgArray[dwLoop])
				{
					*llpMsg = lpMailBox->MsgArray[dwLoop];
					lpMailBox->MsgArray[dwLoop] = NULL;
					lpMailBox->dwMsgNum --;
				}
			}
			LEAVE_CRITICAL_SECTION();
			return MAILBOX_SUCCESS;
		}

		//
		//The following code is used to deal with timeout process.
		//
		switch(lpTimerHandlerParam->dwWakenupReason)
		{
		case WAKEN_UP_REASON_TIMEOUT:
			KMemFree((LPVOID)lpTimerHandlerParam,KMEM_SIZE_TYPE_ANY,0L);
			LEAVE_CRITICAL_SECTION();
			return MAILBOX_TIMEOUT;
		case WAKEN_UP_REASON_HAVE_MESSAGE:
			for(dwLoop = 0;dwLoop < MAILBOX_MSG_NUM;dwLoop ++)
			{
				if(NULL != lpMailBox->MsgArray[dwLoop])
				{
					*llpMsg = lpMailBox->MsgArray[dwLoop];
					lpMailBox->MsgArray[dwLoop] = NULL;
					lpMailBox->dwMsgNum --;
					if(dwTimeOut)    //Should cancel the timer.
					{
						System.CancelTimer((__COMMON_OBJECT*)&System,
							(__COMMON_OBJECT*)lpTimerObject);
						KMemFree((LPVOID)lpTimerHandlerParam,KMEM_SIZE_TYPE_ANY,0L);
					}
				}
			}
			LEAVE_CRITICAL_SECTION();
			return MAILBOX_SUCCESS;
		default:
			KMemFree((LPVOID)lpTimerHandlerParam,KMEM_SIZE_TYPE_ANY,0L);
			LEAVE_CRITICAL_SECTION();
			return MAILBOX_FAILED;
		}

	//The following code will never be reached.
	return MAILBOX_FAILED;
}

//
//The implementation of CancelWaiting.
//This routine is called by timeout processing timer handler,this routine only
//delete the waiting kernel thread indicated by lpKernelThread from waiting queue
//of mailbox indicated by lpThis,mark it's status to READY,and insert it into READY queue.
//If the kernel thread is not exist in the waiting queue,it means that the kernel thread
//may get a message.
//This routine is called in handler context.
//

static BOOL CancelWaiting(__COMMON_OBJECT* lpThis,__COMMON_OBJECT* lpThread)
{
	__MAILBOX*                lpMailBox            = NULL;
	__KERNEL_THREAD_OBJECT*   lpKernelThread       = NULL;

	if((NULL == lpThis) || (NULL == lpThread))  //Parameters check.
		return FALSE;

	lpMailBox         = (__MAILBOX*)lpThis;
	lpKernelThread    = (__KERNEL_THREAD_OBJECT*)lpThread;
	if(!lpMailBox->lpWaitingQueue->DeleteFromQueue((__COMMON_OBJECT*)lpMailBox->lpWaitingQueue,
		(__COMMON_OBJECT*)lpKernelThread))    //Delete the kernel thread from waiting queue.
	{
		return FALSE;    //The kernel thread object is not in the waiting queue.
	}
	lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_READY;
	KernelThreadManager.lpReadyQueue->InsertIntoQueue(
		(__COMMON_OBJECT*)KernelThreadManager.lpReadyQueue,
		(__COMMON_OBJECT*)lpKernelThread,
		lpKernelThread->dwScheduleCounter);    //Insert into READY queue.

	return TRUE;
}

//
//The implementation of TimerHandler,which is called by timeout timer.
//

static DWORD TimerHandler(LPVOID lpParam)
{
	__TIMER_HANDLER_PARAM*           lpHandlerParam = NULL;
	if(NULL == lpParam)
		return 0L;
	lpHandlerParam = (__TIMER_HANDLER_PARAM*)lpParam;
	//if(lpHandlerParam->bCanceled)    //The timer is canceled by the current kernel thread.
	//	goto __TERMINAL;

	if(CancelWaiting((__COMMON_OBJECT*)lpHandlerParam->lpMailBox,
		(__COMMON_OBJECT*)lpHandlerParam->lpKernelThread))
	{
		lpHandlerParam->dwWakenupReason = WAKEN_UP_REASON_TIMEOUT;
	}
	return 0L;
}


//
//------------------------------------------------------------------------------------------
//

//
//Pre-declaration of static routines.
//

static VOID  ReleaseSemaphore(__COMMON_OBJECT*);
static DWORD GetSemaphore(__COMMON_OBJECT,DWORD);

//
//The implementation of semaphore's initialize routine.
//

BOOL SemaphoreInitialize(__COMMON_OBJECT* lpThis)
{
	__SEMAPHORE*           lpSema          = NULL;
	__PRIORITY_QUEUE*      lpWaitingQueue  = NULL;

	if(NULL == lpThis)    //Check parameter.
		return FALSE;

	lpSema = (__SEMAPHORE*)lpThis;

	lpWaitingQueue = (__PRIORITY_QUEUE*)ObjectManager.CreateObject(&ObjectManager,
		NULL,
		OBJECT_TYPE_PRIORITY_QUEUE);
	if(NULL == lpWaitingQueue)    //Can not create waiting queue.
		return FALSE;

	if(!lpWaitingQueue->Initialize((__COMMON_OBJECT*)lpWaitingQueue))
	{
		ObjectManager.DestroyObject(&ObjectManager,
			(__COMMON_OBJECT*)lpWaitingQueue);      //Destroy the waiting queue.
		return FALSE;
	}

	lpSema->dwSemaphoreNum        = 0L;
	lpSema->lpWaitingQueue        = lpWaitingQueue;

	return TRUE;
}

//
//The implementation of semaphore's uninitialize routine.
//

VOID SemaphoreUninitialize(__COMMON_OBJECT* lpThis)
{
	__SEMAPHORE*          lpSema       = NULL;

	if(NULL == lpThis)
		return;

	lpSema = (__SEMAPHORE*)lpThis;
	ObjectManager.DestroyObject(&ObjectManager,
		(__COMMON_OBJECT*)lpSema->lpWaitingQueue);

	return;
}

//
//The implementation of ReleaseSemaphore routine.
//

static VOID ReleaseSemaphore(__COMMON_OBJECT* lpThis)
{
}

//
//The implementation of GetSemaphore routine.
//

static DWORD GetSemaphore(__COMMON_OBJECT* lpThis,DWORD dwTimeOut)
{
	return 0L;
}

