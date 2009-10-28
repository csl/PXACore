//***********************************************************************/
//    Author                    : Garry
//    Original Date             : May,27 2004
//    Module Name               : taskctrl.cpp
//    Module Funciton           : 
//                                This module countains task control data
//                                structures's operation.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __STDAFX_H__
#include "..\INCLUDE\StdAfx.h"
#endif

#ifndef __HELLO_TAIWAN__
#include "hellocn.h"
#endif

#ifndef __TASK_CTRL__
#include "taskctrl.h"
#endif

#ifndef __GLOBAL_VAR__
#include "globvar.h"
#endif

//
//Task queue's operation implementation.
//
BOOL QueueFull(__TASK_CTRL_BLOCK *pTCB)
{
	return MAX_MESSAGE_NUM == pTCB->wCurrentMsgCounter ? TRUE : FALSE;
}

BOOL QueueEmpty(__TASK_CTRL_BLOCK *pTCB)
{
	return 0 == pTCB->wCurrentMsgCounter ? TRUE : FALSE;
}

BOOL SendMessage(__TASK_CTRL_BLOCK* pTCB,WORD wCommand,DWORD dwParam)
{
	//BYTE Buffer[12];         //---------- ** debug ** ----------

	if(QueueFull(pTCB))
		return FALSE;

	if(NULL == pTCB)
		return FALSE;

	//__asm{                   //--------- ** debug ** ----------
	//	mov eax,0x99999999
	//	mov eax,0x88888888
	//}
	pTCB->MsgQueue[pTCB->wTrial].dwParam   = dwParam;
	pTCB->MsgQueue[pTCB->wTrial].wCommand  = wCommand;
	pTCB->MsgQueue[pTCB->wTrial].wReserved = 0x0000;

	//PrintLine("Send a message to the current task.");   //-------- ** debug ** -------
	//Int2Str(DWORD(pTCB->MsgQueue[pTCB->wTrial].wCommand),Buffer);
	//PrintLine(Buffer);

	pTCB->wTrial ++;
	if(MAX_MESSAGE_NUM == pTCB->wTrial)
		pTCB->wTrial = 0x0000;
	pTCB->wCurrentMsgCounter ++;
	return TRUE;
}

BOOL GetMessage(__TASK_CTRL_BLOCK* pTCB,__MSG* pMsg)
{
	if(QueueEmpty(pTCB))
		return FALSE;
	if(NULL == pMsg)
		return FALSE;

	pMsg->dwParam    = pTCB->MsgQueue[pTCB->wHeader].dwParam;
	pMsg->wCommand   = pTCB->MsgQueue[pTCB->wHeader].wCommand;
	pMsg->wReserved  = 0x0000;
	pTCB->wHeader ++;
	if(MAX_MESSAGE_NUM == pTCB->wHeader)
		pTCB->wHeader = 0x0000;
	pTCB->wCurrentMsgCounter --;
	return TRUE;
}                                //End of the queue operation's implementation.


/*BOOL DispatchMessage(__MSG* pMsg,__EVENT_HANDLER pEventHandler)
{
	BOOL bResult = FALSE;
    
	if(NULL == pMsg)
		return bResult;
	if(NULL == pEventHandler)
		return bResult;
	pEventHandler(pMsg->wCommand,pMsg->dwParam,0L);  //Dispatch the message.
	bResult = TRUE;
	return bResult;
}*/


/************************************************************************/
//    Task Management Section.
/************************************************************************/

//
//Global variables.
//

//__TASK_CTRL_BLOCK*    g_pCurrentTask = NULL;
