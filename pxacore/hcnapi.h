//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jun,24 2006
//    Module Name               : HCNAPI.H
//    Module Funciton           : 
//                                This module countains Hello Taiwan's API(Application
//                                Programming Interface)'s definition.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __STDAFX_H__
#include ".\INCLUDE\StdAfx.h"
#endif

#ifndef __HCNAPI_H__
#define __HCNAPI_H__

#define HANDLE __COMMON_OBJECT*    //HANDLE's definition.

HANDLE CreateKernelThread(DWORD dwStackSize,    //Create a kernel thread.
						  DWORD dwStatus,
						  DWORD dwPriority,
						  __KERNEL_THREAD_ROUTINE lpStartRoutine,
						  LPVOID lpRoutineParam,
						  LPVOID lpReserved);

VOID DestroyKernelThread(HANDLE hThread);

DWORD WaitForThisObject(HANDLE hObject);

#endif
