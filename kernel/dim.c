//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Feb,25 2005
//    Module Name               : dim.cpp
//    Module Funciton           : 
//                                This module countains the implementation code of Device
//                                Input Manager.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include "stdafx.h"

//
//The implementation of Initialize routine.
//

static BOOL DimInitialize(struct __COMMON_OBJECT* lpThis,
						  struct __COMMON_OBJECT* lpFocusThread,
						  struct __COMMON_OBJECT* lpShellThread)
{
	struct __DEVICE_INPUT_MANAGER*    lpInputMgr = NULL;

	if(NULL == lpThis)    //Parameter check.
		return FALSE;

	lpInputMgr = (struct __DEVICE_INPUT_MANAGER*)lpThis;

	lpInputMgr->lpFocusKernelThread = (struct __KERNEL_THREAD_OBJECT*)lpFocusThread;
	lpInputMgr->lpShellKernelThread = (struct __KERNEL_THREAD_OBJECT*)lpShellThread;

	return TRUE;
}


//
//The implementation of SendDeviceMessage.
//

static DWORD SendDeviceMessage(struct __COMMON_OBJECT*    lpThis,
							   struct __DEVICE_MESSAGE*   lpDevMsg,
							   struct __COMMON_OBJECT*    lpTarget)
{
	struct __DEVICE_INPUT_MANAGER*    lpInputMgr     = NULL;
	//struct __KERNEL_THREAD_OBJECT*    lpTargetThread = NULL;
	__KERNEL_THREAD_MESSAGE*   lpThreadMsg    = NULL;
	DWORD                      dwFlags        = 0L;

	if((NULL == lpThis) || (NULL == lpDevMsg))    //Parameter check.
		return DEVICE_MANAGER_FAILED;

	lpInputMgr   = (struct __DEVICE_INPUT_MANAGER*)lpThis;
	lpThreadMsg  = (__KERNEL_THREAD_MESSAGE*)lpDevMsg;

	if(lpTarget != NULL)
	{
		SendMessage(lpTarget, lpThreadMsg);
		return DEVICE_MANAGER_SUCCESS;
	}

	if(lpInputMgr->lpFocusKernelThread != NULL)
	{
		if(KERNEL_THREAD_STATUS_TERMINAL == lpInputMgr->lpFocusKernelThread->dwThreadStatus)    
			//The current focus
			//kernel thread is
			//terminal,so we must
			//reset the current
			//focus kernel thread
			//pointer,and send the
			//device message to shell
			//kernel thread.
		{
			//ENTER_CRITICAL_SECTION();
			__ENTER_CRITICAL_SECTION(NULL,dwFlags);
			lpInputMgr->lpFocusKernelThread = NULL;
			//LEAVE_CRITICAL_SECTION();
			__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

			if(NULL != lpInputMgr->lpShellKernelThread)
			{
				SendMessage((struct __COMMON_OBJECT*)(lpInputMgr->lpShellKernelThread),
					lpThreadMsg);
				return DEVICE_MANAGER_SUCCESS;
			}
			else    //The current shell kernel thread is not exists.
			{
				return DEVICE_MANAGER_NO_SHELL_THREAD;
			}
		}           //The current status of the focus kernel thread is not TERMINAL.
		else
		{
			SendMessage((struct __COMMON_OBJECT*)lpInputMgr->lpFocusKernelThread,
				lpThreadMsg);
			return DEVICE_MANAGER_SUCCESS;
		}
	}
	else            //The current focus kernel thread is not exists.
	{
		if(NULL != lpInputMgr->lpShellKernelThread)
		{
			SendMessage((struct __COMMON_OBJECT*)lpInputMgr->lpShellKernelThread,
				lpThreadMsg);
			return DEVICE_MANAGER_SUCCESS;
		}
		else
		{
			return DEVICE_MANAGER_NO_SHELL_THREAD;
		}
	}

	return DEVICE_MANAGER_SUCCESS;
}

//
//The implementation of SetFocusThread routine.
//

static struct __COMMON_OBJECT* SetFocusThread(struct __COMMON_OBJECT*  lpThis,
									   struct __COMMON_OBJECT*  lpFocusThread)
{
	struct __DEVICE_INPUT_MANAGER*    lpInputMgr = NULL;
	struct __KERNEL_THREAD_OBJECT*    lpRetVal   = NULL;
	DWORD                      dwFlags    = 0L;
	
	if(NULL == lpThis)    //Parameter check.
		return (struct __COMMON_OBJECT*)lpRetVal;

	lpInputMgr = (struct __DEVICE_INPUT_MANAGER*)lpThis;

	lpRetVal = lpInputMgr->lpFocusKernelThread;

	//ENTER_CRITICAL_SECTION();
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	lpInputMgr->lpFocusKernelThread = (struct __KERNEL_THREAD_OBJECT*)lpFocusThread;
	//LEAVE_CRITICAL_SECTION();
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

	return (struct __COMMON_OBJECT*)lpRetVal;
}

//
//The implementation of SetShellThread routine.
//

static struct __COMMON_OBJECT* SetShellThread(struct __COMMON_OBJECT*  lpThis,
									   struct __COMMON_OBJECT*  lpShellThread)
{
	struct __DEVICE_INPUT_MANAGER*    lpInputMgr = NULL;
	struct __KERNEL_THREAD_OBJECT*    lpRetVal   = NULL;
	DWORD                      dwFlags    = 0L;
	
	if(NULL == lpThis)    //Parameter check.
		return (struct __COMMON_OBJECT*)lpRetVal;

	lpInputMgr = (struct __DEVICE_INPUT_MANAGER*)lpThis;

	lpRetVal = lpInputMgr->lpShellKernelThread;

	//ENTER_CRITICAL_SECTION();
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	lpInputMgr->lpShellKernelThread = (struct __KERNEL_THREAD_OBJECT*)lpShellThread;
	//LEAVE_CRITICAL_SECTION();
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

	return (struct __COMMON_OBJECT*)lpRetVal;
}


/************************************************************************
*************************************************************************
*************************************************************************
*************************************************************************
************************************************************************/

//
//The definition of Global Object DeviceInputManager.
//

struct __DEVICE_INPUT_MANAGER DeviceInputManager = 
{
	NULL,                                     //lpFocusKernelThread.
	NULL,                                     //lpShellKernelThread.
	SendDeviceMessage,                        //SendDeviceMessage routine.
	SetFocusThread,                           //SetFocusThread routine.
	SetShellThread,                           //SetShellThread routine.
	DimInitialize                             //Initialize routine.
};


