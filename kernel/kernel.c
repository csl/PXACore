//***********************************************************************/
//    Author                    : Garry
//    Original Date             : May,02 2005
//    Module Name               : kernel.cpp
//    Module Funciton           : 
//                                
//    Last modified Author      : csl
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//***********************************************************************/

#include "stdafx.h"
#include "stat_s.h"
#include "l_stdio.h"

//Welcome information.
char* pszStartMsg1 = "Hello Taiwan for arm running now";
char* pszWelcome   = "Welcome to use Hello Taiwan!";

//Help information.
char* pszHelpInfo = "Any help please press 'help' + return.";

//External global variables.
extern __DRIVER_ENTRY DriverEntryArray[];


void DeadLoop()
{
	while(true);
}

//System idle thread.
//When there is no kernel thread to schedule,the system idle thread will run.
//In this kernel thread,some system level tasks can be done.
//This kernel thread will never exit until the system is down.
#define GLOBAL_COUNTER 1024*1024*8*20

/**/
static DWORD dwIdleCounter = 0L;
DWORD SystemIdle()
{

	while(TRUE)
	{
		dwIdleCounter++;
		if(GLOBAL_COUNTER == dwIdleCounter)
		{
			dwIdleCounter = 0;
			printf("System idle thread is sheduled,I-I-I-I-I-I-I-I-I-I-I.\n");
		}
	}

}

DWORD SystemShell(void)
{
	shell_execute();

	return 0L;
}

int main()
{
	struct __KERNEL_THREAD_OBJECT*       lpIdleThread     = NULL;
	struct __KERNEL_THREAD_OBJECT*       lpShellThread    = NULL;
	int dwIndex = 0;
	
	//The following code is executed in no-interruptable envrionment.
	DisableInterrupt();    
	
	//Initialize Kernel Thread Manager.
	if(!KernelThreadManager.Initialize((struct __COMMON_OBJECT*) &KernelThreadManager))
		goto __TERMINAL; 

	//Initialize the Device manager.
	if(!DeviceManager.Initialize(&DeviceManager))  goto __TERMINAL;

	//Initialize Input-Output Manager.
	if(!IOManager.Initialize((struct __COMMON_OBJECT*)&IOManager))  goto __TERMINAL;

	//Initialize system object.
	if(!System.Initialize((struct __COMMON_OBJECT*) &System))                
		goto __TERMINAL;

	//Initialize  Driver
	dwIndex = 0;
	while(DriverEntryArray[dwIndex])
	{
		if(!IOManager.LoadDriver(DriverEntryArray[dwIndex])) //Failed to load.
		{
			printf("Failed to load the %d device driver.\n", dwIndex);
		}
		dwIndex++;  //Continue to load.
	}

	
	lpIdleThread = KernelThreadManager.CreateKernelThread(    //Create system idle thread.
		(struct __COMMON_OBJECT*)&KernelThreadManager,
		0L,
		KERNEL_THREAD_STATUS_READY,
		PRIORITY_LEVEL_NORMAL,         //Lowest priority level.
		SystemIdle,
		(LPVOID)(&dwIdleCounter),
		NULL,
		"IDLE");

	if(NULL == lpIdleThread)
	{
		printf("Can not create idle kernel thread,please restart the system.\n");
		//__ERROR_HANDLER(ERROR_LEVEL_FATAL,0L,NULL);
		goto __TERMINAL;
	}

	lpShellThread = KernelThreadManager.CreateKernelThread(   //Create shell thread.
		(struct __COMMON_OBJECT*)&KernelThreadManager,
		0L,
		KERNEL_THREAD_STATUS_READY,
		PRIORITY_LEVEL_NORMAL,
		SystemShell,
		NULL,
		NULL,
		"SHELL");

	if(NULL == lpShellThread)
	{
		printf("Can not create system shell thread,please restart the system.");
		//__ERROR_HANDLER(ERROR_LEVEL_FATAL,0L,NULL);
		goto __TERMINAL;
	}

	printf("%s\n",pszWelcome);

	//Start first process
	lpShellThread->dwThreadStatus = KERNEL_THREAD_STATUS_RUNNING;
	KernelThreadManager.lpCurrentKernelThread = lpShellThread;
	RestoreKernelThread(lpShellThread);

	DeadLoop();

__TERMINAL:

    	printf("PXACore Dead\n");
	DeadLoop();
}
