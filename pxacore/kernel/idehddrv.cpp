/***********************************************************************/
//    Author                    : Garry
//    Original Date             : Aug,03 2004
//    Module Name               : idehddrv.cpp
//    Module Funciton           : 
//                                This module countains the IDE interface
//                                Hard disk's control code.
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

//The following global variables countains the system's all hard disk,hard disk
//driver,and hard disk controller's information.

#define MAX_HD_NUM      0x04     //The max hard disk number supported by the system.
#define MAX_PART_NUM    0x04     //The max partition number of one hard disk.
#define BYTE_PER_SECTOR 512

/*struct __PART_INFO{
	DWORD     dwPartType;
	DWORD     dwStartSector;
	DWORD     dwStartCylinder;
	DWORD     dwStartHeader;
	DWORD     dwStartLbaSector;    //The logical sector number,in LBA mode.
};*/

/*static struct __HD_INFO{
	DWORD     dwSectorNumPerTrack;
	DWORD     dwCylinderNum;
	DWORD     dwHeaderNum;
	DWORD     dwLbaTotalSector;           //The logical sector number,in LBA mode.
	__PART_INFO PartInfo[MAX_PART_NUM];   //The max number of partition in one hard disk.
}g_HDInfo[MAX_HD_NUM];                    //The local hard disk's control array.
*/


BOOL ReadHDSector(LPVOID lpBuffer,
				  UCHAR byStartSector,
				  UCHAR byCylinderLo,
				  UCHAR byCylinderHi,
				  UCHAR byCtrlDrvHdr)     //bit 0 - 3 : Header.
				                          //bit 4     : Driver.
										  //bit 5 - 7 : Controller.
{
	BOOL          bResult      = FALSE;
	DWORD         dwCounter    = 0x00000000;
	UCHAR         byFlags      = 0x00;
	UCHAR         byDrvHdr     = 0x00;

	if(NULL == lpBuffer)
		return bResult;
	if(byCtrlDrvHdr >= 32)               //Currently,the system can not support
		                                 //more than one controller.
        return bResult;

	//---------- ** debug ** -------------
	WriteByteToPort(0x00,IDE_CTRL0_PORT_CTRL);  //Build the controller method.

	while(dwCounter < 0xFFFFFFFF)       //Wait for controller and driver ready.
	{
		ReadByteFromPort(&byFlags,IDE_CTRL0_PORT_STATUS);
		if(!CONTROLLER_READY(byFlags) || !DRIVER_READY(byFlags))
		{
			dwCounter ++;
			continue;
		}
		break;
	}
	if(0xFFFFFFFF == dwCounter)        //Time out.
	{
		PrintLine("The controller or driver is not ready in waiting time.");
		return bResult;
	}

	switch(16 & byCtrlDrvHdr)         //Determine which driver is to be accessed.
	{
	case 0:
		byDrvHdr = FORM_DRIVER_HEADER(IDE_DRV0_LBA,byCtrlDrvHdr);
		break;
	case 16:
		byDrvHdr = FORM_DRIVER_HEADER(IDE_DRV1_LBA,byCtrlDrvHdr);
		break;
	default:
		return bResult;
		break;
	}

	WriteByteToPort(0x4b,IDE_CTRL0_PORT_PRECOMP);
	WriteByteToPort(0x01,IDE_CTRL0_PORT_SECTORNUM);
	WriteByteToPort(byStartSector,IDE_CTRL0_PORT_STARTSECTOR);
	WriteByteToPort(byCylinderLo,IDE_CTRL0_PORT_CYLINDLO);
	WriteByteToPort(byCylinderHi,IDE_CTRL0_PORT_CYLINDHI);
	WriteByteToPort(byDrvHdr,IDE_CTRL0_PORT_HEADER);
	WriteByteToPort(IDE_CMD_READ,IDE_CTRL0_PORT_CMD);

	while(TRUE)
	{
		ReadByteFromPort(&byFlags,IDE_CTRL0_PORT_STATUS);
		if(CONTROLLER_READY(byFlags))
			break;
	}

    ReadWordStringFromPort(lpBuffer,BYTE_PER_SECTOR,IDE_CTRL0_PORT_DATA);

	bResult = TRUE;
	return bResult;
}
