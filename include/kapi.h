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

#ifndef __I386
#define __I386
#endif

#ifndef __HELLO_TAIWAN__
#include "hellocn.h"
#endif

#ifndef __TYPES_H__
#include "TYPES.H"
#endif

#ifndef __COMMOBJ_H__
#include "COMMOBJ.H"
#endif

#ifndef __SYN_MECH_H__
#include "SYN_MECH.H"
#endif

#ifndef __COMQUEUE_H__
#include "COMQUEUE.H"
#endif

#ifndef __OBJQUEUE_H__
#include "objqueue.h"
#endif

#ifndef __KTMGR_H__
#include "ktmgr.h"
#endif

#ifndef __PERF_H__
#include "PERF.H"
#endif

#ifndef __SYSTEM_H__
#include "SYSTEM.H"
#endif

#ifndef __DIM_H__
#include "dim.h"
#endif

#ifndef __MEMMGR_H__
#include "memmgr.h"
#endif

#ifndef __PAGEIDX_H__
#include "PAGEIDX.H"
#endif

#ifndef __VMM_H__
#include "VMM.H"
#endif

#ifndef __KMEMMGR__
#include "kmemmgr.h"
#endif

#ifndef __ARCHSTD_H__
#include "ARCHSTD.H"
#endif

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

HANDLE SetTimer(DWORD dwTimerID,
				DWORD dwMillionSecond,
				__DIRECT_TIMER_HANDLER lpHandler,
				LPVOID lpHandlerParam,
				DWORD dwTimerFlags);

VOID CancelTimer(HANDLE hTimer);

HANDLE CreateEvent(BOOL bInitialStatus);

VOID DestroyEvent(HANDLE hEvent);

DWORD SetEvent(HANDLE hEvent);

DWORD ResetEvent(HANDLE hEvent);

HANDLE CreateMutex();

VOID DestroyMutex(HANDLE hMutex);

DWORD ReleaseMutex(HANDLE hEvent);

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
