
#include "types.h"
//#include "port.h"

//#include "statcpu.h"
//#include "stat_s.h"
//#include "l_stdio.h"

//Welcome information.
char* pszStartMsg1 = "Hello Taiwan running now.If you have any question,";
char* pszStartMsg2 = "please send email to : garryxin@yahoo.com.cn.";
char* pszWelcome   = "Welcome to use Hello Taiwan!";

//Help information.
char* pszHelpInfo = "Any help please press 'help' + return.";

//External global variables.
//extern __DRIVER_ENTRY DriverEntryArray[];

//KEY_HANDLER g_keyHandler = NULL;
//INT_HANDLER g_intHandler = NULL;

extern void SerialInit();

void DeadLoop()
{
	while(true);
}

//
//System idle thread.
//When there is no kernel thread to schedule,the system idle thread will run.
//In this kernel thread,some system level tasks can be done.
//This kernel thread will never exit until the system is down.
//

#define GLOBAL_COUNTER 1024*1024*8*20
/**/
/*
static DWORD dwIdleCounter = 0L;
DWORD SystemIdle(LPVOID lpData)
{

	DWORD* lpdwCounter = NULL;
	lpdwCounter = (DWORD*)lpData;
	//DWORD dwCounter = 0L;
	while(TRUE)
	{
		(*lpdwCounter) ++;
		if(GLOBAL_COUNTER == *lpdwCounter)
		{
			(*lpdwCounter) = 0;
			//PrintLine("System idle thread is sheduled,I-I-I-I-I-I-I-I-I-I-I.");
		}
	}

}
*/
//
//System shell thread.
//This kernel thread is a shell.
//
/*
DWORD SystemShell(LPVOID)
{
	EntryPoint();
*/
	/*DWORD dwCounter = 0L;
	while(TRUE)
	{
		dwCounter ++;
		if(GLOBAL_COUNTER == dwCounter)
		{
			dwCounter = 0;
			PrintLine("System shell thread is scheduled.");
		}
	}*/
/*
	return 0L;
}
*/
//
//System keeper kernel thread.
//
/*
DWORD SystemKeeper(LPVOID)
{
	DWORD dwCounter = 0L;
	while(TRUE)
	{
		dwCounter ++;
		if(GLOBAL_COUNTER == dwCounter)
		{
			dwCounter = 0;
			PrintLine("System keeper thread is scheduled,K-K-K-K-K-K-K-K-K-K-K-K-K.");
		}
	}
}
*/
//
//The following function as a kernal thread function's wraper.
//The kernal thread's start routine's type is VOID KThreadRoutine(LPVOID).
//
/*
VOID ShellThreadWrap(LPVOID)
{
	EntryPoint();
}
*/
int main()
{
	//__KERNEL_THREAD_OBJECT*       lpIdleThread     = NULL;
	//__KERNEL_THREAD_OBJECT*       lpShellThread    = NULL;
	//__KERNEL_THREAD_OBJECT*       lpKeeperThread   = NULL;
	SerialInit();
	//init_interrupt_control();
	DeadLoop();

	return 0;
/*
	DWORD                         dwKThreadID      = 0;
	DWORD                         dwIndex          = 0;
	BYTE                          strInfo[64];

	DisableInterrupt();    //The following code is executed in no-interruptable envrionment.

	ClearScreen();         //Print out welcome message.
	PrintStr(pszStartMsg1);
	PrintStr(pszStartMsg2);
	ChangeLine();
	GotoHome();

	PrintStr(pszHelpInfo); //Print out help information.
	ChangeLine();
	GotoHome();

	g_keyHandler = SetKeyHandler(_KeyHandler);     //Set key board handler.

	*(__PDE*)PD_START = NULL_PDE;    //Set the first page directory entry to NULL,to indicate
	                                 //this location is not initialized yet.

*/
	/********************************************************************************
	*********************************************************************************
	*********************************************************************************
	*********************************************************************************
	*********************************************************************************/
	//
	//The following code initializes system level global objects.
	//
/*
#ifdef __ENABLE_VIRTUAL_MEMORY    //Should enable virtual memory model.

	lpVirtualMemoryMgr = (__VIRTUAL_MEMORY_MANAGER*)ObjectManager.CreateObject(&ObjectManager,
		NULL,
		OBJECT_TYPE_VIRTUAL_MEMORY_MANAGER);    //Create virtual memory manager object.
	if(NULL == lpVirtualMemoryMgr)    //Failed to create this object.
		goto __TERMINAL;

	if(!lpVirtualMemoryMgr->Initialize((__COMMON_OBJECT*)lpVirtualMemoryMgr))
		goto __TERMINAL;
#endif
*/

/*
	if(!KernelThreadManager.Initialize((__COMMON_OBJECT*)&KernelThreadManager))
		goto __TERMINAL;                                                     //Initialize 
	                                                                         //Kernel Thread
	                                                                         //Manager.

	if(!System.Initialize((__COMMON_OBJECT*)&System))                        //Initialize
		                                                                     //system object.
		goto __TERMINAL;


	if(!PageFrameManager.Initialize((__COMMON_OBJECT*)&PageFrameManager,
		(LPVOID)0x02000000,
		(LPVOID)0x09FFFFFF))
		goto __TERMINAL;

	if(!MemoryManager.Initialize((__COMMON_OBJECT*)&MemoryManager))
		goto __TERMINAL;

	if(!IOManager.Initialize((__COMMON_OBJECT*)&IOManager))                  //Initialize
	                                                                         //Input-Output
	                                                                         //Manager.
		goto __TERMINAL;

	if(!DeviceManager.Initialize(&DeviceManager))                            //Initialize the
		                                                                     //Device manager.
	    goto __TERMINAL;

	if(!StatCpuObject.Initialize(&StatCpuObject))  //Initialize CPU statistics object.
	{
		goto __TERMINAL;
	}

	//
	//Load drivers here.
	//All drivers embedded into the kernel will be loaded here,and the drivers
	//implemented as module will be loaded later.
	//
	dwIndex = 0;
	while(DriverEntryArray[dwIndex])
	{
		if(!IOManager.LoadDriver(DriverEntryArray[dwIndex])) //Failed to load.
		{
			sprintf(strInfo,"Failed to load the %dth driver.",dwIndex); //Show an error.
			PrintLine(strInfo);
		}
		dwIndex ++;  //Continue to load.
	}

	lpIdleThread = KernelThreadManager.CreateKernelThread(    //Create system idle thread.
		(__COMMON_OBJECT*)&KernelThreadManager,
		0L,
		KERNEL_THREAD_STATUS_READY,
		PRIORITY_LEVEL_LOWEST,                                //Lowest priority level.
		SystemIdle,
		(LPVOID)(&dwIdleCounter),
		NULL,
		"IDLE");

	if(NULL == lpIdleThread)
	{
		//PrintLine("Can not create idle kernel thread,please restart the system.");
		__ERROR_HANDLER(ERROR_LEVEL_FATAL,0L,NULL);
		goto __TERMINAL;
	}

	//Create statistics kernel thread.
	lpStatKernelThread = KernelThreadManager.CreateKernelThread(
		(__COMMON_OBJECT*)&KernelThreadManager,
		0L,
		KERNEL_THREAD_STATUS_READY,
		PRIORITY_LEVEL_HIGH,  //With high priority.
		StatThreadRoutine,
		NULL,
		NULL,
		"CPU STAT");

	if(NULL == lpStatKernelThread)
	{
		//PrintLine("Can not create idle kernel thread,please restart the system.");
		__ERROR_HANDLER(ERROR_LEVEL_FATAL,0L,NULL);
		goto __TERMINAL;
	}

	lpShellThread = KernelThreadManager.CreateKernelThread(   //Create shell thread.
		(__COMMON_OBJECT*)&KernelThreadManager,
		0L,
		KERNEL_THREAD_STATUS_READY,
		PRIORITY_LEVEL_NORMAL,
		SystemShell,
		NULL,
		NULL,
		"SHELL");

	if(NULL == lpShellThread)
	{
		//PrintLine("Can not create system shell thread,please restart the system.");
		__ERROR_HANDLER(ERROR_LEVEL_FATAL,0L,NULL);
		goto __TERMINAL;
	}
	g_lpShellThread = lpShellThread;     //Initialize the shell thread global variable.

	if(!DeviceInputManager.Initialize((__COMMON_OBJECT*)&DeviceInputManager,
		                              NULL,
		                              (__COMMON_OBJECT*)lpShellThread))
									    //Initialize the DeviceInputManager object.
	{
		__ERROR_HANDLER(ERROR_LEVEL_FATAL,0L,NULL);
		goto __TERMINAL;
	}
*/

/*
#ifdef __ENABLE_VIRTUAL_MEMORY
	//
	//Now,we enable the page mechanism.
	//
	__asm{
		push eax
		mov eax,PD_START
		mov cr3,eax
		mov eax,cr0
		or eax,0x80000000
		mov cr0,eax
		pop eax
	}
#endif
*/

/*
	//g_intHandler = SetTimerHandler(TimerHandler);  //Set the timer interrupt handler.
	SetTimerHandler(GeneralIntHandler);

	//g_pCurrentTask = &tcbShell;  //Will be deleted

	StrCpy("[system-view]",&HostName[0]);

	//EntryPoint();                //Create the shell process.

	//
	//The following kernal thread is created,and it is the
	//first kernal thread,as a operating system shell.
	//
*/

/*	dwKThreadID = CreateKThread(16384,
		KTHREAD_STATUS_READY,
		0L,
		ShellThreadWrap,
		NULL,
		NULL);
	if(0 == dwKThreadID)          //If can not create the shell thread.
		goto __TERMINAL;

	g_pShellCtrlBlock = GetKThreadControlBlock(dwKThreadID);  //Initialize the shell's
	                                                          //control block.*/
	//EnableInterrupt();

	//DeadLoop();
	//EntryPoint();
	//ScheduleKThread();            //Schedule the kernal thread.

	//The following code will never be executed if corrected.
//__TERMINAL:
	//ChangeLine();
	//GotoHome();
	//PrintStr("STOP : An error occured,please power off your computer and restart it.");
	//__ERROR_HANDLER(ERROR_LEVEL_FATAL,0L,"Initializing process failed!");
    
	//DeadLoop();
}
