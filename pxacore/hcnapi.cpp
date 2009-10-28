/***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jun,24 2006
//    Module Name               : HCNAPI.CPP
//    Module Funciton           : 
//                                This module countains Hello Taiwan's API(Application
//                                Programming Interface)'s implementation.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __HCNAPI_H__
#include "HCNAPI.H"
#endif

//
//The implementation of CreateKernelThread routine.
//This routine only wraps CreateKernelThread routine of KernelThreadMgr object.
//
HANDLE CreateKernelThread(DWORD dwStackSize,
						  DWORD dwStatus,
						  DWORD dwPriority,
						  __KERNEL_THREAD_ROUTINE lpStartRoutine,
						  LPVOID lpRoutineParam,
						  LPVOID lpReserved)
{
	return (HANDLE)KernelThreadManager.CreateKernelThread(
		(__COMMON_OBJECT*)&KernelThreadManager,
		dwStackSize,
		dwStatus,
		dwPriority,
		lpStartRoutine,
		lpRoutineParam,
		lpReserved,
		NULL);
}

//
//The implementation of DestroyKernelThread.
//
VOID DestroyKernelThread(HANDLE hThread)
{
	KernelThreadManager.DestroyKernelThread(
		(__COMMON_OBJECT*)&KernelThreadManager,
		(__COMMON_OBJECT*)hThread);
}

//
//The implementation of WaitForThisObject routine.
//
DWORD WaitForThisObject(HANDLE hObject)
{
	__COMMON_OBJECT* lpCommonObject = (__COMMON_OBJECT*)hObject;
	__KERNEL_THREAD_OBJECT* lpThread = NULL;
	__EVENT* lpEvent = NULL;
	__MUTEX* lpMutex = NULL;

	if(NULL == lpCommonObject) //Invalid parameter.
		return 0L;

	switch(lpCommonObject->dwObjectType)
	{
	case OBJECT_TYPE_KERNEL_THREAD:
		lpThread = (__KERNEL_THREAD_OBJECT*)lpCommonObject;
		return lpThread->WaitForThisObject((__COMMON_OBJECT*)lpThread);
	case OBJECT_TYPE_EVENT:
		lpEvent = (__EVENT*)lpCommonObject;
		return lpEvent->WaitForThisObject((__COMMON_OBJECT*)lpEvent);
	case OBJECT_TYPE_MUTEX:
		lpMutex = (__MUTEX*)lpCommonObject;
		return lpMutex->WaitForThisObject((__COMMON_OBJECT*)lpMutex);
	default:
		break;
	}
	return 0L;
}