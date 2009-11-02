//***********************************************************************/
//    Author                    : Garry
//    Original Date             : 2004-07-05
//    Module Name               : ktmsg.cpp
//    Module Funciton           : 
//                                This module countains kernal thread message
//                                implementation code.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __STDAFX_H__
#include "StdAfx.h"
#endif

__THREAD_MSG_QUEUE::__THREAD_MSG_QUEUE()
{
	LPVOID    pStartAddress = NULL;
	DWORD     dwSize        = 0L;

	pStartAddress = (LPVOID)&this->ktmsg[0];
	dwSize        = sizeof(__KTHREAD_MSG) * MAX_KTHREAD_MSG_NUM;
	MemZero(pStartAddress,dwSize);

	this->dwCurrentMsgNum = 0L;
	this->dwHeader        = 0L;
	this->dwTrial         = 0L;
}
