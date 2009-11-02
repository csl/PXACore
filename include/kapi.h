/***********************************************************************/
//    Author                    : Garry
//    Original Date             : Nov,02 2006
//    Module Name               : KAPI.H
//    Module Funciton           : 
//                                Declares all kernel service routines can be
//                                used by other modules in kernel.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __KAPI_H__
#define __KAPI_H__
#endif
/*
#ifndef __I386
#define __I386
#endif

#ifndef __HELLO_TAIWAN__
#include "hellocn.h"
#endif
*/
#include "types.h"
#include "commobj.h"
//#include "syn_mech.h"
#include "comqueue.h"
#include "objqueue.h"
#include "ktmgr.h"
//#include "perf.h"
//#include "system.h"
#include "dim.h"
#include "memmgr.h"
//#include "PAGEIDX.H"
//#include "VMM.H"
#include "kmemmgr.h"
//#include "ARCHSTD.H"

typedef __COMMON_OBJECT* HANDLE;  //Use handle to refer any kernel object.
typedef __KERNEL_THREAD_MESSAGE MSG;

HANDLE CreateKernelThread(DWORD dwStackSize,
			  DWORD dwStatus,
			  DWORD dwPriority,
			  __KERNEL_THREAD_ROUTINE lpStartRoutine,
			  LPVOID lpRoutineParam,
			  LPVOID lpReserved,
			  LPSTR  lpszName);

VOID DestroyKernelThread(HANDLE hThread);

DWORD SetLastError(DWORD dwNewError);

DWORD GetLastError();

DWORD GetThreadID(HANDLE hThread);

DWORD SetThreadPriority(HANDLE hThread,DWORD dwPriority);

BOOL GetMessage(MSG* lpMsg);

BOOL SendMessage(HANDLE hThread,MSG* lpMsg);

BOOL Sleep(DWORD dwMillionSecond);

/*
HANDLE SetTimer(DWORD dwTimerID,
				DWORD dwMillionSecond,
				__DIRECT_TIMER_HANDLER lpHandler,
				LPVOID lpHandlerParam,
				DWORD dwTimerFlags);

VOID CancelTimer(HANDLE hTimer);
*/
HANDLE CreateEvent(BOOL bInitialStatus);

VOID DestroyEvent(HANDLE hEvent);

DWORD SetEvent(HANDLE hEvent);

DWORD ResetEvent(HANDLE hEvent);

HANDLE CreateMutex();

VOID DestroyMutex(HANDLE hMutex);

DWORD ReleaseMutex(HANDLE hEvent);
/*
DWORD WaitForThisObject(HANDLE hObject);

DWORD WaitForThisObjectEx(HANDLE hObject,DWORD dwMillionSecond);

HANDLE ConnectInterrupt(__INTERRUPT_HANDLER lpInterruptHandler,
			LPVOID              lpHandlerParam,
			UCHAR               ucVector);

VOID DisconnectInterrupt(HANDLE hInterrupt);

LPVOID VirtualAlloc(LPVOID lpDesiredAddr,
			DWORD  dwSize,
			DWORD  dwAllocateFlags,
			DWORD  dwAccessFlags,
			UCHAR* lpszRegName);

VOID VirtualFree(LPVOID lpVirtualAddr);
*/
