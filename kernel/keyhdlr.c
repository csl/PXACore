#ifndef __STDAFX_H__
#include "..\INCLUDE\StdAfx.h"
#endif

//
//The following procedure is used by master to dispatch the key board
//event.
//This event parase the dwParam,and set up a __MSG struct,initialize
//it,and add a message to the current task.
//
VOID _KeyHandler(DWORD dwParam)
{
	//__KTHREAD_MSG Msg;
	//__KERNEL_THREAD_MESSAGE Msg;
	struct __DEVICE_MESSAGE Msg;

	//Mark in interrupt context.
	System.ucIntNestLevel += 1;
	//Msg.wCommand   = KeyUpEvent(dwParam) ? KTMSG_KEY_UP : KTMSG_KEY_DOWN;
	//Msg.dwParam    = dwParam;

	Msg.wDevMsgType   = KeyUpEvent(dwParam) ? KTMSG_KEY_UP : KTMSG_KEY_DOWN;
	Msg.dwDevMsgParam = dwParam;

	DeviceInputManager.SendDeviceMessage((__COMMON_OBJECT*)&DeviceInputManager, &Msg, NULL);
	//SendMessage((__COMMON_OBJECT*)g_lpShellThread,&Msg);
	System.ucIntNestLevel -= 1;
	return;
}
