//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jul,24 2005
//    Module Name               : MAILBOX.CPP
//    Module Funciton           : 
//                                This module countains mailbox's implementation code.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include "stdafx.h"

//#include ".\NetCore\NETBUFF.H"

//
//Pre-declaration of static routines.
//

static DWORD GetMail(__COMMON_OBJECT*,LPVOID*,DWORD);
static DWORD SendMail(__COMMON_OBJECT*,LPVOID,DWORD);
static DWORD MailBoxTimerHandler(LPVOID);

//
//Defines a parameter object,to be used by timer handler.
//

BEGIN_DEFINE_OBJECT(__MAILBOX_TIMERH_PARAM)
    __MAILBOX*               lpMailBox;               //Point to mailbox object.
    __KERNEL_THREAD_OBJECT*  lpKernelThread;          //Point to kernel thread object who
	                                                  //set the timer.
	DWORD                    dwWakeupReason;          //Wake up reason.
	DWORD                    dwSetReason;             //Timer set reason,why set a timer.
END_DEFINE_OBJECT()

#define WAKE_UP_REASON_HAVE_MESSAGE    0x00000001     //The kernel thread is waken up by SendMail routine.
#define WAKE_UP_REASON_TIMEOUT         0x00000002     //The kernel thread is waken up by timer.

#define TIMER_SET_REASON_GETTING       0x00000003     //If a timer is set by a kernel thread wanting
                                                      //to get a mail,but the mailbox is empty,then
													  //it set this value.
#define TIMER_SET_REASON_SENDING       0x00000004     //If a timer is set by a kernel thread wanting
													  //to send a mail,but the mailbox is full,then
													  //it set a timer,and set this value.

#define MAILBOX_TIMEOUT_TIMER_ID       1024

//
//The implementation of Initialize routine.
//This routine does the following:
// 1. Set all member variables to NULL or zero;
// 2. Create two queues,getting queue and sending queue;
// 3. Initializes the queue variables to the two queues.
//

BOOL MailBoxInitialize(__COMMON_OBJECT* lpThis)
{
	__PRIORITY_QUEUE*       lpGettingQueue       = NULL;
	__PRIORITY_QUEUE*       lpSendingQueue       = NULL;
	__MAILBOX*              lpMailBox            = NULL;
	BOOL                    bResult              = FALSE;

	if(NULL == lpThis)    //Parameter check.
		return FALSE;

	lpMailBox = (__MAILBOX*)lpThis;

	lpGettingQueue = (__PRIORITY_QUEUE*)ObjectManager.CreateObject(&ObjectManager,
		NULL,
		OBJECT_TYPE_PRIORITY_QUEUE);    //Create getting queue.
	if(NULL == lpGettingQueue)          //Can not create getting queue,may has not enough memory.
		goto __TERMINAL;
	if(!lpGettingQueue->Initialize((__COMMON_OBJECT*)lpGettingQueue))  //Can not initialize it.
		goto __TERMINAL;

	lpSendingQueue = (__PRIORITY_QUEUE*)ObjectManager.CreateObject(&ObjectManager,
		NULL,
		OBJECT_TYPE_PRIORITY_QUEUE);    //Create sending queue.
	if(NULL == lpSendingQueue)
		goto __TERMINAL;
	if(!lpSendingQueue->Initialize((__COMMON_OBJECT*)lpGettingQueue))  //Initialize it.
		goto __TERMINAL;

	lpMailBox->lpMsg            = NULL;
	lpMailBox->lpGettingQueue   = lpGettingQueue;
	lpMailBox->lpSendingQueue   = lpSendingQueue;
	lpMailBox->GetMail          = GetMail;
	lpMailBox->SendMail         = SendMail;

	bResult = TRUE;


__TERMINAL:
	if(!bResult)    //The initializing process is fail.
	{
		if(NULL == lpGettingQueue)
			ObjectManager.DestroyObject(&ObjectManager,
			(__COMMON_OBJECT*)lpGettingQueue);    //Destroy getting queue.
		if(NULL == lpSendingQueue)
			ObjectManager.DestroyObject(&ObjectManager,
			(__COMMON_OBJECT*)lpSendingQueue);    //Destroy sending queue.
	}
	return bResult;
}

//
//The implementation of Uninitialize routine.
//This routine only destory the queue objects of mail box.
//

VOID MailBoxUninitialize(__COMMON_OBJECT* lpThis)
{
	__MAILBOX*                lpMailBox        = NULL;

	if(NULL == lpThis)
		return;

	lpMailBox = (__MAILBOX*)lpThis;
	ObjectManager.DestroyObject(&ObjectManager,
		(__COMMON_OBJECT*)lpMailBox->lpGettingQueue);    //Destroy the getting queue.
	ObjectManager.DestroyObject(&ObjectManager,
		(__COMMON_OBJECT*)lpMailBox->lpSendingQueue);    //Destroy the sending queue.

	return;
}

//
//The implementation of GetMail routine.
//This routine does the following:
// 1. Check if the mail box is empty;
// 2. If not empty,then get the mail,and check if there are kernel threads waiting to send mail;
// 3. If there are kernel threads waiting to send mail,then wake up the first one;
// 4. return success;
// 5. If the mail box is empty,then block the current kenrel thread;
// 6. If the dwTimeOut parameter is not zero,then set a timer;
// 7. Reschedule all kernel threads of the system;
// 8. After been waken up,check the wake up reason;
// 9. If waken up by timer,then return MAILBOX_TIMEOUT;
// 10. If waken up by SendMail routine,then get the mail,and cancel timer if
//     set;
// 11. Return appropriate value.
//

static DWORD GetMail(__COMMON_OBJECT* lpThis,LPVOID* llpMsg,DWORD dwTimeOut)
{
	__MAILBOX*                           lpMailBox                = NULL;
	__KERNEL_THREAD_OBJECT*              lpKernelThread           = NULL;
	__TIMER_OBJECT*                      lpTimerObject            = NULL;
	__MAILBOX_TIMERH_PARAM*              lpHandlerParam           = NULL;
	DWORD                                dwFlags                  = 0L;

	if((NULL == lpThis) || (NULL == llpMsg))    //Parameter check.
		return MAILBOX_FAILED;

	lpMailBox = (__MAILBOX*)lpThis;

	//ENTER_CRITICAL_SECTION();
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	if(NULL != lpMailBox->lpMsg)
	{
		*llpMsg = lpMailBox->lpMsg;           //Get the message.
		lpMailBox->lpMsg = NULL;              //Clear the mailbox.
		lpKernelThread = (__KERNEL_THREAD_OBJECT*)
			lpMailBox->lpSendingQueue->GetHeaderElement(
			(__COMMON_OBJECT*)lpMailBox->lpSendingQueue,
			NULL);
		if(NULL == lpKernelThread)    //There is not any kernel thread waiting to send message.
		{
			//LEAVE_CRITICAL_SECTION();
			__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
			return MAILBOX_SUCCESS;
		}
		else    //There at lease one kernel thread waiting to send a mail,so wake up it.
		{
			lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_READY;
			KernelThreadManager.AddReadyKernelThread(
				(__COMMON_OBJECT*)&KernelThreadManager,
				lpKernelThread);  //Add to ready queue.
			__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
			return MAILBOX_SUCCESS;
		}
	}
	else    //There is not any message,so must block the kernel thread.
	{
		lpKernelThread = KernelThreadManager.lpCurrentKernelThread;
		lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_BLOCKED;
		if(dwTimeOut)    //Should set a timeout timer.
		{
			lpHandlerParam = (__MAILBOX_TIMERH_PARAM*)KMemAlloc(sizeof(__MAILBOX_TIMERH_PARAM),
				KMEM_SIZE_TYPE_ANY);
			if(NULL == lpHandlerParam)    //Can not allocate memory.
			{
				lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_RUNNING;
				//LEAVE_CRITICAL_SECTION();
				__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
				return MAILBOX_NO_RESOURCE;
			}
			lpHandlerParam->lpMailBox        = lpMailBox;
			lpHandlerParam->lpKernelThread   = lpKernelThread;
			lpHandlerParam->dwWakeupReason   = WAKE_UP_REASON_HAVE_MESSAGE;
			lpHandlerParam->dwSetReason      = TIMER_SET_REASON_GETTING;

			lpTimerObject = (__TIMER_OBJECT*)System.SetTimer((__COMMON_OBJECT*)&System,
				lpKernelThread,
				MAILBOX_TIMEOUT_TIMER_ID,
				dwTimeOut,
				MailBoxTimerHandler,
				lpHandlerParam,
				TIMER_FLAGS_ONCE);

			if(NULL == lpTimerObject)    //Can not set a timer,maybe has not enough memory.
			{
				lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_RUNNING;
				//LEAVE_CRITICAL_SECTION();
				__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
				KMemFree((LPVOID)lpHandlerParam,KMEM_SIZE_TYPE_ANY,0L);
				return MAILBOX_NO_RESOURCE;
			}
		}
		lpMailBox->lpGettingQueue->InsertIntoQueue((__COMMON_OBJECT*)lpMailBox->lpGettingQueue,
			(__COMMON_OBJECT*)lpKernelThread,
			0L);    //Insert into the getting queue.
		//LEAVE_CRITICAL_SECTION();
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		KernelThreadManager.ScheduleFromProc(&lpKernelThread->KernelThreadContext);

		//
		//The following code is executed after this kernel thread is waken up.
		//
		//ENTER_CRITICAL_SECTION();
		__ENTER_CRITICAL_SECTION(NULL,dwFlags);
		if(!dwTimeOut)    //Have not set a timer.
		{
			*llpMsg = lpMailBox->lpMsg;
			lpMailBox->lpMsg = NULL;
			//LEAVE_CRITICAL_SECTION();
			__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
			return MAILBOX_SUCCESS;
		}
		//
		//The following code deals with the situation that a timer has been set.
		//

		switch (lpHandlerParam->dwWakeupReason)
		{
		case WAKE_UP_REASON_HAVE_MESSAGE:  //In this case,we should cancel timer first.
			*llpMsg = lpMailBox->lpMsg;
			lpMailBox->lpMsg = NULL;
			System.CancelTimer((__COMMON_OBJECT*)&System,
				(__COMMON_OBJECT*)lpTimerObject);
			__LEAVE_CRITICAL_SECTION(NULL,dwFlags);  //NOTE : This is modified later.
			KMemFree((LPVOID)lpHandlerParam,KMEM_SIZE_TYPE_ANY,0L);
			return MAILBOX_SUCCESS;
			break;
		case WAKE_UP_REASON_TIMEOUT:
			//LEAVE_CRITICAL_SECTION();
			__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
			KMemFree((LPVOID)lpHandlerParam,KMEM_SIZE_TYPE_ANY,0L);
			return MAILBOX_TIMEOUT;
		default:
			//LEAVE_CRITICAL_SECTION();
			__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
			KMemFree((LPVOID)lpHandlerParam,KMEM_SIZE_TYPE_ANY,0L);
			return MAILBOX_FAILED;
		}
	}
	return 0L;
}

//
//The implementation of SendMail.
//This routine does the following:
// 1.
// 

static DWORD SendMail(__COMMON_OBJECT* lpThis,LPVOID lpMsg,DWORD dwTimeOut)
{
	__MAILBOX*                           lpMailBox       = NULL;
	__KERNEL_THREAD_OBJECT*              lpKernelThread  = NULL;
	DWORD                                dwFlags         = 0L;

	if((NULL == lpThis) || (NULL == lpMsg))  //Parameters check.
		return MAILBOX_FAILED;

	lpMailBox = (__MAILBOX*)lpThis;
	//ENTER_CRITICAL_SECTION();
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	if(NULL == lpMailBox->lpMsg)
	{
		lpMailBox->lpMsg = lpMsg;    //Send the mail.
		lpKernelThread = (__KERNEL_THREAD_OBJECT*)
			lpMailBox->lpGettingQueue->GetHeaderElement(
			(__COMMON_OBJECT*)lpMailBox->lpGettingQueue,
			NULL);
		if(NULL == lpKernelThread)    //There is not any kernel thread waiting for message.
		{
			//LEAVE_CRITICAL_SECTION();
			__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
			return MAILBOX_SUCCESS;
		}
		else               //There is one kernel thread waiting for message at least.
		{
			lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_READY;
			KernelThreadManager.AddReadyKernelThread(
				(__COMMON_OBJECT*)&KernelThreadManager,
				lpKernelThread);  //Add to ready queue.
			__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
			return MAILBOX_SUCCESS;
		}
	}
	else    //The mailbox is full.
	{
		//LEAVE_CRITICAL_SECTION();
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		return MAILBOX_FAILED;
	}
}

//
//The implementation of MailBoxTimerHandler.
//This routine is called by timer object,to indicate the timeout event,and this routine
//runs at the interrupt context.
//It does the following:
// 1. Check the reason that kernel thread set a timer;
// 2. If the reason is to timeout the send event,then try to wake up the sending thread;
// 3. If the reason is to timeout the get event,then try to wake up a getting thread;
// 4. Set the wake up reason correctly,to tell the target kernel thread proper reason
//    that it waken up.
//

static DWORD MailBoxTimerHandler(LPVOID lpParam)
{
	__MAILBOX_TIMERH_PARAM*             lpHandlerParam      = NULL;
	__PRIORITY_QUEUE*                   lpWaitingQueue      = NULL;
	__KERNEL_THREAD_OBJECT*             lpKernelThread      = NULL;
	
	if(NULL == lpParam)
		return 0L;
	lpHandlerParam = (__MAILBOX_TIMERH_PARAM*)lpParam;
	switch(lpHandlerParam->dwSetReason)
	{
	case TIMER_SET_REASON_GETTING:
		lpWaitingQueue = lpHandlerParam->lpMailBox->lpGettingQueue;
		lpKernelThread = lpHandlerParam->lpKernelThread;
		if(!lpWaitingQueue->DeleteFromQueue((__COMMON_OBJECT*)lpWaitingQueue,
			(__COMMON_OBJECT*)lpKernelThread))    //The kernel thread maybe waken up.
			return 0L;
		lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_READY;  //Wake up the thread.
		KernelThreadManager.AddReadyKernelThread(
			(__COMMON_OBJECT*)&KernelThreadManager,
			lpKernelThread);  //Add to ready queue.

		lpHandlerParam->dwWakeupReason = WAKE_UP_REASON_TIMEOUT;
		return 1L;
		break;
	case TIMER_SET_REASON_SENDING:
		break;
	default:
		return 0L;
	}
	return 0L;
}

