//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jul,03 2005
//    Module Name               : SYSD_S.CPP
//    Module Funciton           : 
//                                This module countains system diag application's implemen-
//                                tation code.
//                                This file countains the shell command or application
//                                code,as it's name has ended by "_S".
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

#ifndef __SYSD_S_H__
#include "..\INCLUDE\SYSD_S.H"
#endif

#ifndef __PCI_DRV_H__
#include "..\INCLUDE\PCI_DRV.H"
#endif

#include "..\INCLUDE\STAT_S.H"

//
//Pre-declare routines.
//
static DWORD CommandParser(LPSTR);
static DWORD memcheck(__CMD_PARA_OBJ*);
static DWORD cintperf(__CMD_PARA_OBJ*);
static DWORD exit(__CMD_PARA_OBJ*);
static DWORD help(__CMD_PARA_OBJ*);
static DWORD beep(__CMD_PARA_OBJ*);
static DWORD overload(__CMD_PARA_OBJ*);
static DWORD pcilist(__CMD_PARA_OBJ*);
static DWORD devinfo(__CMD_PARA_OBJ*);
static DWORD cpuload(__CMD_PARA_OBJ*);
static DWORD devlist(__CMD_PARA_OBJ*);

//
//The following is a map between command and it's handler.
//
static struct __SYS_DIAG_CMD_MAP{
	LPSTR                lpszCommand;
	DWORD                (*CommandHandler)(__CMD_PARA_OBJ*);
	LPSTR                lpszHelpInfo;
}SysDiagCmdMap[] = {
	{"memcheck",          memcheck,         "  memcheck             : Check the consistant of the physical memory."},
	{"beep",              beep,             "  beep                 : Beep a while to test the sound system."},
	{"overload",          overload,         "  overload             : Set a overload event to test the cint performance."},
	{"cintperf",          cintperf,         "  cintperf             : Print out the last system clock interrupt's CPU clock."},
	{"pcilist",           pcilist,          "  pcilist              : List all PCI device(s) of the system."},
	{"devinfo",           devinfo,          "  devinfo              : Print out information about a PCI device."},
	{"cpuload",           cpuload,          "  cpuload              : Display CPU statistics information."},
	{"devlist",           devlist,          "  devlist              : List all devices' information in the system."},
	{"exit",              exit,             "  exit                 : Exit the application."},
	{"help",              help,             "  help                 : Print out this screen."},
	{NULL,				  NULL,             NULL}
};

//
//The following is a helper routine,it only prints out a "#" character as prompt.
//

static VOID PrintPound()
{
	WORD  wr = 0x0700;
	
	wr += '#';
	ChangeLine();
	GotoHome();
	PrintCh(wr);
}

//
//This is the application's entry point.
//

DWORD SysDiagStart(LPVOID)
{
	BYTE                        strCmdBuffer[MAX_BUFFER_LEN];
	BYTE                        ucCurrentPtr                  = 0;
	BYTE                        bt;
	WORD                        wr                            = 0x0700;
	__KERNEL_THREAD_MESSAGE     Msg;
	DWORD                       dwRetVal;

	PrintPound();    //Print out the prompt.

	while(TRUE)
	{
		if(GetMessage(&Msg))
		{
			if(MSG_KEY_DOWN == Msg.wCommand)    //This is a key down message.
			{
				bt = LOBYTE(LOWORD(Msg.dwParam));
				switch(bt)
				{
				case VK_RETURN:                //This is a return key.
					if(0 == ucCurrentPtr)      //There is not any character before this key.
					{
						PrintPound();
						break;
					}
					else
					{
						strCmdBuffer[ucCurrentPtr] = 0;    //Set the terminal flag.
						dwRetVal = CommandParser(strCmdBuffer);
						switch(dwRetVal)
						{
						case SYS_DIAG_CMD_PARSER_TERMINAL: //Exit command is entered.
							goto __TERMINAL;
						case SYS_DIAG_CMD_PARSER_INVALID:  //Can not parse the command.
							PrintLine("    Invalid command.");
							//PrintPound();
							break;
						case SYS_DIAG_CMD_PARSER_FAILED:
							PrintLine("Failed to process the command.");
							break;
						case SYS_DIAG_CMD_PARSER_SUCCESS:      //Process the command successfully.
							//PrintPound();
							break;
						default:
							break;
						}
						ucCurrentPtr = 0;    //Re-initialize the buffer pointer.
						PrintPound();
					}
					break;
				case VK_BACKSPACE:
					if(ucCurrentPtr)
					{
						ucCurrentPtr --;
						GotoPrev();
					}
					break;
				default:
					if(ucCurrentPtr < MAX_BUFFER_LEN)    //The command buffer is not overflow.
					{
						strCmdBuffer[ucCurrentPtr] = bt;
						ucCurrentPtr ++;
						wr += bt;
						PrintCh(wr);
						wr  = 0x0700;
					}
					break;
				}
			}
			else
			{
				if(Msg.wCommand == KERNEL_MESSAGE_TIMER)
				{
					PrintLine("Timer message received.");
				}
			}
		}
	}

__TERMINAL:
	return 0L;
}

//
//The following routine processes the input command string.
//It is called by SysDiagStart.
//

static DWORD CommandParser(LPSTR lpszCmdLine)
{
	DWORD                  dwRetVal          = SYS_DIAG_CMD_PARSER_INVALID;
	DWORD                  dwIndex           = 0L;
	__CMD_PARA_OBJ*        lpCmdParamObj     = NULL;

	if((NULL == lpszCmdLine) || (0 == lpszCmdLine[0]))    //Parameter check
		return SYS_DIAG_CMD_PARSER_INVALID;

	lpCmdParamObj = FormParameterObj(lpszCmdLine);
	if(NULL == lpCmdParamObj)    //Can not form a valid command parameter object.
	{
		return SYS_DIAG_CMD_PARSER_FAILED;
	}

	if(0 == lpCmdParamObj->byParameterNum)  //There is not any parameter.
	{
		return SYS_DIAG_CMD_PARSER_FAILED;
	}

	//
	//The following code looks up the command map,to find the correct handler that handle
	//the current command.If find,then calls the handler,else,return SYS_DIAG_CMD_PARSER_INVALID
	//to indicate the failure.
	//
	while(TRUE)
	{
		if(NULL == SysDiagCmdMap[dwIndex].lpszCommand)
		{
			dwRetVal = SYS_DIAG_CMD_PARSER_INVALID;
			break;
		}
		if(StrCmp(SysDiagCmdMap[dwIndex].lpszCommand,lpCmdParamObj->Parameter[0]))  //Find the handler.
		{
			dwRetVal = SysDiagCmdMap[dwIndex].CommandHandler(lpCmdParamObj);
			break;
		}
		else
		{
			dwIndex ++;
		}
	}

//__TERMINAL:
	if(NULL != lpCmdParamObj)
		ReleaseParameterObj(lpCmdParamObj);

	return dwRetVal;
}

//
//The exit command's handler.
//

static DWORD exit(__CMD_PARA_OBJ* lpCmdObj)
{
	return SYS_DIAG_CMD_PARSER_TERMINAL;
}

//
//The memcheck command's handler.
//

static DWORD memcheck(__CMD_PARA_OBJ* lpCmdObj)
{
	return SYS_DIAG_CMD_PARSER_SUCCESS;
}

//
//The help command's handler.
//

static DWORD help(__CMD_PARA_OBJ* lpCmdObj)
{
	DWORD               dwIndex = 0L;

	while(TRUE)
	{
		if(NULL == SysDiagCmdMap[dwIndex].lpszHelpInfo)
			break;

		PrintLine(SysDiagCmdMap[dwIndex].lpszHelpInfo);
		dwIndex ++;
	}
	return SYS_DIAG_CMD_PARSER_SUCCESS;
}

//
//The cintperf command's handler.
//

static DWORD cintperf(__CMD_PARA_OBJ* lpCmdObj)
{
	BYTE              strBuffer[18];
	DWORD             dwFlags;
	__PERF_RECORDER   Pr;

	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	Pr = TimerIntPr;
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

	u64Hex2Str(&Pr.u64Result,strBuffer);
	PrintLine("Last clock circle counter : ");
	PrintLine(strBuffer);
	u64Hex2Str(&Pr.u64Max,strBuffer);
	PrintLine("Max clock circle counter : ");
	PrintLine(strBuffer);

	return SYS_DIAG_CMD_PARSER_SUCCESS;
}

//
//The overload command's handler.
//

static DWORD overload(__CMD_PARA_OBJ* lpCmdObj)
{
	DWORD            dwTimerNum       = 10;
	DWORD            dwTimerID        = 00200000;
	DWORD            dwTimeSpan       = 100;
	__COMMON_OBJECT* lpTimerObject    = NULL;

	if((NULL == lpCmdObj) || (lpCmdObj->byParameterNum < 2))  //Parameter check.
	{
		PrintLine("  Please input the beep time,in millionsecond.");
		return SYS_DIAG_CMD_PARSER_INVALID;
	}

	if(!Str2Hex(lpCmdObj->Parameter[1],&dwTimerNum))  //Get the time span to beep.
	{
		PrintLine("  Invalid time parameter.");
		return SYS_DIAG_CMD_PARSER_INVALID;
	}

	while(dwTimerNum)
	{
		lpTimerObject = System.SetTimer((__COMMON_OBJECT*)&System,
			KernelThreadManager.lpCurrentKernelThread,
			dwTimerID,
			dwTimeSpan,
			NULL,
			NULL,
			TIMER_FLAGS_ONCE);
		if(NULL == lpTimerObject)    //Failed to set timer.
		{
			PrintLine("  Can not set timer,please try again.");
			return SYS_DIAG_CMD_PARSER_FAILED;
		}
		dwTimerNum --;
	}

	return SYS_DIAG_CMD_PARSER_SUCCESS;
}

//
//The beep command's handler.
//

static DWORD beep(__CMD_PARA_OBJ* lpCmdObj)
{
	__KERNEL_THREAD_MESSAGE       Msg;
	DWORD                         dwTimerID      = 00100000;
	DWORD                         dwTimeSpan     = 0L;
	__COMMON_OBJECT*              lpTimerObject  = NULL;
	UCHAR                         ucCtrlByte;

	if((NULL == lpCmdObj) || (lpCmdObj->byParameterNum < 2))  //Parameter check.
	{
		PrintLine("  Please input the beep time,in millionsecond.");
		return SYS_DIAG_CMD_PARSER_INVALID;
	}

	if(!Str2Hex(lpCmdObj->Parameter[1],&dwTimeSpan))  //Get the time span to beep.
	{
		PrintLine("  Invalid time parameter.");
		return SYS_DIAG_CMD_PARSER_INVALID;
	}

	//
	//Now,the variable dwTimeSpan countains the time to beep.
	//
	lpTimerObject = System.SetTimer((__COMMON_OBJECT*)&System,
		KernelThreadManager.lpCurrentKernelThread,
		dwTimerID,
		dwTimeSpan,
		NULL,
		NULL,
		TIMER_FLAGS_ONCE);
	if(NULL == lpTimerObject)    //Failed to set timer.
	{
		PrintLine("  Can not set timer,please try again.");
		return SYS_DIAG_CMD_PARSER_FAILED;
	}

	//
	//Begin to beep.
	//
	ReadByteFromPort(&ucCtrlByte,0x61);
	ucCtrlByte |= 0x03;
	WriteByteToPort(ucCtrlByte,0x61);
	
	while(TRUE)
	{
		if(GetMessage(&Msg))
		{
			if((Msg.wCommand == KERNEL_MESSAGE_TIMER) &&   //Beep time is over.
			   (Msg.dwParam  == dwTimerID))
			{
				ucCtrlByte &= ~0x03;
				WriteByteToPort(ucCtrlByte,0x61);  //Stop the beep.
				PrintLine("  Beep over");
				goto __TERMINAL;
			}
		}
	}

__TERMINAL:
	return SYS_DIAG_CMD_PARSER_SUCCESS;
}

//
//The implementation of pcilist.
//This routine is the handler of 'pcilist' command.
//

static VOID  OutputDevInfo(__PHYSICAL_DEVICE* lpPhyDev);  //Helper routine.

static DWORD pcilist(__CMD_PARA_OBJ* lpParamObj)
{
	DWORD                  dwLoop          = 0L;
	__PHYSICAL_DEVICE*     lpPhyDev        = NULL;

	PrintLine("    Device ID/Vendor ID    Bus Number    Description");

	for(dwLoop = 0;dwLoop < MAX_BUS_NUM;dwLoop ++)
	{
		if(DeviceManager.SystemBus[dwLoop].dwBusType == BUS_TYPE_PCI)
		{
			lpPhyDev = DeviceManager.SystemBus[dwLoop].lpDevListHdr;
			while(lpPhyDev)  //Travel all list
			{
				OutputDevInfo(lpPhyDev);
				lpPhyDev = lpPhyDev->lpNext;
			}
		}
	}
	return SYS_DIAG_CMD_PARSER_SUCCESS;
}

static struct __PCI_DEV_DESC{
	WORD         wClasscode;
	LPSTR        lpszDesc;
}PciDevDesc[] = {
	{0x0000,     "Old devices(no-PCI device)."},
	{0x0001,     "VGA-Compatible devices."},

	{0x0100,     "SCSI Bus controller."},
	{0x0101,     "IDE controller."},
	{0x0102,     "Floppy disk controller."},
	{0x0103,     "IPI Bus controller."},
	{0x0104,     "RAID controller."},
	{0x0180,     "Mass storage controller."},

	{0x0200,     "Ethernet controller."},
	{0x0201,     "Token ring controller."},
	{0x0202,     "FDDI controller."},
	{0x0203,     "ATM controller."},
	{0x0204,     "ISDN controller."},
	{0x0280,     "Other network controller."},

	{0x0300,     "VGA controller."},
	{0x0301,     "XGA controller."},
	{0x0302,     "3D controller."},
	{0x0380,     "Other display controller."},

	{0x0400,     "Video device."},
	{0x0401,     "Audio device."},
	{0x0402,     "Computer telephony device."},
	{0x0480,     "Other multimedia device."},

	{0x0500,     "RAM device."},
	{0x0501,     "Flash device."},
	{0x0580,     "Other memory controller."},

	{0x0600,     "PCI-Host bridge."},
	{0x0601,     "ISA bridge."},
	{0x0602,     "EISA bridge."},
	{0x0603,     "MAC bridge."},
	{0x0604,     "PCI-PCI bridge."},
	{0x0605,     "PCMCIA bridge."},
	{0x0606,     "NuBus bridge."},
	{0x0607,     "CardBus bridge."},
	{0x0608,     "RACEway bridge."},
	{0x0680,     "Other bridge device."},

	{0x0700,     "Serial controller."},
	{0x0701,     "Parallel port."},
	{0x0702,     "Multiport serial controller."},
	{0x0703,     "Generic modem."},
	{0x0780,     "Other communications device."},

	{0x0800,     "Programmable Interrupt Controller."},
	{0x0801,     "DMA controller."},
	{0x0802,     "System timer."},
	{0x0803,     "RTC controller."},
	{0x0804,     "Hot-plug controller."},
	{0x0880,     "Other system device."},

	{0x0900,     "Keyboard controller."},
	{0x0901,     "Digitizer pen."},
	{0x0902,     "Mouse controller."},
	{0x0903,     "Scanner controller."},
	{0x0904,     "Gameport controller."},
	{0x0980,     "Other input controller."},

	{0x0A00,     "Generic docking station."},
	{0x0A80,     "Other type of docking station."},

	{0x0B00,     "386 processor."},
	{0x0B01,     "486 processor."},
	{0x0B02,     "Pentium processor."},
	{0x0B10,     "Alpha processor."},
	{0x0B20,     "PowerPC processor."},
	{0x0B30,     "MIPS processor."},
	{0x0B40,     "Co-processor."},

	{0x0C00,     "IEEE 1394."},
	{0x0C01,     "ACCESS BUS."},
	{0x0C02,     "SSA."},
	{0x0C03,     "USB controller or device."},
	{0x0C04,     "Fibre channel."},
	{0x0C05,     "System management bus."},

	{0xFFFF,     NULL}    //This is the last entry of this array.
};

#define DEFAULT_PCI_DESC    "Unknown PCI device."

static VOID OutputDevInfo(__PHYSICAL_DEVICE* lpPhyDev)
{
	BYTE                 strBuff[80];
	DWORD                dwVendor          = 0L;
	DWORD                dwDevice          = 0L;
	DWORD                dwBusNum          = 0L;
	LPSTR                lpszDes           = NULL;
	WORD                 wClassCode        = 0;
	DWORD                dwLoop            = 0L;
	LPVOID               lpParam[4];

	if(NULL == lpPhyDev)
		return;
	dwDevice = (DWORD)lpPhyDev->DevId.PCI_Identifier.wDevice;
	dwVendor = (DWORD)lpPhyDev->DevId.PCI_Identifier.wVendor;
	dwBusNum = lpPhyDev->lpHomeBus->dwBusNum;
	wClassCode = (WORD)(lpPhyDev->DevId.PCI_Identifier.dwClass >> 16);

	while(PciDevDesc[dwLoop].lpszDesc)
	{
		if(PciDevDesc[dwLoop].wClasscode == wClassCode)
		{
			lpszDes = PciDevDesc[dwLoop].lpszDesc;
			break;
		}
		dwLoop ++;
	}
	if(NULL == lpszDes)
		lpszDes = DEFAULT_PCI_DESC;

	lpParam[0]  = (LPVOID)&dwDevice;
	lpParam[1]  = (LPVOID)&dwVendor;
	lpParam[2]  = (LPVOID)&dwBusNum;
	lpParam[3]  = (LPVOID)lpszDes;

	FormString(strBuff,"    %x/%x      %d             %s",lpParam);
	PrintLine(strBuff);
}

//
//A helper routine,used to print out information of a PCI device.
//
static VOID PrintDevInfo(__PHYSICAL_DEVICE* lpPhyDev)
{
	__PCI_DEVICE_INFO*   lpPciInfo           = NULL;
	BYTE                 strBuff[80];
	DWORD                dwLoop              = 0L;
	DWORD                dwDevNum            = 0L;
	DWORD                dwFuncNum           = 0L;
	DWORD                dwVector            = 0xFF;
	UCHAR                ucResType           = 0;
	DWORD                dwStartAddr         = 0L;
	DWORD                dwEndAddr           = 0L;
	LPVOID               lppParam[6];
	
	if(NULL == lpPhyDev)  //Invalid parameter.
		return;

	lpPciInfo      = (__PCI_DEVICE_INFO*)lpPhyDev->lpPrivateInfo;
	dwDevNum       = lpPciInfo->DeviceNum;
	dwFuncNum      = lpPciInfo->FunctionNum;
	for(dwLoop = 0;dwLoop < MAX_RESOURCE_NUM;dwLoop ++)
	{
		if(RESOURCE_TYPE_INTERRUPT == lpPhyDev->Resource[dwLoop].dwResType)
		{
			dwVector = (DWORD)lpPhyDev->Resource[dwLoop].ucVector;
			break;
		}
	}
	if(RESOURCE_TYPE_EMPTY != lpPhyDev->Resource[0].dwResType)
	{
		switch(lpPhyDev->Resource[0].dwResType)
		{
		case RESOURCE_TYPE_MEMORY:
			ucResType = 'M';        //Memory.
			dwStartAddr = (DWORD)lpPhyDev->Resource[0].MemoryRegion.lpStartAddr;
			dwEndAddr   = (DWORD)lpPhyDev->Resource[0].MemoryRegion.lpEndAddr;
			break;
		case RESOURCE_TYPE_IO:
			ucResType = 'I';        //Memory.
			dwStartAddr = (DWORD)lpPhyDev->Resource[0].IOPort.wStartPort;
			dwEndAddr   = (DWORD)lpPhyDev->Resource[0].IOPort.wEndPort;
			break;
		default:
			break;
		}
	}
	//
	//Now,we shall print out all device information.
	//
	lppParam[0] = (LPVOID)&dwDevNum;
	lppParam[1] = (LPVOID)&dwFuncNum;
	lppParam[2] = (LPVOID)&dwVector;
	lppParam[3] = (LPVOID)&ucResType;
	lppParam[4] = (LPVOID)&dwStartAddr;
	lppParam[5] = (LPVOID)&dwEndAddr;

	PrintLine("    Dev/Func    IntVector    ResType    ResStart      ResEnd");
	FormString(strBuff,"         %d/%d           %d          %c    0x%x    0x%x",lppParam);
	PrintLine(strBuff);
	//
	//Print out the rest resource information.
	//
	for(dwLoop = 1;dwLoop < MAX_RESOURCE_NUM;dwLoop ++)
	{
		if(lpPhyDev->Resource[dwLoop].dwResType == RESOURCE_TYPE_EMPTY)
			break;

		switch(lpPhyDev->Resource[dwLoop].dwResType)
		{
		case RESOURCE_TYPE_MEMORY:
			ucResType = 'M';        //Memory.
			dwStartAddr = (DWORD)lpPhyDev->Resource[dwLoop].MemoryRegion.lpStartAddr;
			dwEndAddr   = (DWORD)lpPhyDev->Resource[dwLoop].MemoryRegion.lpEndAddr;
			break;
		case RESOURCE_TYPE_IO:
			ucResType = 'I';        //Memory.
			dwStartAddr = (DWORD)lpPhyDev->Resource[dwLoop].IOPort.wStartPort;
			dwEndAddr   = (DWORD)lpPhyDev->Resource[dwLoop].IOPort.wEndPort;
			break;
		default:
			continue;    //Continue to process another.
		}
		lppParam[0] = (LPVOID)&ucResType;
		lppParam[1] = (LPVOID)&dwStartAddr;
		lppParam[2] = (LPVOID)&dwEndAddr;
		FormString(strBuff,"                                   %c    0x%x    0x%x",lppParam);
		PrintLine(strBuff);
	}
	return;
}

//
//The handler of devinfo command.
//
static DWORD devinfo(__CMD_PARA_OBJ* lpCmdObj)
{
	__PHYSICAL_DEVICE*          lpPhyDev       = NULL;
	DWORD                       dwVendor       = 0;
	DWORD                       dwDevice       = 0;
	__IDENTIFIER                DevId;

	if((NULL == lpCmdObj) || (lpCmdObj->byParameterNum < 3))  //Parameter check.
	{
		PrintLine("  Usage: devinfo vendor_id device_id.");
		return SYS_DIAG_CMD_PARSER_INVALID;
	}

	if(!Str2Hex(lpCmdObj->Parameter[1],&dwVendor))  //Get the time span to beep.
	{
		PrintLine("  Invalid vendor ID parameter.");
		return SYS_DIAG_CMD_PARSER_INVALID;
	}

	if(!Str2Hex(lpCmdObj->Parameter[2],&dwDevice))  //Get the time span to beep.
	{
		PrintLine("  Invalid device ID parameter.");
		return SYS_DIAG_CMD_PARSER_INVALID;
	}
	
	DevId.dwBusType   = BUS_TYPE_PCI;
	DevId.PCI_Identifier.ucMask  = PCI_IDENTIFIER_MASK_VENDOR | PCI_IDENTIFIER_MASK_DEVICE;
	DevId.PCI_Identifier.wVendor = (WORD)dwVendor;
	DevId.PCI_Identifier.wDevice = (WORD)dwDevice;
	lpPhyDev = DeviceManager.GetDevice(
		&DeviceManager,
		BUS_TYPE_PCI,
		&DevId,
		NULL);
	if(NULL == lpPhyDev)
	{
		PrintLine("Can not find proper device.");
		return SYS_DIAG_CMD_PARSER_SUCCESS;
	}
	while(lpPhyDev)
	{
		PrintDevInfo(lpPhyDev);  //Print out the device information.
		lpPhyDev = DeviceManager.GetDevice(
			&DeviceManager,
			BUS_TYPE_PCI,
			&DevId,
			lpPhyDev);
	}
	return SYS_DIAG_CMD_PARSER_SUCCESS;
}

static DWORD cpuload(__CMD_PARA_OBJ*)
{
	__KERNEL_THREAD_MESSAGE msg;

	msg.wCommand = STAT_MSG_SHOWSTATINFO;
	SendMessage((__COMMON_OBJECT*)lpStatKernelThread,
		&msg);
	return SYS_DIAG_CMD_PARSER_SUCCESS;
}

static DWORD devlist(__CMD_PARA_OBJ*)
{
	__KERNEL_THREAD_MESSAGE msg;

	msg.wCommand = STAT_MSG_LISTDEV;
	SendMessage((__COMMON_OBJECT*)lpStatKernelThread,
		&msg);
	return SYS_DIAG_CMD_PARSER_SUCCESS;
}

