//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jul,20 2005
//    Module Name               : MAILBOX.H
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

#ifndef __MAILBOX_H__
#define __MAILBOX_H__
#endif

//
//The definition of Mailbox object.
//

#define MAILBOX_MSG_NUM    32    //How many messages one mailbox can countain.

BEGIN_DEFINE_OBJECT(__MAILBOX)
    INHERIT_FROM_COMMON_OBJECT
	INHERIT_FROM_COMMON_SYNCHRONIZATION_OBJECT
	DWORD                     (*SendMail)(struct __COMMON_OBJECT* lpThis,LPVOID lpMsg);
    	DWORD                     (*GetMail)(struct __COMMON_OBJECT* lpThis,LPVOID* llpMsg,DWORD dwTimeOut);
	BOOL                      (*CancelWaiting)(struct __COMMON_OBJECT* lpThis,struct __COMMON_OBJECT* lpKernelThread);

	LPVOID                    MsgArray[MAILBOX_MSG_NUM];
	DWORD                     dwMsgNum;
	struct __PRIORITY_QUEUE*         lpWaitingQueue;
END_DEFINE_OBJECT()    //End of the mailbox's definition.

#define MAILBOX_SUCCESS    0x00000001
#define MAILBOX_NO_SLOT    0x00000002
#define MAILBOX_TIMEOUT    0x00000003
#define MAILBOX_FAILED     0x00000004

//
//The following object is used by timer handler as parameter.
//

BEGIN_DEFINE_OBJECT(__TIMER_HANDLER_PARAM)
    	struct __MAILBOX*                lpMailBox;          //Mailbox.
	struct __KERNEL_THREAD_OBJECT*   lpKernelThread;     //Kernel thread object.
	BOOL                      bCanceled;          //Canceled flags.Once a kernel thread submit 
	                                              //a get message operation with timeout,but before
	                                              //timeout,there is a message availiable,then
	                                              //the kernel thread must cancel the timer.
	                                              //Set this variable to TRUE,can cancel a timer.
	LPVOID*                   llpHandlerParam;    //This variable is used to trace the status of
	                                              //Timer handler param.If the param object is
	                                              //free,then set the *llpHandlerParam to NULL.
	DWORD                     dwWakenupReason;    //Waken up reason.
END_DEFINE_OBJECT()

#define WAKEN_UP_REASON_TIMEOUT         0x00000001
#define WAKEN_UP_REASON_HAVE_MESSAGE    0x00000002

//
//Mailbox's initializing routine.
//

BOOL MailboxInitialize(struct __COMMON_OBJECT* lpThis);

//
//Mailbox's Uninitialize routine.
//

VOID MailboxUninitialize(struct __COMMON_OBJECT* lpThis);


//
//---------------------------- The definition of semaphore object ------------------------
//

BEGIN_DEFINE_OBJECT(__SEMAPHORE)
    INHERIT_FROM_COMMON_OBJECT
	INHERIT_FROM_COMMON_SYNCHRONIZATION_OBJECT
	DWORD                 dwSemaphoreNum;          //Semaphore number.
    	struct __PRIORITY_QUEUE*     lpWaitingQueue;          //Kernel thread queue.
	VOID                  ReleaseSemaphore(struct __COMMON_OBJECT* lpThis);
	DWORD                 GetSemaphore(struct __COMMON_OBJECT* lpThis,DWORD dwTimeOut);
END_DEFINE_OBJECT()

//
//The definition of Initialize routine.
//

BOOL SemaphoreInitialize(struct __COMMON_OBJECT* lpThis);

//
//The definition of Uninitialize routine.
//

VOID SemaphoreUninitialize(struct __COMMON_OBJECT* lpThis);

