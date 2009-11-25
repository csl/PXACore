//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct,22 2006
//    Module Name               : STAT_S.CPP
//    Module Funciton           : 
//                                Countains CPU overload ratio statistics related
//                                data structures and routines.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include "stdafx.h"
#include "stat_s.h"
#include "statcpu.h"
#include "l_stdio.h"

#define KERNEL_MESSAGE_TIMER        0x0006

struct __KERNEL_THREAD_OBJECT*  lpStatKernelThread = NULL;  //Used to save statistics kernel thread's object.

//
//The following routine is used to print out all devices information
//in system.
//
static VOID ShowDevList()
{
	struct __DEVICE_OBJECT*   lpDevObject = IOManager.lpDeviceRoot;
	BYTE strInfo[80];
	
	printf("    Device Name    Attribute    BlockSize    RdSize/WrSize\n");
	while(lpDevObject)
	{
		sprintf(strInfo,"    %11s   0x%8X    %9d    %d/%d",
			lpDevObject->DevName,
			lpDevObject->dwAttribute,
			lpDevObject->dwBlockSize,
			lpDevObject->dwMaxReadSize,
			lpDevObject->dwMaxWriteSize);
		printf("%s\n", strInfo);
		lpDevObject = lpDevObject->lpNext;
	}
}

//
//This routine is used to print out CPU statistics information.
//
static VOID ShowStatInfo()  //Display the statistics information.
{
	struct __THREAD_STAT_OBJECT* lpStatObj = &StatCpuObject.IdleThreadStatObj;
	char Buff[MAX_BUFFER_SIZE];

	//Print table header.
	printf("      Thread Name  Thread ID  1s usage  60s usage  5m usage\n");
	printf("    -------------  ---------  --------  ---------  --------\n");
	//For each kernel thread,print out statistics information.
	do{
		sprintf(Buff,"    %13s  %9X  %6d.%d  %7d.%d  %6d.%d",
			lpStatObj->lpKernelThread->KernelThreadName,
			lpStatObj->lpKernelThread->dwThreadID,
			lpStatObj->wCurrPeriodRatio / 10,
			lpStatObj->wCurrPeriodRatio % 10,
			lpStatObj->wOneMinuteRatio  / 10,
			lpStatObj->wOneMinuteRatio  % 10,
			lpStatObj->wMaxStatRatio    / 10,
			lpStatObj->wMaxStatRatio    % 10
			//lpStatObj->TotalCpuCycle.dwHighPart,
			//lpStatObj->TotalCpuCycle.dwLowPart
			);
		printf("%s\n", Buff);

		lpStatObj = lpStatObj->lpNext;
	}while(lpStatObj != &StatCpuObject.IdleThreadStatObj);
}

//
//Entry point of the statistics kernel thread.
//
DWORD StatThreadRoutine(LPVOID lpData)
{
	struct __TIMER_OBJECT*           lpTimer  = NULL;
	struct __KERNEL_THREAD_MESSAGE   msg;

	//Set a timer,to calculate statistics information periodic.
	lpTimer = (struct __TIMER_OBJECT*)System.SetTimer((struct __COMMON_OBJECT*) &System,
		KernelThreadManager.lpCurrentKernelThread,
		1024,
		1000,
		NULL,
		NULL,
		TIMER_FLAGS_ALWAYS);

	while(TRUE)
	{
		if(GetMessage(&msg)) //Get message to process.
		{
			switch(msg.wCommand)
			{
			case KERNEL_MESSAGE_TIMER:
				StatCpuObject.DoStat();  //Do statistics.
				break;
			case STAT_MSG_SHOWSTATINFO:
				ShowStatInfo();
				break;
			case STAT_MSG_LISTDEV:  //List the device information.
				ShowDevList();
				break;
			case STAT_MSG_TERMINAL:  //Should exit.
				goto __EXIT;
			default:
				break;
			}
		}
	}

__EXIT:
	//Cancel the timer first.
	System.CancelTimer((struct __COMMON_OBJECT*)&System,
		(struct __COMMON_OBJECT*)lpTimer);
	return 0L;
}
