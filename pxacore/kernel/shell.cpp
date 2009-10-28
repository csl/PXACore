//***********************************************************************/
//    Author                    : Garry
//    Original Date             : May,27 2004
//    Module Name               : shell.cpp
//    Module Funciton           : 
//                                This module countains shell procedures.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/
#ifndef __STDAFX_H__
#include "..\INCLUDE\STDAFX.H"
#endif

#ifndef __KAPI_H__
#include "..\INCLUDE\KAPI.H"
#endif

#ifndef __IOCTRL_S_H__
#include "..\INCLUDE\IOCTRL_S.H"
#endif

#ifndef __SYSD_S_H__
#include "..\INCLUDE\SYSD_S.H"
#endif

#ifndef __RT8139_H__
#include "..\INCLUDE\RT8139.H"
#endif

#ifndef __EXTCMD_H__
#include "..\EXTCMD.H"
#endif

//
//Global variables.
//
#define MAX_HOSTNAME_LEN  16
#define VERSION_INFO "Hello Taiwan [Version 1.500]"

BYTE               HostName[MAX_HOSTNAME_LEN] = {0};          //Host name.
__TASK_CTRL_BLOCK  tcbShell = {0,0,0,0,0,
                   0,0,0,0,0,0,0,0,0,0,0,
				   0,0,0,0,0,0,0,0,0,0,0,
				   0,0,0,0,0,0,0,0,0,0,0,
				   0,0,0,0,0,0,0,0,0,0xffffffff};        //The shell's task control block.
				   

__KTHREAD_CONTROL_BLOCK* g_pShellCtrlBlock   = NULL;     //The shell's kernal thread control
                                                         //block pointer.

__KERNEL_THREAD_OBJECT*  g_lpShellThread     = NULL;     //The system shell's kernel thread 
                                                         //object.

static BYTE        CmdBuffer[MAX_BUFFER_LEN] = {0};  //Command buffer.
static WORD        BufferPtr = 0;         //Buffer pointer,points to the first
                                          //free byte of the CmdBuffer.

//
//Global functions.
//

//
//The following function form the command parameter object link from the command
//line string.
//

__CMD_PARA_OBJ* FormParameterObj(LPSTR pszCmd)
{
	__CMD_PARA_OBJ*     pObjBuffer = NULL;    //Local variables.
	__CMD_PARA_OBJ*     pBasePtr   = NULL;
	__CMD_PARA_OBJ*     pTmpObj    = NULL;
	DWORD               dwCounter  = 0x0000;
	DWORD               index      = 0x0000;

	if(NULL == pszCmd)    //Parameter check.
		return NULL;

	pObjBuffer = (__CMD_PARA_OBJ*)KMemAlloc(4096,KMEM_SIZE_TYPE_4K);
	if(NULL == pObjBuffer)
		goto __TERMINAL;

	pBasePtr = pObjBuffer;
	MemZero(pBasePtr,4096);

	while(*pszCmd)
	{
		if(' ' == *pszCmd)
		{
			pszCmd ++;
			continue; 
		}                                 //Filter the space.
		
		if(('-' == *pszCmd) || ('/' == *pszCmd))
		{
			pszCmd ++;
			pObjBuffer->byFunctionLabel = *pszCmd;
			pszCmd ++;                    //Skip the function label byte.
			continue;
		}
		else
		{
			/*while((' ' != *pszCmd) && *pszCmd)  //To find the first parameter.
			{
				pszCmd ++;
			}
			if(!*pszCmd)
				break;
			while(' ' == *pszCmd)    //Filter the space.
				pszCmd ++;

			if(!*pszCmd)
				break;*/
			index = 0x0000;
			while(('-' != *pszCmd) && ('/' != *pszCmd) && *pszCmd)
			{
				while((' ' != *pszCmd) && (*pszCmd) && (dwCounter <= CMD_PARAMETER_LEN))
				{
					pObjBuffer->Parameter[index][dwCounter] = *pszCmd;
					pszCmd ++;
					dwCounter ++;
				}
				pObjBuffer->Parameter[index][dwCounter] = 0x00;  //Set the terminal flag.
				index ++;               //Ready to copy the next parameter to parameter object.
				dwCounter = 0;

				if(!*pszCmd)
					break;
				while(' ' != *pszCmd)
					pszCmd ++;          //Skip the no space characters if the parameter's length
				                        //is longer than the const CMD_PARAMETER_LEN.
				while(' ' == *pszCmd)
					pszCmd ++;          //Skip the space character.
			}

			pTmpObj = pObjBuffer;       //Update the current parameter object.
			pObjBuffer = (__CMD_PARA_OBJ*)NextParaAddr(pTmpObj,index);
			pTmpObj->byParameterNum = LOBYTE(LOWORD(index));
			if(!*pszCmd)
				break;
			pTmpObj->pNext = pObjBuffer;
		}
	}

__TERMINAL:
	return pBasePtr;
}

//
//The following routine releases the parameter object created by FormParameterObj routine.
//

VOID ReleaseParameterObj(__CMD_PARA_OBJ* lpParamObj)
{
	if(NULL == lpParamObj)  //Parameter check.
		return;

	KMemFree((LPVOID)lpParamObj,KMEM_SIZE_TYPE_4K,4096);  //Release the memory.
	return;
}

//
//Command handler predefinitions.
//

VOID VerHandler(LPSTR);          //Handles the version command.
VOID MemHandler(LPSTR);          //Handles the memory command.
VOID SysInfoHandler(LPSTR);      //Handles the sysinfo command.
VOID HlpHandler(LPSTR);
VOID DateHandler(LPSTR);
VOID TimeHandler(LPSTR);
VOID CpuHandler(LPSTR);
VOID SptHandler(LPSTR);
VOID ClsHandler(LPSTR);
VOID RunTimeHandler(LPSTR);
VOID TestHandler(LPSTR);
VOID UnTestHandler(LPSTR);
VOID MemViewHandler(LPSTR);
VOID SendMsgHandler(LPSTR);
VOID KtViewHandler(LPSTR);
VOID SysNameHandler(LPSTR);
VOID IoCtrlApp(LPSTR);
VOID SysDiagApp(LPSTR);

#define CMD_OBJ_NUM  18

__CMD_OBJ  CmdObj[CMD_OBJ_NUM] = {
	{"version"  ,    VerHandler},
	{"memory"   ,    MemHandler},
	{"sysinfo"  ,    SysInfoHandler},
	{"sysname"  ,    SysNameHandler},
	{"help"     ,    HlpHandler},
	{"date"     ,    DateHandler},
	{"time"     ,    TimeHandler},
	{"cpuinfo"  ,    CpuHandler},
	{"support"  ,    SptHandler},
	{"runtime"  ,    RunTimeHandler},
	{"test"     ,    TestHandler},
	{"untest"   ,    UnTestHandler},
	{"memview"  ,    MemViewHandler},
	{"sendmsg"  ,    SendMsgHandler},
	{"ktview"   ,    KtViewHandler},
	{"ioctrl"   ,    IoCtrlApp},
	{"sysdiag"  ,    SysDiagApp},
	{"cls"      ,    ClsHandler}
};

//
//Global Functions.
//

VOID HlpHandler(LPSTR)           //Command 'help' 's handler.
{
	LPSTR strHelpTitle   = "    The following command is availiable currently:";
	LPSTR strHelpVer     = "    version      : Print out the version information.";
	LPSTR strHelpMem     = "    memory       : Print out the memory layout.";
	LPSTR strHelpSysInfo = "    sysinfo      : Print out the system context.";
	LPSTR strSysName     = "    sysname      : Change the system host name.";
	LPSTR strHelpHelp    = "    help         : Print out this screen.";
	LPSTR strDate        = "    date         : Display or reset system date.";
	LPSTR strTime        = "    time         : Display or reset system time.";
	LPSTR strSupport     = "    support      : Print out technical support information.";
	LPSTR strRunTime     = "    runtime      : Display the total run time since last reboot.";
	LPSTR strMemView     = "    memview      : View a block memory's content.";
	LPSTR strSendMsg     = "    sendmsg      : Send a message to a kernal thread.";
	LPSTR strKtView      = "    ktview       : View all the kernal threads' information.";
	LPSTR strIoCtrlApp   = "    ioctrl       : Start IO control application.";
	LPSTR strSysDiagApp  = "    sysdiag      : System or hardware diag application.";
	LPSTR strCls         = "    cls          : Clear the whole display buffer.";

	PrintLine(strHelpTitle);              //Print out the help information line by line.
	PrintLine(strHelpVer);
	PrintLine(strHelpMem);
	PrintLine(strHelpSysInfo);
	PrintLine(strSysName);
	PrintLine(strHelpHelp);
	PrintLine(strDate);
	PrintLine(strTime);
	PrintLine(strSupport);
	PrintLine(strRunTime);
	PrintLine(strMemView);
	PrintLine(strSendMsg);
	PrintLine(strKtView);
	PrintLine(strIoCtrlApp);
	PrintLine(strSysDiagApp);
	PrintLine(strCls);
}

//
//sysname handler.
//This handler changes the system name,and save it to system config database.
//

static VOID SaveSysName(LPSTR)
{
}

VOID SysNameHandler(LPSTR pszSysName)
{
	__CMD_PARA_OBJ*    pCmdObj = NULL;

	pCmdObj = FormParameterObj(pszSysName);
	if(NULL == pCmdObj)
	{
		PrintLine("Not enough system resource to interpret the command.");
		goto __TERMINAL;
	}
	if((0 == pCmdObj->byParameterNum) || (0 == pCmdObj->Parameter[0][0]))
	{
		PrintLine("Invalid command parameter.");
		goto __TERMINAL;
	}

	if(StrLen(pCmdObj->Parameter[0]) >= MAX_HOSTNAME_LEN)
	{
		PrintLine("System name must not exceed 16 bytes.");
		goto __TERMINAL;
	}

	SaveSysName(pCmdObj->Parameter[0]);
	StrCpy(pCmdObj->Parameter[0],&HostName[0]);
__TERMINAL:
	if(NULL != pCmdObj)
		KMemFree((LPVOID)pCmdObj,KMEM_SIZE_TYPE_4K,4096);
	return;
}

//
//Local helper function.
//The function print out all of the kernal threads' information.
//
static LPSTR                      pszThreadID   = "    Thread ID     : ";
static LPSTR                      pszContext    = "    Context:";
static LPSTR                      pszEax        = "        EAX       : ";
static LPSTR                      pszEbx        = "        EBX       : ";
static LPSTR                      pszEcx        = "        ECX       : ";
static LPSTR                      pszEdx        = "        EDX       : ";
static LPSTR                      pszEsi        = "        ESI       : ";
static LPSTR                      pszEdi        = "        EDI       : ";
static LPSTR                      pszEbp        = "        EBP       : ";
static LPSTR                      pszEsp        = "        ESP       : ";
static LPSTR                      pszEFlags     = "        EFlags    : ";
static LPSTR                      pszStartAddr  = "    Start Address : ";
static LPSTR                      pszStackSize  = "    Stack Size    : ";
static LPSTR                      pszCurrMsgNum = "    Message num   : ";

static VOID PrintAllKt(__KTHREAD_CONTROL_BLOCK** ppControlBlock)
{
	BYTE                       Buffer[32];

	for(DWORD i = 0;i < MAX_KTHREAD_NUM;i ++)
	{
		if(NULL == ppControlBlock[i])
			continue;
		PrintLine(pszThreadID);
		Int2Str(ppControlBlock[i]->dwKThreadID,Buffer);
		PrintStr(Buffer);

		PrintLine(pszContext);
		PrintLine(pszEax);
		Hex2Str(ppControlBlock[i]->dwEAX,Buffer);
		PrintStr(Buffer);

		PrintLine(pszEbx);
		Hex2Str(ppControlBlock[i]->dwEBX,Buffer);
		PrintStr(Buffer);

		PrintLine(pszEcx);
		Hex2Str(ppControlBlock[i]->dwECX,Buffer);
		PrintStr(Buffer);

		PrintLine(pszEdx);
		Hex2Str(ppControlBlock[i]->dwEDX,Buffer);
		PrintStr(Buffer);

		PrintLine(pszEsi);
		Hex2Str(ppControlBlock[i]->dwESI,Buffer);
		PrintStr(Buffer);

		PrintLine(pszEdi);
		Hex2Str(ppControlBlock[i]->dwEDI,Buffer);
		PrintStr(Buffer);

		PrintLine(pszEbp);
		Hex2Str(ppControlBlock[i]->dwEBP,Buffer);
		PrintStr(Buffer);

		PrintLine(pszEsp);
		Hex2Str(ppControlBlock[i]->dwESP,Buffer);
		PrintStr(Buffer);

		PrintLine(pszEFlags);
		Hex2Str(ppControlBlock[i]->dwEFlags,Buffer);
		PrintStr(Buffer);

		PrintLine(pszStartAddr);
		Hex2Str((DWORD)ppControlBlock[i]->pKThreadRoutine,Buffer);
		PrintStr(Buffer);

		PrintLine(pszStackSize);
		Hex2Str(ppControlBlock[i]->dwStackSize,Buffer);
		PrintStr(Buffer);

		PrintLine(pszCurrMsgNum);
		Hex2Str(ppControlBlock[i]->wCurrentMsgNum,Buffer);
		PrintStr(Buffer);

		ChangeLine();
		GotoHome();
	}
}

static VOID KtViewUsage()
{
	PrintLine("    Usage :");
	PrintLine("    ktview -i [-?] kthread_id");
	PrintLine("    Where :");
	PrintLine("        -i         : View the kernal thread's information.");
	PrintLine("        kthread_id : Kernal thread's ID");
	PrintLine("        -?         : Print out the help information of the command.");
}

static VOID PrintKtByID(DWORD dwKThreadID)
{
	__KTHREAD_CONTROL_BLOCK* pControlBlock = NULL;
	DWORD dwIndex                          = 0L;
	BYTE  Buffer[12];

	if((dwKThreadID < 1) || (dwKThreadID > MAX_KTHREAD_NUM))
	{
		PrintLine("Invalid kernal thread ID");
		return;
	}

	dwIndex = dwKThreadID - 1;
	pControlBlock = g_pKThreadQueue[dwIndex];
	if(NULL == pControlBlock)
	{
		PrintLine("The kernal thread is not exist.");
		return;
	}
	PrintLine(pszThreadID);
	Int2Str(pControlBlock->dwKThreadID,Buffer);
	PrintStr(Buffer);

	PrintLine(pszContext);
	PrintLine(pszEax);
	Hex2Str(pControlBlock->dwEAX,Buffer);
	PrintStr(Buffer);

	PrintLine(pszEbx);
	Hex2Str(pControlBlock->dwEBX,Buffer);
	PrintStr(Buffer);

	PrintLine(pszEcx);
	Hex2Str(pControlBlock->dwECX,Buffer);
	PrintStr(Buffer);

	PrintLine(pszEdx);
	Hex2Str(pControlBlock->dwEDX,Buffer);
	PrintStr(Buffer);

	PrintLine(pszEsi);
	Hex2Str(pControlBlock->dwESI,Buffer);
	PrintStr(Buffer);

	PrintLine(pszEdi);
	Hex2Str(pControlBlock->dwEDI,Buffer);
	PrintStr(Buffer);

	PrintLine(pszEbp);
	Hex2Str(pControlBlock->dwEBP,Buffer);
	PrintStr(Buffer);

	PrintLine(pszEsp);
	Hex2Str(pControlBlock->dwESP,Buffer);
	PrintStr(Buffer);

	PrintLine(pszEFlags);
	Hex2Str(pControlBlock->dwEFlags,Buffer);
	PrintStr(Buffer);

	PrintLine(pszStartAddr);
	Hex2Str((DWORD)pControlBlock->pKThreadRoutine,Buffer);
	PrintStr(Buffer);

	PrintLine(pszStackSize);
	Hex2Str(pControlBlock->dwStackSize,Buffer);
	PrintStr(Buffer);

	PrintLine(pszCurrMsgNum);
	Hex2Str(pControlBlock->wCurrentMsgNum,Buffer);
	PrintStr(Buffer);

	ChangeLine();
	GotoHome();
}

VOID KtViewHandler(LPSTR pszPara)
{
	__CMD_PARA_OBJ*  pCmdObj = NULL;
	DWORD            dwID    = 0L;
	BOOL             bResult = FALSE;
	
	if((NULL == pszPara) || 0 == *pszPara)
	{
		PrintAllKt(&g_pKThreadQueue[0]);
		goto __TERMINAL;
	}

	pCmdObj = FormParameterObj(pszPara);
	if(NULL == pCmdObj)
	{
		PrintLine("Can not allocate the resource to interpret the command.");
		goto __TERMINAL;
	}

	switch(pCmdObj->byFunctionLabel)
	{
	case 'i':
	case 0:
		bResult = Str2Hex(pCmdObj->Parameter[0],&dwID);
		if(FALSE == bResult)
		{
			PrintLine("Can not interpret the command's parameter.");
			goto __TERMINAL;
		}
		PrintKtByID(dwID);
		break;
	case '?':
	default:
		KtViewUsage();
		break;
	}

__TERMINAL:
	if(NULL != pCmdObj)
		KMemFree((LPVOID)pCmdObj,KMEM_SIZE_TYPE_4K,4096);
	return;
}

//
//The helper functions,print out the usage information.
//

static VOID SendMsgUsage()
{
	PrintLine("    Usage :");
	PrintLine("      sendmsg kthread_id command [parameter1] [parameter2]");
	PrintLine("    Where :");
	PrintLine("      kthread_id  : Kernal thread ID.");
	PrintLine("      command     : Command number.");
	PrintLine("      parameter1  : The first parameter(optional).");
	PrintLine("      parameter2  : The second parameter(optional).");
}

VOID SendMsgHandler(LPSTR pszPara)
{
	__CMD_PARA_OBJ*          pCmdObj       = NULL;
	DWORD                    dwID          = 0L;
	__KTHREAD_MSG            msg;
	BOOL                     bResult       = FALSE;
	DWORD                    dwCommand     = 0L;
	__KTHREAD_CONTROL_BLOCK* pControlBlock = NULL;

	if((NULL == pszPara) || (0 == *pszPara))
	{
		SendMsgUsage();
		goto __TERMINAL;
	}

	pCmdObj = FormParameterObj(pszPara);
	if(NULL == pCmdObj)
	{
		PrintLine("Can not allocate resource to interpret the command.");
		goto __TERMINAL;
	}

	if(pCmdObj->byParameterNum < 2)
	{
		PrintLine("Miss command code.");
		goto __TERMINAL;
	}

	bResult = Str2Hex(pCmdObj->Parameter[0],&dwID);
	if((FALSE == bResult)
		|| (dwID < 1)
		|| (dwID > MAX_KTHREAD_NUM))
	{
		PrintLine("Invalid kernal thread ID.");
		goto __TERMINAL;
	}

	pControlBlock = GetKThreadControlBlock(dwID);
	if(NULL == pControlBlock)
	{
		PrintLine("The kernal thread is not exist.");
		goto __TERMINAL;
	}

	bResult = Str2Hex(pCmdObj->Parameter[1],&dwCommand);
	if(FALSE == bResult)
	{
		PrintLine("Invalid command code.");
		goto __TERMINAL;
	}

	switch(pCmdObj->byParameterNum)
	{
	case 2:                               //No parameter.
		msg.wCommand   = LOWORD(dwCommand);
		msg.dwParam_01 = 0L;
		msg.dwParam_02 = 0L;
		KtSendMessage(pControlBlock,&msg);
		break;
	case 3:                               //With one parameter.
		msg.wCommand   = LOWORD(dwCommand);
		bResult        = Str2Hex(pCmdObj->Parameter[2],&msg.dwParam_01);
		if(FALSE == bResult)
		{
			PrintLine("Invalid parameter.");
			break;
		}
		msg.dwParam_02 = 0L;
		KtSendMessage(pControlBlock,&msg);
		break;
	case 4:                               //With two parameter.
	default:
		msg.wCommand   = LOWORD(dwCommand);
		bResult        = Str2Hex(pCmdObj->Parameter[2],&msg.dwParam_01);
		if(FALSE == bResult)
		{
			PrintLine("Invalid parameter.");
			break;
		}
		bResult        = Str2Hex(pCmdObj->Parameter[3],&msg.dwParam_02);
		if(FALSE == bResult)
		{
			PrintLine("Invalid parameter.");
			break;
		}
		KtSendMessage(pControlBlock,&msg);
		break;
	}
__TERMINAL:
	if(NULL != pCmdObj)
		KMemFree((LPVOID)pCmdObj,KMEM_SIZE_TYPE_4K,4096);
	return;
}

//
//Memview handler.
//
VOID MemViewHandler(LPSTR pszCmd)
{
	__CMD_PARA_OBJ* pParaObj   = NULL;
	DWORD dwIndex              = 0x00000000;
	DWORD dwStartAddress       = 0x00000000;  //Local variables defination.
	DWORD bResult              = FALSE;
	DWORD dwNumber             = 0x00000000;
	DWORD i                    = 0x00000000;
	BYTE  memBuffer[12];

	if(0 == *pszCmd)
	{
__PRINT_USAGE:
		PrintLine("    Usage : View a block memory's content.");
		PrintLine("    memview -k[u] [process_id] start_mem_addr number");
		PrintLine("    Where :");
		PrintLine("      -k : View the kernal mode memory content.");
		PrintLine("      -u : View the user mode memory content.");
		PrintLine("      process_id : The process to be viewed.");
		PrintLine("      start_mem_addr : The start address of the memory be viewed.");
		PrintLine("      number : How many double word's memory to be viewed.");
		goto __TERMINAL;
	}

	pParaObj = FormParameterObj(pszCmd);    //Form the command parameter object by
	                                        //the parameter pszCmd.

	switch(pParaObj->byFunctionLabel)
	{
	case 0:                                 //If no function label,default to kernal memory
		                                    //view.
		//break;
	case 'k':
		bResult = Str2Hex(pParaObj->Parameter[0],&dwStartAddress);  //Convert tht first parameter
		                                                            //from string to hex number.
		if(FALSE == bResult)
		{
			PrintLine("Can not convert the first parameter to hex number.");
			break;
		}
		bResult = Str2Hex(pParaObj->Parameter[1],&dwNumber);  //Convert the second parameter
		                                                      //from string to hex number.
		if(FALSE == bResult)
		{
			PrintLine("Can not convert the second parameter to hex number.");
			break;
		}

		if(dwStartAddress + dwNumber * 4 > 16 * 1024 * 1024)
		{
			PrintLine("The memory block you want to view is exceed the kernal space.");
			break;
		}

		PrintLine("    ---------- ** Mem Content ** ----------    ");
		ChangeLine();
		GotoHome();
		for(i = 0;i < dwNumber;i ++)
		{
			PrintStr("        0x");
			Hex2Str(dwStartAddress,memBuffer);
			PrintStr(memBuffer);
			PrintStr("      0x");
			Hex2Str(*(DWORD*)dwStartAddress,memBuffer);
			PrintStr(memBuffer);
			ChangeLine();
			GotoHome();
			dwStartAddress += 4;
		}
		break;
	case 'u':
		PrintLine("Does not support now.");
		break;
	default:
		goto __PRINT_USAGE;
		break;
	}
	/*while(pParaObj)
	{
		for(dwIndex = 0;dwIndex < (DWORD)pParaObj->byParameterNum;dwIndex ++)
		{
			ConvertToUper(pParaObj->Parameter[dwIndex]);
			PrintLine(pParaObj->Parameter[dwIndex]);
		}
		pParaObj = pParaObj->pNext;
	}*/

__TERMINAL:
	if(NULL != pParaObj)
		KMemFree((LPVOID)pParaObj,KMEM_SIZE_TYPE_4K,4096);
	return;
}

//LPSTR g_pszTest = "Hello,Taiwan!!";

#define KMSG_USER_SUM 0x00FF
#define KMSG_USER_ACC 0x00FE

static __KTHREAD_CONTROL_BLOCK* g_pThread1ControlBlock = NULL;

VOID ShellThread1(LPVOID)
{
	static DWORD dwSum = 0L;
	__KTHREAD_CONTROL_BLOCK* pControlBlock = g_pThread1ControlBlock;
	__KTHREAD_MSG            msg;
	DWORD                    dwTmp1        = 0L;
	DWORD                    dwTmp2        = 0L;
	BYTE                     Buffer[32];

	while(TRUE)
	{
		if(KtGetMessage(pControlBlock,&msg))
		{
			switch(msg.wCommand)
			{
			case KMSG_USER_SUM:
				dwTmp1 = msg.dwParam_01;
				dwTmp2 = msg.dwParam_02;
				dwSum  = dwTmp1 + dwTmp2;
				Int2Str(dwSum,Buffer);
				PrintLine("The sum is : ");
				PrintStr(Buffer);
				break;
			case KMSG_USER_ACC:
				dwTmp1 = msg.dwParam_01;
				dwSum  = 0L;
				for(dwTmp2 = 0;dwTmp2 < dwTmp1;dwTmp2 ++)
				{
					dwSum += dwTmp2 + 1;
				}
				PrintLine("The accumulated result is : ");
				Int2Str(dwSum,Buffer);
				PrintStr(Buffer);
				break;
			default:
				break;
			}
		}
	}
}

VOID ShellThread2(LPVOID)
{
	static DWORD dwThread2Counter = 0x0000;
	while(TRUE)
	{
		dwThread2Counter ++;
		if(0 == dwThread2Counter % 0x00FFFFFF)
		{
			PrintLine("Kernal thread is running : kernal-thread-2.");
			dwThread2Counter = 0;
		}
	}
}

DWORD ShellThread3(LPVOID)
{
	DWORD dwCounter = 1L;
	while(dwCounter < 10)
	{
		dwCounter ++;
		PrintLine("Thread 3 is running,increment the dwCounter.");
	}
	return 0L;
}

__EVENT* lpWriteEvent  = NULL;
__EVENT* lpReadEvent   = NULL;

DWORD ReadThread(LPVOID)
{
	while(TRUE)
	{
		lpReadEvent->WaitForThisObject((__COMMON_OBJECT*)lpReadEvent);
		PrintLine("Read resource from shared pool.");
		lpReadEvent->ResetEvent((__COMMON_OBJECT*)lpReadEvent);
		lpWriteEvent->SetEvent((__COMMON_OBJECT*)lpWriteEvent);
	}
	return 0L;
}

DWORD WriteThread(LPVOID)
{
	while(TRUE)
	{
		lpWriteEvent->WaitForThisObject((__COMMON_OBJECT*)lpWriteEvent);
		PrintLine("Write resource to shared pool.");
		lpWriteEvent->ResetEvent((__COMMON_OBJECT*)lpWriteEvent);
		lpReadEvent->SetEvent((__COMMON_OBJECT*)lpReadEvent);
	}
	return 0L;
}

DWORD SleepThread(LPVOID)
{
	DWORD dwSleepCount = 10L;

	while(dwSleepCount)
	{
		PrintLine("I am sleeping thread.");
		KernelThreadManager.Sleep((__COMMON_OBJECT*)&KernelThreadManager,500);
		dwSleepCount --;
	}
	return 0L;
}

DWORD EchoRoutine(LPVOID)
{
	__KERNEL_THREAD_MESSAGE Msg;
	BYTE                    bt;
	WORD                    wr = 0x0700;
	while(TRUE)
	{
		if(GetMessage(&Msg))
		{
			if(Msg.wCommand == MSG_KEY_DOWN)
			{
				bt = LOBYTE(LOWORD(Msg.dwParam));
				if(('q' == bt) || ('x' == bt))
					goto __TERMINAL;
				wr += bt;
				PrintCh(wr);
				wr = 0x0700;
			}
		}
	}
__TERMINAL:
	return 0L;
}

__MUTEX*    lpMutex   = NULL;
__MAILBOX*  lpMailBox = NULL;
__EVENT*    lpEvent   = NULL;

static DWORD SynThread1(LPVOID)
{
	__KERNEL_THREAD_MESSAGE Msg;
	BYTE                    bt;

	while(TRUE)
	{
		if(GetMessage(&Msg))
		{
			if(Msg.wCommand == MSG_KEY_DOWN)
			{
				PrintLine("Set event object.");
				lpEvent->SetEvent((__COMMON_OBJECT*)lpEvent);
				bt = LOBYTE(LOWORD(Msg.dwParam));
				if('q' == bt)
					return 0L;
			}
		}
	}
	return 0L;
}

static DWORD SynThread2(LPVOID)
{
	DWORD           dwRetVal = 0L;

	while(TRUE)
	{
		dwRetVal = lpEvent->WaitForThisObjectEx((__COMMON_OBJECT*)lpEvent,
			300);
		switch(dwRetVal)
		{
		case OBJECT_WAIT_RESOURCE:
			PrintLine("I am first thread,wait a resource.");
			lpEvent->ResetEvent((__COMMON_OBJECT*)lpEvent);
			break;
		case OBJECT_WAIT_TIMEOUT:
			PrintLine("I am first thread,wait time out...");
			break;
		}
	}
	return 0L;
}

static DWORD SynThread3(LPVOID)
{	
	DWORD           dwRetVal = 0L;

	while(TRUE)
	{
		dwRetVal = lpEvent->WaitForThisObjectEx((__COMMON_OBJECT*)lpEvent,
			600);
		switch(dwRetVal)
		{
		case OBJECT_WAIT_RESOURCE:
			PrintLine("I am second thread,wait a resource.");
			lpEvent->ResetEvent((__COMMON_OBJECT*)lpEvent);
			break;
		case OBJECT_WAIT_TIMEOUT:
			PrintLine("I am second thread,wait time out...");
			break;
		}
	}
	return 0L;
}

static DWORD CriticalThread1(LPVOID)
{
	DWORD dwCounter = 0;
	while(TRUE)
	{
		dwCounter ++;
		if(0xFFFFFFFF == dwCounter)
		{
			//PrintLine("I am critical kernel thread 1.");
			dwCounter = 0;
			//break;
		}
	}
	return 0L;
}

static DWORD CriticalThread2(LPVOID)
{
	DWORD dwCounter = 0;
	while(TRUE)
	{
		dwCounter ++;
		if(0xFFFFFFFF == dwCounter)
		{
			//PrintLine("I am critical kernel thread 2.");
			dwCounter = 0;
			//break;
		}
	}
	return 0L;
}

static DWORD CriticalThread3(LPVOID)
{
	DWORD dwCounter = 0;
	while(TRUE)
	{
		dwCounter ++;
		if(0xFFFFFFFF == dwCounter)
		{
			//PrintLine("I am critical kernel thread 3.");
			dwCounter = 0;
			//break;
		}
	}
	return 0L;
}

static DWORD HighThread(LPVOID)
{
	DWORD dwCounter = 0;
	while(TRUE)
	{
		dwCounter ++;
		if(0xFFFFFFFF == dwCounter)
		{
			//PrintLine("I am high priority kernel thread.");
			dwCounter = 0;
			//break;
		}
	}
	return 0L;
}

static DWORD LazyPig(LPVOID)
{
	DWORD dwRice = 0;
	while(TRUE)
	{
		while(TRUE)
		{
			dwRice ++;
			if(0xFFFFFFFF == dwRice)
			{
				dwRice = 0;
				break;
			}
		}
		KernelThreadManager.Sleep((__COMMON_OBJECT*)&KernelThreadManager,1000);  //Sleep 1s.
	}
	return 0L;
}

VOID TestHandler(LPSTR)
{
	__KERNEL_THREAD_OBJECT*  lpCritical1 = NULL;
	__KERNEL_THREAD_OBJECT*  lpCritical2 = NULL;
	__KERNEL_THREAD_OBJECT*  lpCritical3 = NULL;
	__KERNEL_THREAD_OBJECT*  lpHigh      = NULL;

	lpCritical1 = KernelThreadManager.CreateKernelThread(
		(__COMMON_OBJECT*)&KernelThreadManager,
		0L,
		KERNEL_THREAD_STATUS_READY,
		PRIORITY_LEVEL_LOW,
		CriticalThread1,
		NULL,
		NULL,
		"Test thread1");

	lpCritical2 = KernelThreadManager.CreateKernelThread(
		(__COMMON_OBJECT*)&KernelThreadManager,
		0L,
		KERNEL_THREAD_STATUS_READY,
		PRIORITY_LEVEL_LOW,
		CriticalThread2,
		NULL,
		NULL,
		"Test thread2");

	lpCritical3 = KernelThreadManager.CreateKernelThread(
		(__COMMON_OBJECT*)&KernelThreadManager,
		0L,
		KERNEL_THREAD_STATUS_READY,
		PRIORITY_LEVEL_LOW,
		CriticalThread3,
		NULL,
		NULL,
		"Test thread3");

	lpHigh = KernelThreadManager.CreateKernelThread(
		(__COMMON_OBJECT*)&KernelThreadManager,
		0L,
		KERNEL_THREAD_STATUS_READY,
		PRIORITY_LEVEL_LOW,
		HighThread,
		NULL,
		NULL,
		"Test thread4");

	KernelThreadManager.CreateKernelThread(
		(__COMMON_OBJECT*)&KernelThreadManager,
		0L,
		KERNEL_THREAD_STATUS_READY,
		PRIORITY_LEVEL_NORMAL,
		LazyPig,
		NULL,
		NULL,
		"Lazy pig");

	/*
	lpCritical1->WaitForThisObject((__COMMON_OBJECT*)lpCritical1);
	lpCritical2->WaitForThisObject((__COMMON_OBJECT*)lpCritical2);
	lpCritical3->WaitForThisObject((__COMMON_OBJECT*)lpCritical3);
	lpHigh->WaitForThisObject((__COMMON_OBJECT*)lpHigh);

	KernelThreadManager.DestroyKernelThread((__COMMON_OBJECT*)&KernelThreadManager,
		(__COMMON_OBJECT*)lpCritical1);
	KernelThreadManager.DestroyKernelThread((__COMMON_OBJECT*)&KernelThreadManager,
		(__COMMON_OBJECT*)lpCritical2);
	KernelThreadManager.DestroyKernelThread((__COMMON_OBJECT*)&KernelThreadManager,
		(__COMMON_OBJECT*)lpCritical3);
	KernelThreadManager.DestroyKernelThread((__COMMON_OBJECT*)&KernelThreadManager,
		(__COMMON_OBJECT*)lpHigh);
		*/
}

VOID IoCtrlApp(LPSTR)
{
	__KERNEL_THREAD_OBJECT*    lpIoCtrlThread    = NULL;

	lpIoCtrlThread = KernelThreadManager.CreateKernelThread(
		(__COMMON_OBJECT*)&KernelThreadManager,
		0L,
		KERNEL_THREAD_STATUS_READY,
		PRIORITY_LEVEL_NORMAL,
		IoCtrlStart,
		NULL,
		NULL,
		"IO CTRL");
	if(NULL == lpIoCtrlThread)    //Can not create the IO control thread.
	{
		PrintLine("Can not create IO control thread.");
		return;
	}

	DeviceInputManager.SetFocusThread((__COMMON_OBJECT*)&DeviceInputManager,
		(__COMMON_OBJECT*)lpIoCtrlThread);    //Set the current focus to IO control
	                                          //application.

	lpIoCtrlThread->WaitForThisObject((__COMMON_OBJECT*)lpIoCtrlThread);  //Block the shell
	                                                                      //thread until
	                                                                      //the IO control
	                                                                      //application end.
	KernelThreadManager.DestroyKernelThread((__COMMON_OBJECT*)&KernelThreadManager,
		(__COMMON_OBJECT*)lpIoCtrlThread);  //Destroy the thread object.

}

//
//System diag application's shell start code.
//

VOID SysDiagApp(LPSTR)
{
	__KERNEL_THREAD_OBJECT*        lpSysDiagThread    = NULL;

	lpSysDiagThread = KernelThreadManager.CreateKernelThread(
		(__COMMON_OBJECT*)&KernelThreadManager,
		0L,
		KERNEL_THREAD_STATUS_READY,
		PRIORITY_LEVEL_NORMAL,
		SysDiagStart,
		NULL,
		NULL,
		"SYS DIAG");
	if(NULL == lpSysDiagThread)    //Can not create the kernel thread.
	{
		PrintLine("Can not start system diag application,please retry again.");
		return;
	}

	DeviceInputManager.SetFocusThread((__COMMON_OBJECT*)&DeviceInputManager,
		(__COMMON_OBJECT*)lpSysDiagThread);

	lpSysDiagThread->WaitForThisObject((__COMMON_OBJECT*)lpSysDiagThread);
	KernelThreadManager.DestroyKernelThread((__COMMON_OBJECT*)&KernelThreadManager,
		(__COMMON_OBJECT*)lpSysDiagThread);  //Destroy the kernel thread object.
}

static __MUTEX*  lpMutexObj = NULL;
static DWORD ProduceRoutine(LPVOID)
{
	DWORD dwCounter = 2;
	while(TRUE)
	{
		KernelThreadManager.Sleep((__COMMON_OBJECT*)&KernelThreadManager,
			2000);
		lpMutexObj->ReleaseMutex((__COMMON_OBJECT*)lpMutexObj);
		dwCounter --;
		if(dwCounter == 0)
		{
			break;
		}
	}
	PrintLine("Produce kernel thread run over.");
	DestroyMutex((__COMMON_OBJECT*)lpMutexObj);  //DestroyMutex.
	return 0L;
}

static DWORD ConsumerRoutine1(LPVOID)
{
	DWORD dwResult;
	while(TRUE)
	{
		dwResult = lpMutexObj->WaitForThisObjectEx((__COMMON_OBJECT*)lpMutexObj,
			500);
		switch(dwResult)
		{
		case OBJECT_WAIT_RESOURCE:
			PrintLine("Consumer1 --> OK,resource is available.");
			break;
		case OBJECT_WAIT_TIMEOUT:
			PrintLine("Consumer1 --> Waiting time out...");
			break;
		case OBJECT_WAIT_DELETED:
			PrintLine("Consumer1 --> Produce kernel thread is over,exit.");
			return 0L;
		default:
			BUG();
		}
		//lpEventObj->ResetEvent((__COMMON_OBJECT*)lpEventObj);  //Reset event.
	}
	return 0L;
}

static DWORD ConsumerRoutine2(LPVOID)
{
	DWORD dwResult;
	while(TRUE)
	{
		dwResult = lpMutexObj->WaitForThisObjectEx((__COMMON_OBJECT*)lpMutexObj,
			800);
		switch(dwResult)
		{
		case OBJECT_WAIT_RESOURCE:
			PrintLine("Consumer2 --> OK,resource is available.");
			break;
		case OBJECT_WAIT_TIMEOUT:
			PrintLine("Consumer2 --> Waiting time out...");
			break;
		case OBJECT_WAIT_DELETED:
			PrintLine("Consumer2 --> Produce kernel thread is over,exit.");
			return 0L;
		default:
			BUG();
		}
		//lpEventObj->ResetEvent((__COMMON_OBJECT*)lpEventObj);  //Reset event.
	}
	return 0L;
}

static DWORD ConsumerRoutine3(LPVOID)
{
	DWORD dwResult;
	while(TRUE)
	{
		dwResult = lpMutexObj->WaitForThisObjectEx((__COMMON_OBJECT*)lpMutexObj,
			1000);
		switch(dwResult)
		{
		case OBJECT_WAIT_RESOURCE:
			PrintLine("Consumer3 --> OK,resource is available.");
			break;
		case OBJECT_WAIT_TIMEOUT:
			PrintLine("Consumer3 --> Waiting time out...");
			break;
		case OBJECT_WAIT_DELETED:
			PrintLine("Consumer3 --> Produce kernel thread is over,exit.");
			return 0L;
		default:
			BUG();
		}
		//lpEventObj->ResetEvent((__COMMON_OBJECT*)lpEventObj);  //Reset event.
	}
	return 0L;
}

VOID UnTestHandler(LPSTR)
{
	__KERNEL_THREAD_OBJECT*    lpProduce;
	__KERNEL_THREAD_OBJECT*    lpConsumer1;
	__KERNEL_THREAD_OBJECT*    lpConsumer2;
	__KERNEL_THREAD_OBJECT*    lpConsumer3;

	lpMutexObj = (__MUTEX*)CreateMutex();  //Create event object.
	if(NULL == lpMutexObj)
	{
		PrintLine("Can not create Mutex object.");
		return;
	}
	
	lpProduce = KernelThreadManager.CreateKernelThread((__COMMON_OBJECT*)&KernelThreadManager,
		0L,
		KERNEL_THREAD_STATUS_READY,
		PRIORITY_LEVEL_NORMAL,
		ProduceRoutine,
		NULL,
		NULL,
		"Produce");
	if(NULL == lpProduce)
	{
		PrintLine("Can not create produce kernel thread.");
		return;
	}

	lpConsumer1 = KernelThreadManager.CreateKernelThread((__COMMON_OBJECT*)&KernelThreadManager,
		0L,
		KERNEL_THREAD_STATUS_READY,
		PRIORITY_LEVEL_NORMAL,
		ConsumerRoutine1,
		NULL,
		NULL,
		"CONS1");
	if(NULL == lpConsumer1)
	{
		PrintLine("Can not create consumer1 kernel thread.");
		return;
	}

	lpConsumer2 = KernelThreadManager.CreateKernelThread((__COMMON_OBJECT*)&KernelThreadManager,
		0L,
		KERNEL_THREAD_STATUS_READY,
		PRIORITY_LEVEL_NORMAL,
		ConsumerRoutine2,
		NULL,
		NULL,
		"CONS2");
	if(NULL == lpConsumer2)
	{
		PrintLine("Can not create consumer2 kernel thread.");
		return;
	}

	lpConsumer3 = KernelThreadManager.CreateKernelThread((__COMMON_OBJECT*)&KernelThreadManager,
		0L,
		KERNEL_THREAD_STATUS_READY,
		PRIORITY_LEVEL_NORMAL,
		ConsumerRoutine3,
		NULL,
		NULL,
		"CONS3");
	if(NULL == lpConsumer3)
	{
		PrintLine("Can not create consumer3 kernel thread.");
		return;
	}

	lpProduce->WaitForThisObject((__COMMON_OBJECT*)lpProduce);
	lpConsumer1->WaitForThisObject((__COMMON_OBJECT*)lpConsumer1);
	lpConsumer2->WaitForThisObject((__COMMON_OBJECT*)lpConsumer2);
	lpConsumer3->WaitForThisObject((__COMMON_OBJECT*)lpConsumer3);
}

VOID RunTimeHandler(LPSTR)
{
	DWORD dwTime = System.GetClockTickCounter((__COMMON_OBJECT*)&System);
	BYTE  Buffer[12];
	dwTime /= SYSTEM_TIME_SLICE;
	PrintLine("The system has running ");
	Int2Str(dwTime,Buffer);
	PrintStr(Buffer);
	PrintStr(" second(s).");
}

VOID ClsHandler(LPSTR)
{
	ClearScreen();
}

VOID VerHandler(LPSTR)
{
	GotoHome();
	ChangeLine();
	PrintStr(VERSION_INFO);
}

VOID MemHandler(LPSTR)
{
	PrintLine("------------------ ** memory layout ** ------------------");
	PrintLine("    0x00000000 - 0x000FFFFF        Hardware buffer       ");
	PrintLine("    0x00100000 - 0x0010FFFF        Mini-kernal           ");
	PrintLine("    0x00110000 - 0x013FFFFF        Master(OS Kernal)     ");
	PrintLine("    0x01400000 - 0xFFFFFFFF        User Space            ");
}


LPSTR strHdr[] = {               //I have put the defination of this strings
	                             //in the function SysInfoHandler,but it do
	                             //not work,see the asm code,it generates the
	                             //incorrect asm code!Fuck Bill Gates!.
	"    EDI   :   0x",
	"    ESI   :   0x",
	"    EBP   :   0x",
	"    ESP   :   0x",
	"    EBX   :   0x",
	"    EDX   :   0x",
	"    ECX   :   0x",
	"    EAX   :   0x",
	"    CS-DS :   0x",
	"    FS-GS :   0x",
	"    ES-SS :   0x"};

static BYTE Buffer[] = {"Hello,Taiwan!"};

VOID SysInfoHandler(LPSTR)
{
	DWORD sysContext[11];

#ifdef __I386__
	__asm{                       //Get the system information.
		pushad                   //Save all the general registers.
			                     //NOTICE: This operation only get
								 //the current status of system
								 //where this instruction is executed.
        push eax
        mov eax,dword ptr [esp + 0x04]
		mov dword ptr [ebp - 0x2c],eax    //Get the eax register's value.
		                                  //Fuck Bill Gates!!!!!
		mov eax,dword ptr [esp + 0x08]
		mov dword ptr [ebp - 0x28],eax    //Get the ecx value.
		mov eax,dword ptr [esp + 0x0c]
		mov dword ptr [ebp - 0x24],eax    //edx
		mov eax,dword ptr [esp + 0x10]
		mov dword ptr [ebp - 0x20],eax    //ebx
		mov eax,dword ptr [esp + 0x14]
		mov dword ptr [ebp - 0x1c],eax    //esp
		mov eax,dword ptr [esp + 0x18]
		mov dword ptr [ebp - 0x18],eax    //ebp
		mov eax,dword ptr [esp + 0x1c]
		mov dword ptr [ebp - 0x14],eax    //esi
		mov eax,dword ptr [esp + 0x20]
		mov dword ptr [ebp - 0x10],eax    //edi

		mov ax,cs
		shl eax,0x10
		mov ax,ds
		mov dword ptr [ebp - 0x0c],eax    //Get cs : ds.
		mov ax,fs
		shl eax,0x10
		mov ax,gs
		mov dword ptr [ebp - 0x08],eax    //Get fs : gs.
		mov ax,es
		shl eax,0x10
		mov ax,ss
		mov dword ptr [ebp - 0x04],eax   //Get es : ss.

		pop eax
		popad                    //Restore the stack frame.
	}
#else                            //If not an I386 or above platform.
	GotoHome();
	ChangeLine();
	PrintStr("    This operation can not supported on no-I386 platform.");
	return;
#endif                           //Now,we got the general registers and segment
	                             //registers' value,print them out.
	GotoHome();
	ChangeLine();
	PrintStr("    System context information(general registers and segment registers):");
	for(DWORD bt = 0;bt < 11;bt ++)
	{
		GotoHome();
		ChangeLine();
		PrintStr(strHdr[bt]);
		Hex2Str(sysContext[bt],Buffer);
		//Buffer[8] = 0x00;
		PrintStr(Buffer);
	}
	return;
}

VOID DateHandler(LPSTR)
{
	GotoHome();
	ChangeLine();
	PrintStr("DateHandler called.");
}

VOID TimeHandler(LPSTR)
{
	GotoHome();
	ChangeLine();
	PrintStr("Time Handler called.");
}

VOID CpuHandler(LPSTR)
{
	GotoHome();
	ChangeLine();
	PrintStr("Cpu Handler called.");
}

VOID SptHandler(LPSTR)           //Command 'support' handler.
{
	LPSTR strSupportInfo1 = "    For any technical support,send E-Mail to:";
	LPSTR strSupportInfo2 = "    garryxin@yahoo.com.cn.";
	
	GotoHome();
	ChangeLine();
	PrintStr(strSupportInfo1);
	GotoHome();
	ChangeLine();
	PrintStr(strSupportInfo2);
	return;
}

VOID  DefaultHandler(LPSTR)      //Default command handler.
{
	LPSTR strPrompt = "You entered incorrect command name.";
	ChangeLine();
	GotoHome();
	PrintStr(strPrompt);
	return;
}

//static BYTE tmpBuffer[36] = {0x00000000};   //If I declare this variable as the local
                                            //variable,as following,it would not work,
                                            //but I put it here,it works,I don't know
                                            //why,maybe some rules I do not know,but I
                                            //think it's would be gate's reason,so I
                                            //want to fuck bill gates again!!!
VOID  DoCommand()
{
	DWORD wIndex = 0x0000;
	BOOL bResult = FALSE;        //If find the correct command object,then
	                             //This flag set to TRUE.
	BYTE tmpBuffer[36];
	__KERNEL_THREAD_OBJECT* hKernelThread = NULL;

	CmdBuffer[BufferPtr] = 0x00; //Prepare the command string.
	BufferPtr = 0;

	while((' ' != CmdBuffer[wIndex]) && CmdBuffer[wIndex] && (wIndex < 32))
	{
		tmpBuffer[wIndex] = CmdBuffer[wIndex];
		wIndex ++;
	}
	tmpBuffer[wIndex] = 0;

	for(DWORD dwIndex = 0;dwIndex < CMD_OBJ_NUM;dwIndex ++)
	{
		if(StrCmp(&tmpBuffer[0],CmdObj[dwIndex].CmdStr))
		{
			CmdObj[dwIndex].CmdHandler(&CmdBuffer[wIndex]);  //Call the command handler.
			bResult = TRUE;      //Set the flag.
			break;
		}
	}
	if(bResult)
		goto __END;

	dwIndex = 0;  //Now,should search external command array.

	while(ExtCmdArray[dwIndex].lpszCmdName)
	{
		if(StrCmp(&tmpBuffer[0],ExtCmdArray[dwIndex].lpszCmdName))  //Found.
		{
			hKernelThread = KernelThreadManager.CreateKernelThread(
				(__COMMON_OBJECT*)&KernelThreadManager,
				0L,
				KERNEL_THREAD_STATUS_READY,
				PRIORITY_LEVEL_NORMAL,
				ExtCmdArray[dwIndex].ExtCmdHandler,
				(LPVOID)&CmdBuffer[wIndex],
				NULL,
				NULL);
			if(!ExtCmdArray[dwIndex].bBackground)  //Should wait.
			{
				hKernelThread->WaitForThisObject((__COMMON_OBJECT*)hKernelThread);
				KernelThreadManager.DestroyKernelThread(
					(__COMMON_OBJECT*)&KernelThreadManager,
					(__COMMON_OBJECT*)hKernelThread);  //Destroy it.
			}
			bResult = TRUE;
			goto __END;
		}
		dwIndex ++;
	}

	if(!bResult)
	{
		DefaultHandler(NULL);        //Call the default command handler.
	}

__END:
	return;
}

VOID  PrintPrompt()
{
	LPSTR pszSysName = "[system-view]";
	if(HostName[0])
	{
		PrintLine(&HostName[0]);
	}
	else
	{
		ChangeLine();
	    GotoHome();
	    PrintStr(pszSysName);
	}
	return;
}


BOOL EventHandler(WORD wCommand,WORD wParam,DWORD dwParam)
{
	WORD wr = 0x0700;
	BYTE bt = 0x00;
	BYTE Buffer[12];

	switch(wCommand)
	{
	case MSG_KEY_DOWN:
		bt = LOBYTE(LOWORD(dwParam));
		if(VK_RETURN == bt)
		{
			if(BufferPtr)
				DoCommand();
			PrintPrompt();
			break;
		}
		if(VK_BACKSPACE == bt)
		{
			if(0 != BufferPtr)
			{
				GotoPrev();
				BufferPtr --;
			}
			break;
		}
		else
		{
			if(MAX_BUFFER_LEN - 1 > BufferPtr)
			{
				CmdBuffer[BufferPtr] = bt;
				BufferPtr ++;
				wr += LOBYTE(LOWORD(dwParam));
				PrintCh(wr);
			}
		}
		break;
	case KERNEL_MESSAGE_TIMER:
		switch(dwParam){
		case 100:
			GotoHome();
			ChangeLine();
			PrintStr("Timer ID = 100,please handle it.");
			Int2Str(100,Buffer);
			PrintStr(Buffer);
			break;
		case 200:
			GotoHome();
			ChangeLine();
			PrintStr("Timer ID = 200,please handle it.");
			Int2Str(200,Buffer);
			PrintStr(Buffer);
			break;
		case 300:
			GotoHome();
			ChangeLine();
			PrintStr("Timer ID = 300,please handle it.");
			break;
		case 400:
			GotoHome();
			ChangeLine();
			PrintStr("Timer ID = 400,please handle it.");
			break;
		case 500:
			GotoHome();
			ChangeLine();
			PrintStr("Timer ID = 500,please handle it.");
			break;
		default:
			break;
		}
	default:
		break;
	}
	return 0L;
}

//
//Entry point.
//
DWORD EntryPoint()
{
	//__KTHREAD_MSG Msg;
	__KERNEL_THREAD_MESSAGE KernelThreadMessage;

	PrintPrompt();

	while(TRUE)
	{
		if(GetMessage(&KernelThreadMessage))
		{
			if(KTMSG_THREAD_TERMINAL == KernelThreadMessage.wCommand)
				goto __TERMINAL;
			DispatchMessage(&KernelThreadMessage,EventHandler);
		}
	}

__TERMINAL:                      //Terminal code here.

	return 0L;
}
