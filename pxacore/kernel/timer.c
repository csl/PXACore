//***********************************************************************/
//    Author                    : Garry
//    Original Date             : May,31 2004
//    Module Name               : timer.h
//    Module Funciton           : 
//                                This module countains time handle source
//                                code.
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

//
//Global variables.
//

//Total time counter,it is combined by two parts,one is high part,which
//is a double word,and the other is low part,it is a word.
DWORD g_TimerCounterL = 0x00000000;
WORD  g_TimerCounterH = 0x0000;

//Time object slot array.
__TIME_SLOT_OBJECT TimeSlot[MAX_TIME_SLOT_OBJECT] = {0};

//Total time slot object number currently.
DWORD g_dwTimeObjectNum = 0x00000000;

BOOL AdjustSlot();
//Add a time object into slot.
BOOL SetTimer(__TASK_CTRL_BLOCK* pTCB,DWORD dwTimeSpan,WORD wTimerID)
{
	/*if(MAX_TIME_SLOT_OBJECT == g_dwTimeObjectNum)
		return FALSE;

	TimeSlot[g_dwTimeObjectNum].dwTimeSpan = dwTimeSpan;
	TimeSlot[g_dwTimeObjectNum].pTCB       = pTCB;
	TimeSlot[g_dwTimeObjectNum].wTimerID   = wTimerID;
	g_dwTimeObjectNum ++;*/

	WORD wr = 0x0000;
	for(wr = 0;wr < MAX_TIME_SLOT_OBJECT;wr ++)
	{
		if(0 == TimeSlot[wr].dwTimeSpan)
			break;
	}

	if(MAX_TIME_SLOT_OBJECT == wr)
		return FALSE;

	TimeSlot[wr].pTCB       = pTCB;
	TimeSlot[wr].dwTimeSpan = dwTimeSpan;
	TimeSlot[wr].wTimerID   = wTimerID;

	return TRUE;
}

//Adjust the time object slot,delete all free slots,and reorganize them.
BOOL AdjustSlot()
{
	WORD headptr  = 0x00;
	WORD trailptr = LOWORD(g_dwTimeObjectNum) - 1;

	while(headptr <= trailptr){                     //Adjust the time slot
		                                           //array.
		                                           //This algrithm keeps 2
		                                           //pointers:headptr and
		                                           //trailptr,the headptr
		                                           //points to the first 
		                                           //free slot,and the trail
		                                           //points to the last occupied
		                                           //time slot,then exchange them,
		                                           //repeat this.
		while((0 != TimeSlot[headptr].dwTimeSpan)
			&& (headptr < LOWORD(g_dwTimeObjectNum)))
			headptr ++;                            //Find the first free slot's
		                                           //index.
		while((0 == TimeSlot[trailptr].dwTimeSpan)
			   && trailptr)
			   trailptr --;                        //Find the first occupied slot.
		if(headptr > trailptr)
			break;

		TimeSlot[headptr].wTimerID    = TimeSlot[trailptr].wTimerID;
		TimeSlot[headptr].dwTimeSpan  = TimeSlot[trailptr].dwTimeSpan;  //Swap.
		TimeSlot[headptr].pTCB        = TimeSlot[trailptr].pTCB;
		TimeSlot[trailptr].dwTimeSpan = 0;
		g_dwTimeObjectNum --;
		headptr ++;
		if(trailptr)
			trailptr --;
	};

	return TRUE;
}

//
//Timer interrupt handler.
//The parameter,dwEsp,is the esp register's value before this procedure is
//called.So,the system can access the general registers by this pointer.
//
VOID TimerHandler(DWORD dwEsp)
{
	WORD  wr;
	__MSG msg;
	DWORD dwTmp = 0x00;
	//BYTE  Buffer[12];            //--------- ** debug ** ----------

	g_TimerCounterL ++;          //Increment the low part of timer counter.
	if(0xffffffff == g_TimerCounterL) 
	{
		g_TimerCounterL =  0;
		g_TimerCounterH += 1;
	}
	msg.wCommand = MSG_TIMER;    //Initialize the message.

	for(wr = 0;wr < /*LOWORD(g_dwTimeObjectNum)*/MAX_TIME_SLOT_OBJECT;wr ++)
	{
		if(0 == TimeSlot[wr].dwTimeSpan)
			continue;
		dwTmp = -- TimeSlot[wr].dwTimeSpan;
		if(0 == dwTmp)
		{
			msg.dwParam = TimeSlot[wr].wTimerID;
			SendMessage(TimeSlot[wr].pTCB,msg.wCommand,msg.dwParam);    //Send a message to the task.
		}

	}

	//ScheduleKThread(dwEsp);
	KernelThreadManager.ScheduleFromInt(
		(__COMMON_OBJECT*)&KernelThreadManager,
		(LPVOID)dwEsp);               //Re-schedule the kernal thread.

	//KernelThreadManager.ScheduleFromInt((__COMMON_OBJECT*)&KernelThreadManager,
	//	(LPVOID)dwEsp);

	//PrintLine("Timer handler called,g_dwTimeObjectNum = ");
	//Int2Str(g_dwTimeObjectNum,Buffer);
	//PrintStr(Buffer);

	//AdjustSlot();                //Readjust the time slot.
}

DWORD GetRunTime()
{
	return g_TimerCounterL;
}