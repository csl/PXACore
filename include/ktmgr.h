//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct,18 2004
//    Module Name               : ktmgr.h
//    Module Funciton           : 
//                                This module countains kernel thread and kernel thread 
//                                manager's definition.
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

#ifndef __KTMGR_H__
#define __KTMGR_H__

//
//***********************************************************************
//
#define SYSTEM_TIME_SLICE  10    //***** debug *****
#define MAX_THREAD_NAME    32    //Maximal kernel thread name's length.

DECLARE_PREDEFINED_OBJECT(__KERNEL_FILE)
DECLARE_PREDEFINED_OBJECT(__EVENT)


//Kernel thread's context.

BEGIN_DEFINE_OBJECT(__KERNEL_THREAD_CONTEXT)
    DWORD           dwEFlags;
    WORD            wCS;
	WORD            wReserved;
	DWORD           dwEIP;
	DWORD           dwEAX;
	DWORD           dwEBX;
	DWORD           dwECX;
	DWORD           dwEDX;
	DWORD           dwESI;
	DWORD           dwEDI;
	DWORD           dwEBP;
	DWORD           dwESP;

//
//The following macro is used to initialize a kernel thread's executation
//context.
//

#define INIT_EFLAGS_VALUE 512
#define INIT_KERNEL_THREAD_CONTEXT_I(lpContext,initEip,initEsp) \
    (lpContext)->dwEFlags    = INIT_EFLAGS_VALUE;               \
    (lpContext)->wCS         = 0x08;                            \
	(lpContext)->wReserved   = 0x00;                            \
	(lpContext)->dwEIP       = initEip;                         \
	(lpContext)->dwEAX       = 0x00000000;                      \
	(lpContext)->dwEBX       = 0x00000000;                      \
	(lpContext)->dwECX       = 0x00000000;                      \
	(lpContext)->dwEDX       = 0x00000000;                      \
	(lpContext)->dwESI       = 0x00000000;                      \
	(lpContext)->dwEDI       = 0x00000000;                      \
	(lpContext)->dwEBP       = 0x00000000;                      \
	(lpContext)->dwESP       = initEsp;

//
//In order to access the members of a context giving it's base address,
//we define the following macros to make this job easy.
//

#define CONTEXT_OFFSET_EFLAGS        0x00
#define CONTEXT_OFFSET_CS            0x04
#define CONTEXT_OFFSET_EIP           0x08
#define CONTEXT_OFFSET_EAX           0x0C
#define CONTEXT_OFFSET_EBX           0x10
#define CONTEXT_OFFSET_ECX           0x14
#define CONTEXT_OFFSET_EDX           0x18
#define CONTEXT_OFFSET_ESI           0x1C
#define CONTEXT_OFFSET_EDI           0x20
#define CONTEXT_OFFSET_EBP           0x24
#define CONTEXT_OFFSET_ESP           0x28

END_DEFINE_OBJECT()

//
//Common synchronization object's definition.
//The common synchronization object is a abstract object,all synchronization objects,such
//as event,mutex,etc,all inherited from this object.
//

BEGIN_DEFINE_OBJECT(__COMMON_SYNCHRONIZATION_OBJECT)
    DWORD                (*WaitForThisObject)(__COMMON_SYNCHRONIZATION_OBJECT*);
END_DEFINE_OBJECT()

//
//The following macro is defined to be used by other objects to inherit from
//common synchronization object.
//
#define INHERIT_FROM_COMMON_SYNCHRONIZATION_OBJECT \
	DWORD                (*WaitForThisObject)(__COMMON_OBJECT*);


//
//Kernel thread's message.
//

BEGIN_DEFINE_OBJECT(__KERNEL_THREAD_MESSAGE)
    WORD             wCommand;
    WORD             wParam;
	DWORD            dwParam;
	//DWORD            (*MsgAssocRoutine)(__KERNEL_THREAD_MESSAGE*);
END_DEFINE_OBJECT()

//
//The definition of kernel thread's message.
//

//#define KERNEL_MESSAGE_TERMINAL    0x0001
//#define KERNEL_MESSAGE_TIMER       0x0002

#ifndef MAX_KTHREAD_MSG_NUM
#define MAX_KTHREAD_MSG_NUM 32  //The maximal message number in the kernel thread's message
                                //queue.
#endif

								
#define KERNEL_THREAD_STATUS_RUNNING    0x00000001  //Kernel thread's status.
#define KERNEL_THREAD_STATUS_READY      0x00000002
#define KERNEL_THREAD_STATUS_SUSPENDED  0x00000003
#define KERNEL_THREAD_STATUS_SLEEPING   0x00000004
#define KERNEL_THREAD_STATUS_TERMINAL   0x00000005
#define KERNEL_THREAD_STATUS_BLOCKED    0x00000006

//
//Definition of kernel thread's priority.
//In V1.5,there are 17 kernel thread priority levels,separated into
//five groups,that are,CRITICAL,HIGH,NORMAL,LOW,IDLE,and in each group
//expect IDLE,there are four levels.
//
#define MAX_KERNEL_THREAD_PRIORITY      0x00000010  //16

#define PRIORITY_LEVEL_CRITICAL         0x00000010
#define PRIORITY_LEVEL_CRITICAL_1       0x00000010
#define PRIORIRY_LEVEL_CRITICAL_2       0x0000000F
#define PRIORITY_LEVEL_CRITICAL_3       0x0000000E
#define PRIORITY_LEVEL_CRITICAL_4       0x0000000D

#define PRIORITY_LEVEL_HIGH             0x0000000C
#define PRIORITY_LEVEL_HIGH_1           0x0000000C
#define PRIORITY_LEVEL_HIGH_2           0x0000000B
#define PRIORITY_LEVEL_HIGH_3           0x0000000A
#define PRIORITY_LEVEL_HIGH_4           0x00000009

#define PRIORITY_LEVEL_NORMAL           0x00000008
#define PRIORITY_LEVEL_NORMAL_1         0x00000008
#define PRIORITY_LEVEL_NORMAL_2         0x00000007
#define PRIORITY_LEVEL_NORMAL_3         0x00000006
#define PRIORITY_LEVEL_NORMAL_4         0x00000005

#define PRIORITY_LEVEL_LOW              0x00000004
#define PRIORITY_LEVEL_LOW_1            0x00000004
#define PRIORITY_LEVEL_LOW_2            0x00000003
#define PRIORITY_LEVEL_LOW_3            0x00000002
#define PRIORITY_LEVEL_LOW_4            0x00000001

#define PRIORITY_LEVEL_IDLE             0x00000000

//
//The following macros alse can be used to define kernel
//thread's priority,they are aliases for above priority level.
//
#define PRIORITY_LEVEL_REALTIME         PRIORITY_LEVEL_CRITICAL_1
//#define PRIORITY_LEVEL_CRITICAL         PRIORITY_LEVEL_CRITICAL_4
#define PRIORITY_LEVEL_IMPORTANT        PRIORITY_LEVEL_HIGH_1
//#define PRIORITY_LEVEL_NORMAL           PRIORITY_LEVEL_NORMAL_1
//#define PRIORITY_LEVEL_LOW              PRIORITY_LEVEL_LOW_1
#define PRIORITY_LEVEL_LOWEST           PRIORITY_LEVEL_IDLE
//#define PRIORITY_LEVEL_INVALID          0x00000000

//#define DEFAULT_STACK_SIZE              0x00004000  //Default stack size,if the dwStackSize
								                    //parameter of CreateKernelThread is 0,
													//then the CreateKernelThread will allocate
													//DEFAULT_STACK_SIZE bytes to be used as
													//stack.

//
//The kernel thread object's definition.
//

BEGIN_DEFINE_OBJECT(__KERNEL_THREAD_OBJECT)
    INHERIT_FROM_COMMON_OBJECT
	INHERIT_FROM_COMMON_SYNCHRONIZATION_OBJECT
	__KERNEL_THREAD_CONTEXT              KernelThreadContext;
    __KERNEL_THREAD_CONTEXT*             lpKernelThreadContext;   //Added in V1.5.
    DWORD                                dwThreadID;
	DWORD                                dwThreadStatus;          //Kernel Thread's current
	                                                              //status.
	__PRIORITY_QUEUE*                    lpWaitingQueue;          //Waiting queue of the
	                                                              //kernel thread object.
	                                                              //One kernel thread who
	                                                              //want to wait the current
	                                                              //kernel thread object
	                                                              //will be put into this
	                                                              //queue.

	DWORD                                dwThreadPriority;        //Initialize priority.
	DWORD                                dwScheduleCounter;       //Schedule counter,used to
	                                                              //control the scheduler.
	DWORD                                dwReturnValue;

	DWORD                                dwTotalRunTime;
	DWORD                                dwTotalMemSize;

	LPVOID                               lpHeapObject;           //Pointing to the heap list.
	LPVOID                               lpDefaultHeap;          //Default heap.

	BOOL                                 bUsedMath;              //If used math co-process.

	DWORD                                dwStackSize;
	LPVOID                               lpInitStackPointer;

	DWORD                                (*KernelThreadRoutine)(LPVOID);  //Start address.
	LPVOID                               lpRoutineParam;

	//The following four members are used to manage the message queue of the
	//current kernel thread.
	__KERNEL_THREAD_MESSAGE              KernelThreadMsg[MAX_KTHREAD_MSG_NUM];
	UCHAR                                ucMsgQueueHeader;
	UCHAR                                ucMsgQueueTrial;
	UCHAR                                ucCurrentMsgNum;
	UCHAR                                ucAligment;
	__PRIORITY_QUEUE*                    lpMsgWaitingQueue;

	DWORD                                dwUserData;  //User custom data.
	DWORD                                dwLastError;
	UCHAR                                KernelThreadName[MAX_THREAD_NAME];

	//Used by synchronous object to indicate the waiting result.
	DWORD                                dwWaitingStatus;
END_DEFINE_OBJECT()

//Kernel thread object's initialize and uninitialize routine's definition.
BOOL KernelThreadInitialize(__COMMON_OBJECT* lpThis);
VOID KernelThreadUninitialize(__COMMON_OBJECT* lpThis);


typedef DWORD (*__KERNEL_THREAD_ROUTINE)(LPVOID);  //Kernel thread's start routine.
typedef DWORD (*__THREAD_HOOK_ROUTINE)(__KERNEL_THREAD_OBJECT*,
									   DWORD*);    //Thread hook's protype.

#define THREAD_HOOK_TYPE_CREATE        0x00000001
#define THREAD_HOOK_TYPE_ENDSCHEDULE   0x00000002
#define THREAD_HOOK_TYPE_BEGINSCHEDULE 0x00000004
#define THREAD_HOOK_TYPE_TERMINAL      0x00000008

//Kernel Thread Manager's definition.

BEGIN_DEFINE_OBJECT(__KERNEL_THREAD_MANAGER)
    DWORD                                    dwCurrentIRQL;
    __KERNEL_THREAD_OBJECT*           lpCurrentKernelThread;   //Current kernel thread.

    __PRIORITY_QUEUE*                        lpRunningQueue;
	__PRIORITY_QUEUE*                        lpSuspendedQueue;
	__PRIORITY_QUEUE*                        lpSleepingQueue;
	__PRIORITY_QUEUE*                        lpTerminalQueue;
	__PRIORITY_QUEUE*                        ReadyQueue[MAX_KERNEL_THREAD_PRIORITY + 1];

	DWORD                                    dwNextWakeupTick;

	__THREAD_HOOK_ROUTINE                    lpCreateHook;
	__THREAD_HOOK_ROUTINE                    lpEndScheduleHook;
	__THREAD_HOOK_ROUTINE                    lpBeginScheduleHook;
	__THREAD_HOOK_ROUTINE                    lpTerminalHook;

	__THREAD_HOOK_ROUTINE                    (*SetThreadHook)(
		                                     DWORD          dwHookType,
											 __THREAD_HOOK_ROUTINE lpNew);
	VOID                                     (*CallThreadHook)(
		                                     DWORD          dwHookType,
											 __KERNEL_THREAD_OBJECT* lpPrev,
											 __KERNEL_THREAD_OBJECT* lpNext);

	//Get a schedulable kernel thread from ready queue.
	__KERNEL_THREAD_OBJECT*                  (*GetScheduleKernelThread)(
		                                      __COMMON_OBJECT*  lpThis,
											  DWORD           dwPriority);

	//Add a ready kernel thread to ready queue.
	VOID                                     (*AddReadyKernelThread)(
		                                      __COMMON_OBJECT*  lpThis,
											  __KERNEL_THREAD_OBJECT* lpThread);

	BOOL                                     (*Initialize)(__COMMON_OBJECT* lpThis);

	__KERNEL_THREAD_OBJECT*                  (*CreateKernelThread)(
		                                      __COMMON_OBJECT*          lpThis,
											  DWORD                     dwStackSize,
											  DWORD                     dwStatus,
											  DWORD                     dwPriority,
											  __KERNEL_THREAD_ROUTINE   lpStartRoutine,
											  LPVOID                    lpRoutineParam,
											  LPVOID                    lpReserved,
											  LPSTR                     lpszName);

	VOID                                     (*DestroyKernelThread)(__COMMON_OBJECT* lpThis,
		                                     __COMMON_OBJECT*           lpKernelThread
											 );

	BOOL                                     (*SuspendKernelThread)(
		                                     __COMMON_OBJECT*           lpThis,
											 __COMMON_OBJECT*           lpKernelThread
											 );

	BOOL                                     (*ResumeKernelThread)(
		                                     __COMMON_OBJECT*           lpThis,
											 __COMMON_OBJECT*           lpKernelThread
											 );

	VOID                                     (*ScheduleFromProc)(
		                                     __KERNEL_THREAD_CONTEXT*   lpContext
											 );
	VOID                                     (*ScheduleFromInt)(
		                                     __COMMON_OBJECT*           lpThis,
											 LPVOID                     lpESP
											 );

	DWORD                                    (*SetThreadPriority)(
											 __COMMON_OBJECT*           lpKernelThread,
											 DWORD                      dwNewPriority
											 );

	DWORD                                    (*GetThreadPriority)(
		                                     __COMMON_OBJECT*           lpKernelThread
											 );

	DWORD                                    (*TerminalKernelThread)(
		                                     __COMMON_OBJECT*           lpThis,
											 __COMMON_OBJECT*           lpKernelThread
											 );

	BOOL                                     (*Sleep)(
		                                     __COMMON_OBJECT*           lpThis,
											 //__COMMON_OBJECT*           lpKernelThread,
											 DWORD                      dwMilliSecond
											 );

	BOOL                                     (*CancelSleep)(
		                                     __COMMON_OBJECT*           lpThis,
											 __COMMON_OBJECT*           lpKernelThread
											 );

	DWORD                                    (*SetCurrentIRQL)(
		                                     __COMMON_OBJECT*           lpThis,
											 DWORD                      dwNewIRQL
											 );

	DWORD                                    (*GetCurrentIRQL)(
		                                     __COMMON_OBJECT*           lpThis
											 );

	DWORD                                    (*GetLastError)(
		                                     //__COMMON_OBJECT*           lpKernelThread
											 );
	
	DWORD                                    (*SetLastError)(
		                                     //__COMMON_OBJECT*           lpKernelThread,
											 DWORD                      dwNewError
											 );

	DWORD                                    (*GetThreadID)(
		                                     __COMMON_OBJECT*           lpKernelThread
											 );

	DWORD                                    (*GetThreadStatus)(
		                                     __COMMON_OBJECT*           lpKernelThread
											 );

	DWORD                                    (*SetThreadStatus)(
		                                     __COMMON_OBJECT*           lpKernelThread,
											 DWORD                      dwStatus
											 );

	BOOL                                     (*SendMessage)(
		                                     __COMMON_OBJECT*           lpKernelThread,
											 __KERNEL_THREAD_MESSAGE*   lpMsg
											 );

	BOOL                                     (*GetMessage)(
		                                     __COMMON_OBJECT*           lpKernelThread,
											 __KERNEL_THREAD_MESSAGE*   lpMsg
											 );

	BOOL                                     (*MsgQueueFull)(
		                                     __COMMON_OBJECT*           lpKernelThread
											 );

	BOOL                                     (*MsgQueueEmpty)(
		                                     __COMMON_OBJECT*           lpKernelThread
											 );

	BOOL                                     (*LockKernelThread)(
		                                     __COMMON_OBJECT*           lpThis,
											 __COMMON_OBJECT*           lpKernelThread);

	VOID                                     (*UnlockKernelThread)(
		                                     __COMMON_OBJECT*           lpThis,
											 __COMMON_OBJECT*           lpKernelThread);

END_DEFINE_OBJECT()          //End of the kernel thread manager's definition.

//
//Global functions declare.
//The following routines are used to operate the kernel thread's message queue.
//
typedef DWORD (*__KERNEL_THREAD_MESSAGE_HANDLER)(WORD,WORD,DWORD);  //The protype of event handler.

DWORD DispatchMessage(__KERNEL_THREAD_MESSAGE*,__KERNEL_THREAD_MESSAGE_HANDLER);
                                                                  //The routine dispatch a
                                                                  //message to it's handler.

BOOL  GetMessage(__KERNEL_THREAD_MESSAGE*);                       //Get a message from the 
                                                                  //current kernel thread's
                                                                  //message queue.

BOOL  SendMessage(__COMMON_OBJECT* lpThread,__KERNEL_THREAD_MESSAGE*); //Send a message to the
                                                                       //kernel thread.


/***************************************************************************************
****************************************************************************************
****************************************************************************************
****************************************************************************************
***************************************************************************************/

extern __KERNEL_THREAD_MANAGER KernelThreadManager;

//--------------------------------------------------------------------------------------
//
//                          SYNCHRONIZATION OBJECTS
//
//--------------------------------------------------------------------------------------


//
//Event object's definition.
//The event object is inherited from common object and common synchronization object.
//

BEGIN_DEFINE_OBJECT(__EVENT)
    INHERIT_FROM_COMMON_OBJECT
	INHERIT_FROM_COMMON_SYNCHRONIZATION_OBJECT
	DWORD                 dwEventStatus;
    __PRIORITY_QUEUE*     lpWaitingQueue;
	DWORD                 (*SetEvent)(__COMMON_OBJECT*);
	DWORD                 (*ResetEvent)(__COMMON_OBJECT*);
	DWORD                 (*WaitForThisObjectEx)(__COMMON_OBJECT*,
		                                         DWORD);  //Time out waiting operation.
END_DEFINE_OBJECT()

#define EVENT_STATUS_FREE            0x00000001    //Event status.
#define EVENT_STATUS_OCCUPIED        0x00000002

//
//The following values are returned by WaitForThisObjectEx routine.
//
#define OBJECT_WAIT_MASK             0x0000FFFF
#define OBJECT_WAIT_WAITING          0x00000000
#define OBJECT_WAIT_FAILED           0x00000000
#define OBJECT_WAIT_RESOURCE         0x00000001
#define OBJECT_WAIT_TIMEOUT          0x00000002
#define OBJECT_WAIT_DELETED          0x00000004

BOOL EventInitialize(__COMMON_OBJECT*);            //The event object's initializing routine
VOID EventUninitialize(__COMMON_OBJECT*);          //and uninitializing routine.

//--------------------------------------------------------------------------------------
//
//                                MUTEX
//
//---------------------------------------------------------------------------------------

//
//The definition of MUTEX object.
//

BEGIN_DEFINE_OBJECT(__MUTEX)
    INHERIT_FROM_COMMON_OBJECT                  //Inherit from __COMMON_OBJECT.
	INHERIT_FROM_COMMON_SYNCHRONIZATION_OBJECT  //Inherit from common synchronization object.
	DWORD             dwMutexStatus;
    DWORD             dwWaitingNum;
    __PRIORITY_QUEUE* lpWaitingQueue;
    DWORD             (*ReleaseMutex)(__COMMON_OBJECT* lpThis);
	DWORD             (*WaitForThisObjectEx)(__COMMON_OBJECT* lpThis,
		                                     DWORD dwMillionSecond); //Extension waiting.
END_DEFINE_OBJECT()

#define MUTEX_STATUS_FREE      0x00000001
#define MUTEX_STATUS_OCCUPIED  0x00000002

//
//The initializing routine of MUTEX object and uninitializing routine.
//

BOOL MutexInitialize(__COMMON_OBJECT* lpThis);
VOID MutexUninitialize(__COMMON_OBJECT* lpThis);

//-----------------------------------------------------------------------------------
//
//                            SEMAPHORE
//
//-----------------------------------------------------------------------------------


#endif
