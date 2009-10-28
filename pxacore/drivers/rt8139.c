//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Sep,02 2005
//    Module Name               : RT8139.H
//    Module Funciton           : 
//                                This file is a hardware driver source file.
//                                It contains implementation code for RealTek 8139 series 
//                                network interface card's.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __NETSTD_H__
#include "..\NetCore\NetStd.h"
#endif

#ifndef __RT8139_H__
#include "..\INCLUDE\RT8139.H"
#endif

#ifndef __PCI_DRV_H__
#include "..\INCLUDE\PCI_DRV.H"
#endif

//
//The implementation of DriverEntry routine.
//This is the entry point of all device drivers.
//
BOOL DriverEntry(__DRIVER_OBJECT* lpDrvObj,__RESOURCE* lpRes)
{
	__PHYSICAL_DEVICE*            lpPhyDev     = NULL;
	__IDENTIFIER                  Id;
	DWORD                         dwStartAddr  = 0L;
	DWORD                         dwEndAddr    = 0L;
	LPVOID                        lppParam[2];
	BYTE                          strBuff[80];

	Id.dwBusType = BUS_TYPE_PCI;
	Id.PCI_Identifier.ucMask = PCI_IDENTIFIER_MASK_CLASS;
	Id.PCI_Identifier.dwClass = 0x02000000;  //Ethernet controller.
	
	lpPhyDev = DeviceManager.GetDevice(&DeviceManager,
		BUS_TYPE_PCI,
		&Id,
		NULL);
	if(NULL == lpPhyDev)
	{
		PrintLine("No network controller exists.");
		return FALSE;
	}
	PrintLine("    Resource information for Network Controller:");
	for(DWORD dwLoop = 0;dwLoop < MAX_RESOURCE_NUM;dwLoop ++)
	{
		dwStartAddr = (DWORD)lpPhyDev->Resource[dwLoop].MemoryRegion.lpStartAddr;
		dwEndAddr   = (DWORD)lpPhyDev->Resource[dwLoop].MemoryRegion.lpEndAddr;
		lppParam[0] = (LPVOID)&dwStartAddr;
		lppParam[1] = (LPVOID)&dwEndAddr;
		FormString(strBuff,"    StartAddr: 0x%x    EndAddr: 0x%x",lppParam);
		PrintLine(strBuff);
	}
	return FALSE;
}
