//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Nov,02 2006
//    Module Name               : KAPI.CPP
//    Module Funciton           : 
//                                All routines in kernel module are wrapped
//                                in this file.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include "kapi.h"

HANDLE CreateKernelThread(DWORD dwStackSize,
						  DWORD dwInitStatus,
						  DWORD dwPriority,
						  __KERNEL_THREAD_ROUTINE lpStartRoutine,
						  LPVOID lpRoutineParam,
						  LPVOID lpReserved,
						  LPSTR lpszName)
{
	return (HANDLE)KernelThreadManager.CreateKernelThread(
		(struct __COMMON_OBJECT*)&KernelThreadManager,
		dwStackSize,
		dwInitStatus,
		dwPriority,
		lpStartRoutine,
		lpRoutineParam,
		lpReserved,
		lpszName);
}

VOID DestroyKernelThread(HANDLE hThread)
{
	KernelThreadManager.DestroyKernelThread(
		(struct __COMMON_OBJECT*)&KernelThreadManager,
		hThread);
}

DWORD SetLastError(DWORD dwNewError)
{
	return KernelThreadManager.SetLastError(
		dwNewError);
}

DWORD GetLastError()
{
	return KernelThreadManager.GetLastError();
}

DWORD GetThreadID(HANDLE hThread)
{
	return KernelThreadManager.GetThreadID(
		hThread);
}

DWORD SetThreadPriority(HANDLE hThread,DWORD dwPriority)
{
	return KernelThreadManager.SetThreadPriority(
		hThread,
		dwPriority);
}

BOOL GetMessage(MSG* lpMsg)
{
	__KERNEL_THREAD_OBJECT*  lpKernelThread = NULL;

	lpKernelThread = KernelThreadManager.lpCurrentKernelThread;
	return KernelThreadManager.GetMessage((struct __COMMON_OBJECT*) lpKernelThread, lpMsg);
}

BOOL SendMessage(HANDLE hThread,MSG* lpMsg)
{
	return KernelThreadManager.SendMessage(hThread,lpMsg);
}

BOOL Sleep(DWORD dwMillionSecond)
{
	return KernelThreadManager.Sleep(
		(struct __COMMON_OBJECT*)&KernelThreadManager,
		dwMillionSecond);
}
/*
HANDLE SetTimer(DWORD dwTimerID,
				DWORD dwMillionSecond,
				__DIRECT_TIMER_HANDLER lpHandler,
				LPVOID lpHandlerParam,
				DWORD dwTimerFlags)
{
	return System.SetTimer(
		(struct __COMMON_OBJECT*)&System,
		KernelThreadManager.lpCurrentKernelThread,
		dwTimerID,
		dwMillionSecond,
		lpHandler,
		lpHandlerParam,
		dwTimerFlags);
}

VOID CancelTimer(HANDLE hTimer)
{
	System.CancelTimer(
		(struct __COMMON_OBJECT*)&System,
		hTimer);
}
*/
HANDLE CreateEvent(BOOL bInitialStatus)
{
	struct __COMMON_OBJECT*         lpCommonObject    = NULL;
	__EVENT*                 lpEvent           = NULL;

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
	if(bInitialStatus)
		lpEvent->SetEvent((struct __COMMON_OBJECT*)lpEvent);

__TERMINAL:
	return lpCommonObject;
}

VOID DestroyEvent(HANDLE hEvent)
{
	ObjectManager.DestroyObject(&ObjectManager,hEvent);
}

DWORD SetEvent(HANDLE hEvent)
{
	return ((__EVENT*)hEvent)->SetEvent(hEvent);
}

DWORD ResetEvent(HANDLE hEvent)
{
	return ((__EVENT*)hEvent)->ResetEvent(hEvent);
}

HANDLE CreateMutex()
{
	__MUTEX*            lpMutex    = NULL;

	lpMutex = (__MUTEX*)ObjectManager.CreateObject(
		&ObjectManager,
		NULL,
		OBJECT_TYPE_MUTEX);
	if(NULL == lpMutex)
	{
		return NULL;
	}
	if(!lpMutex->Initialize((struct __COMMON_OBJECT*)lpMutex))  //Can not initialize.
	{
		ObjectManager.DestroyObject(
			&ObjectManager,
			(struct __COMMON_OBJECT*)lpMutex);
		return NULL;
	}

	return (struct __COMMON_OBJECT*)lpMutex;  //Create successfully.
}

VOID DestroyMutex(HANDLE hMutex)
{
	ObjectManager.DestroyObject(&ObjectManager,
		hMutex);
	return;
}

DWORD ReleaseMutex(HANDLE hEvent)
{
	return ((__MUTEX*)hEvent)->ReleaseMutex(hEvent);
}


DWORD WaitForThisObject(HANDLE hObject)
{
	struct __COMMON_OBJECT* lpCommonObject = (struct __COMMON_OBJECT*)hObject;
	__KERNEL_THREAD_OBJECT* lpThread = NULL;
	__EVENT* lpEvent = NULL;
	__MUTEX* lpMutex = NULL;

	if(NULL == lpCommonObject) //Invalid parameter.
		return OBJECT_WAIT_FAILED;

	switch(lpCommonObject->dwObjectType)
	{
	case OBJECT_TYPE_KERNEL_THREAD:
		lpThread = (__KERNEL_THREAD_OBJECT*)lpCommonObject;
		return lpThread->WaitForThisObject((struct __COMMON_OBJECT*)lpThread);
	case OBJECT_TYPE_EVENT:
		lpEvent = (__EVENT*)lpCommonObject;
		return lpEvent->WaitForThisObject((struct __COMMON_OBJECT*)lpEvent);
	case OBJECT_TYPE_MUTEX:
		lpMutex = (__MUTEX*)lpCommonObject;
		return lpMutex->WaitForThisObject((struct __COMMON_OBJECT*)lpMutex);
	default:
		break;
	}
	return OBJECT_WAIT_FAILED;
}
/*
DWORD WaitForThisObjectEx(HANDLE hObject,DWORD dwMillionSecond)
{
	struct __COMMON_OBJECT* lpCommonObject = (struct __COMMON_OBJECT*)hObject;
	__KERNEL_THREAD_OBJECT* lpThread = NULL;
	__EVENT* lpEvent = NULL;
	__MUTEX* lpMutex = NULL;

	if(NULL == lpCommonObject) //Invalid parameter.
		return OBJECT_WAIT_FAILED;

	switch(lpCommonObject->dwObjectType)
	{
	case OBJECT_TYPE_KERNEL_THREAD:
		return OBJECT_WAIT_FAILED;  //Don't support timeout waiting yet.
	case OBJECT_TYPE_EVENT:
		lpEvent = (__EVENT*)lpCommonObject;
		return lpEvent->WaitForThisObjectEx((struct __COMMON_OBJECT*)lpEvent,dwMillionSecond);
	case OBJECT_TYPE_MUTEX:
		lpMutex = (__MUTEX*)lpCommonObject;
		return lpMutex->WaitForThisObjectEx((struct __COMMON_OBJECT*)lpMutex,dwMillionSecond);
	default:
		break;
	}
	return OBJECT_WAIT_FAILED;
}

HANDLE ConnectInterrupt(__INTERRUPT_HANDLER lpInterruptHandler,
						LPVOID              lpHandlerParam,
						UCHAR               ucVector)
{
	return System.ConnectInterrupt(
		(struct __COMMON_OBJECT*)&System,
		lpInterruptHandler,
		lpHandlerParam,
		ucVector,
		0,
		0,
		0,
		TRUE,
		0);
}

VOID DisconnectInterrupt(HANDLE hInterrupt)
{
	System.DisconnectInterrupt(
		(struct __COMMON_OBJECT*)&System,
		hInterrupt);
}

LPVOID VirtualAlloc(LPVOID lpDesiredAddr,
					DWORD  dwSize,
					DWORD  dwAllocateFlags,
					DWORD  dwAccessFlags,
					UCHAR* lpszRegName)
{
	return lpVirtualMemoryMgr->VirtualAlloc(
		(struct __COMMON_OBJECT*)lpVirtualMemoryMgr,
		lpDesiredAddr,
		dwSize,
		dwAllocateFlags,
		dwAccessFlags,
		lpszRegName,
		NULL);
}

VOID VirtualFree(LPVOID lpVirtualAddr)
{
	lpVirtualMemoryMgr->VirtualFree(
		(struct __COMMON_OBJECT*)lpVirtualMemoryMgr,
		lpVirtualAddr);
}
*/
