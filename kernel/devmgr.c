//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct,15 2005
//    Module Name               : DEVMGR.CPP
//    Module Funciton           : 
//                                This module countains device manager object's implemen-
//                                tation code.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include "stdafx.h"


#define PCI_IDENTIFIER_MASK_VENDOR      0x01
#define PCI_IDENTIFIER_MASK_DEVICE      0x02
#define PCI_IDENTIFIER_MASK_CLASS       0x04
#define PCI_IDENTIFIER_MASK_HDRTYPE     0x08
#define PCI_IDENTIFIER_MASK_ALL         (0x01 | 0x02 | 0x04 | 0x08)

/*
#ifndef __PCI_DRV_H__
#include "..\INCLUDE\PCI_DRV.H"
#endif
*/

//Pre-declaration of local routines.
static BOOL DevMgrInitialize(struct __DEVICE_MANAGER*);
static BOOL CheckPortRegion(struct __DEVICE_MANAGER*,struct __RESOURCE*);
static BOOL ReservePortRegion(struct __DEVICE_MANAGER*,struct __RESOURCE*);
static VOID ReleasePortRegion(struct __DEVICE_MANAGER*,struct __RESOURCE*);
static VOID DeleteDevice(struct __DEVICE_MANAGER*,struct __PHYSICAL_DEVICE*);
static BOOL AppendDevice(struct __DEVICE_MANAGER*,struct __PHYSICAL_DEVICE*);
static struct __PHYSICAL_DEVICE* GetDevice(struct __DEVICE_MANAGER*, DWORD, struct __IDENTIFIER*, struct __PHYSICAL_DEVICE*);

//
//The implementation of DeviceManager's initialize routine.
//
static BOOL DevMgrInitialize(struct __DEVICE_MANAGER* lpDevMgr)
{
	BOOL bResult = FALSE;
	struct __RESOURCE* lpRes = NULL;
	DWORD dwLoop = 0;

	if(NULL == lpDevMgr)     //Invalid parameter.
		return FALSE;

	//lpRes = (struct __RESOURCE*)malloc(sizeof(struct __RESOURCE));
	lpRes = (struct __RESOURCE*) KMemAlloc(sizeof(struct __RESOURCE), KMEM_SIZE_TYPE_ANY);
	if(NULL == lpRes)    //Can not allocate memory.
		return FALSE;

	lpRes->dwResType = RESOURCE_TYPE_IO;
	lpRes->IOPort.wStartPort = MIN_IO_PORT;
	lpRes->IOPort.wEndPort   = MAX_IO_PORT;

	lpDevMgr->FreePortResource.dwResType = RESOURCE_TYPE_EMPTY;
	lpDevMgr->FreePortResource.lpNext    = lpRes;
	lpDevMgr->FreePortResource.lpPrev    = lpRes;

	lpRes->lpNext  = &lpDevMgr->FreePortResource;
	lpRes->lpPrev  = &lpDevMgr->FreePortResource;

	lpDevMgr->UsedPortResource.dwResType = RESOURCE_TYPE_EMPTY;
	lpDevMgr->UsedPortResource.lpNext    = &lpDevMgr->UsedPortResource;
	lpDevMgr->UsedPortResource.lpPrev    = &lpDevMgr->UsedPortResource;

	for(dwLoop = 0;dwLoop < MAX_BUS_NUM;dwLoop ++)  //Initialize bus array.
	{
		lpDevMgr->SystemBus[dwLoop].dwBusType    = BUS_TYPE_NULL;
		lpDevMgr->SystemBus[dwLoop].lpDevListHdr = NULL;
		lpDevMgr->SystemBus[dwLoop].lpHomeBridge = NULL;
		lpDevMgr->SystemBus[dwLoop].lpParentBus  = NULL;
	}

	//lpDevMgr->AppendDevice             = AppendDevice;
	//lpDevMgr->CheckPortRegion          = CheckPortRegion;
	//lpDevMgr->DeleteDevice             = DeleteDevice;
	//lpDevMgr->GetDevice                = GetDevice;
	//lpDevMgr->ReleasePortRegion        = ReleasePortRegion;
	//lpDevMgr->ReservePortRegion        = ReservePortRegion;

	//Load system bus drivers here.
	//PciBusDriver(lpDevMgr);

	return TRUE;
}


//A helper routine,used to insert a IO port region into list,and keeps all port region
//in order.
static VOID InsertIntoList(struct __RESOURCE* lpListHdr,struct __RESOURCE* lpRes)
{
	struct __RESOURCE*            lpBefore              = NULL;

	if((NULL == lpListHdr) || (NULL == lpRes)) //Invalid parameters.
		return;

	lpBefore = lpListHdr->lpNext;
	while(lpBefore != lpListHdr)    //Travel the whole list to find a statisfying position.
	{
		if(lpBefore->IOPort.wStartPort >= lpRes->IOPort.wEndPort)  //Find it.
		{
			break;
		}
		lpBefore = lpBefore->lpNext;
	}
	
	//Insert lpRes into the list.
	lpRes->lpNext            = lpBefore;
	lpRes->lpPrev            = lpBefore->lpPrev;
	lpBefore->lpPrev->lpNext = lpRes;
	lpBefore->lpPrev         = lpRes;

	return;
}

//
//Some macros used to operate bi-direction link list.
//

#define INSERT_INTO_LIST(listhdr,node)        \
	InsertIntoList(listhdr,node)

#define DELETE_FROM_LIST(node)                \
	(node)->lpNext->lpPrev = (node)->lpPrev;  \
	(node)->lpPrev->lpNext = (node)->lpNext;


//The following routine is used to merge continues IO port region into one region.
//This is a helper routine,used by ReleasePortRegion to merge continues port region
//into one region.
//This routine searches the whole list,once find two continues region(wEndPort of the
//first region equals to wStartPort - 1 of the second region),then modify the first
//region's wEndPort to the second region's wEndPort,and delete the second region from
//list.
static VOID MergeRegion(struct __RESOURCE* lpListHdr)
{
	struct __RESOURCE*         lpFirst     = NULL;
	struct __RESOURCE*         lpSecond    = NULL;
	struct __RESOURCE*         lpTmp       = NULL;

	if(NULL == lpListHdr)
		return;

	lpFirst = lpListHdr->lpNext;

	while(lpFirst != lpListHdr)
	{
		lpSecond = lpFirst->lpNext;

		if(lpSecond == lpListHdr)    //Finished to search.
		{
			break;
		}
		if(lpFirst->IOPort.wEndPort + 1 == lpSecond->IOPort.wStartPort)  //Statisfy the continues
			                                                             //condition.
		{
			DELETE_FROM_LIST(lpSecond);    //Delete the second port region.
			lpFirst->IOPort.wEndPort = lpSecond->IOPort.wEndPort;  //Modify the first port region.
			KMemFree((LPVOID) lpSecond,KMEM_SIZE_TYPE_ANY,0L);
			//free((LPVOID)lpSecond);
			continue;
		}
		lpFirst = lpFirst->lpNext;
	}
}

//
//The implementation of ReservePortRegion routine.
//The second parameter,lpRes,indicates desired start address of port region,
//and port region's length can be calculated by minus start value from end value
//of the desired port region.
//
static BOOL ReservePortRegion(struct __DEVICE_MANAGER* lpDevMgr, struct __RESOURCE* lpRes)
{
	BOOL                       bResult                    = FALSE;
	WORD                       wSize                      = 0L;
	struct __RESOURCE*        lpStatisfy                 = NULL;
	struct __RESOURCE*        lpPotential                = NULL;
	BOOL                       bFind                      = FALSE;
	DWORD                     dwFlags                    = 0L;
	struct __RESOURCE*        lpFirstRegion              = NULL;
	struct __RESOURCE*        lpSecondRegion             = NULL;
	struct __RESOURCE*        lpRes1                     = NULL;

	if((NULL == lpDevMgr) || (NULL == lpRes))  //Invalid parameters.
		return bResult;
	if(lpRes->dwResType != RESOURCE_TYPE_IO)   //Invalid parameter.
		return bResult;
	if(lpRes->IOPort.wEndPort - lpRes->IOPort.wStartPort >= MAX_IO_PORT)
		return bResult;

	//lpRes1 = (struct __RESOURCE*)malloc(sizeof(struct __RESOURCE));

	lpRes1 = (struct __RESOURCE*)KMemAlloc(sizeof(struct __RESOURCE),KMEM_SIZE_TYPE_ANY);

	if(NULL == lpRes1)  //Can not allocate resource.
		return bResult;

	//
	//First,we look for free port list of DeviceManager object,to find
	//a block of port region statisify the original desired(lpRes).
	//
	wSize  = lpRes->IOPort.wEndPort - lpRes->IOPort.wStartPort + 1;

	__ENTER_CRITICAL_SECTION(NULL,dwFlags);

	lpStatisfy = lpDevMgr->FreePortResource.lpNext;

	while(lpStatisfy != &lpDevMgr->FreePortResource)    //Trave all the free port list.
	{
		if((lpStatisfy->IOPort.wStartPort <= lpRes->IOPort.wStartPort) &&
		   (lpStatisfy->IOPort.wEndPort   >= lpRes->IOPort.wEndPort))     //Find one.
		{
			bFind = TRUE;
			break;
		}
		if((lpStatisfy->IOPort.wEndPort - lpStatisfy->IOPort.wStartPort) >= wSize)
		{
			lpPotential = lpStatisfy;    //The current block can statisfy the size of original
			                             //request,so it is a potential statisfying region.
		}
		lpStatisfy = lpStatisfy->lpNext;
	}

	if(bFind)    //Found a IO port region statisfy the original request.
	{
		DELETE_FROM_LIST(lpStatisfy);    //Delete the region from free list.

		if(lpStatisfy->IOPort.wStartPort < lpRes->IOPort.wStartPort) //Exceed the request.
		{
			//lpFirstRegion = (struct __RESOURCE*)malloc(sizeof(struct __RESOURCE));
			lpFirstRegion = (struct __RESOURCE*)
					KMemAlloc(sizeof(struct __RESOURCE),KMEM_SIZE_TYPE_ANY);

			if(NULL != lpFirstRegion)  //Allocate successfully.
			{
				lpFirstRegion->dwResType = RESOURCE_TYPE_IO;
				lpFirstRegion->IOPort.wStartPort = lpStatisfy->IOPort.wStartPort;
				lpFirstRegion->IOPort.wEndPort   = lpRes->IOPort.wStartPort - 1;
				INSERT_INTO_LIST(&lpDevMgr->FreePortResource,lpFirstRegion);  //Insert into
			                                                                  //free list.
			}
		}

		if(lpStatisfy->IOPort.wEndPort > lpRes->IOPort.wEndPort) //Exceed the request.
		{
			//lpSecondRegion = (struct __RESOURCE*)malloc(sizeof(struct __RESOURCE));
			lpSecondRegion = (struct __RESOURCE*)
					KMemAlloc(sizeof(struct __RESOURCE),KMEM_SIZE_TYPE_ANY);

			if(NULL != lpSecondRegion)  //Allocate successfully.
			{
				lpSecondRegion->dwResType = RESOURCE_TYPE_IO;
				lpSecondRegion->IOPort.wStartPort = lpRes->IOPort.wEndPort + 1;
				lpSecondRegion->IOPort.wEndPort   = lpStatisfy->IOPort.wEndPort;
				INSERT_INTO_LIST(&lpDevMgr->FreePortResource,lpSecondRegion);
			}
		}

		lpRes1->dwResType = RESOURCE_TYPE_IO;
		lpRes1->IOPort.wStartPort = lpRes->IOPort.wStartPort;
		lpRes1->IOPort.wEndPort   = lpRes->IOPort.wEndPort;

		INSERT_INTO_LIST(&lpDevMgr->UsedPortResource,lpRes1);  //Insert it into used port list

		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		bResult = TRUE;

		KMemFree((LPVOID)lpStatisfy,KMEM_SIZE_TYPE_ANY,0L);

		//free((LPVOID)lpStatisfy);
		goto __TERMINAL;
	}

	if(lpPotential)    //Though can not find a IO port region statisfy the original request,
		       //but there is at least one region statisfy requesting size,so reserve
		       //this region.
	{
		DELETE_FROM_LIST(lpPotential);    //Delete from free list.
		if((WORD)(lpPotential->IOPort.wEndPort - lpPotential->IOPort.wStartPort) > wSize)
		{
			//lpFirstRegion = (struct __RESOURCE*)malloc(sizeof(struct __RESOURCE));
			lpFirstRegion = (struct __RESOURCE*)
					KMemAlloc(sizeof(struct __RESOURCE),KMEM_SIZE_TYPE_ANY);
			if(lpFirstRegion)  //Allocate memory successfully.
			{
				lpFirstRegion->dwResType = RESOURCE_TYPE_IO;
				lpFirstRegion->IOPort.wStartPort = lpPotential->IOPort.wStartPort + wSize;
				lpFirstRegion->IOPort.wEndPort   = lpPotential->IOPort.wEndPort;
				INSERT_INTO_LIST(&lpDevMgr->FreePortResource,lpFirstRegion);
			}
		}

		lpRes1->dwResType = RESOURCE_TYPE_IO;
		lpRes1->IOPort.wStartPort = lpPotential->IOPort.wStartPort;
		lpRes1->IOPort.wEndPort   = lpPotential->IOPort.wStartPort + wSize - 1;

		INSERT_INTO_LIST(&lpDevMgr->UsedPortResource,lpRes1);

		lpRes->IOPort.wStartPort  = lpRes1->IOPort.wStartPort;
		lpRes->IOPort.wEndPort    = lpRes1->IOPort.wEndPort;

		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		bResult = TRUE;

		KMemFree((LPVOID) lpPotential, KMEM_SIZE_TYPE_ANY, 0L);
		//free((LPVOID)lpPotential);
		goto __TERMINAL;
	}

	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

__TERMINAL:
	if(!bResult)  //Failed to reserve port.
	{
		KMemFree((LPVOID) lpRes1, KMEM_SIZE_TYPE_ANY, 0L);  //Free memory used by lpRes1.
		//free((LPVOID)lpRes1);
		return bResult;
	}
	return bResult;
}

//
//The implementation of ReleasePortRegion routine.
//
static VOID ReleasePortRegion(struct __DEVICE_MANAGER* lpDevMgr, struct __RESOURCE* lpRes)
{
	BOOL                      bResult                    = FALSE;
	struct __RESOURCE*               lpUsed            = NULL;
	DWORD                     dwFlags                  = 0L;

	if((NULL == lpDevMgr) || (NULL == lpRes)) //Invalid parameters.
		return;

	if(RESOURCE_TYPE_IO != lpRes->dwResType)  //Invalid resource descriptor.
		return;

	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	lpUsed = lpDevMgr->UsedPortResource.lpNext;

	while(lpUsed != &lpDevMgr->UsedPortResource)  //Travel whole list.
	{
		if((lpUsed->IOPort.wStartPort == lpRes->IOPort.wStartPort) &&
		   (lpUsed->IOPort.wEndPort   == lpRes->IOPort.wEndPort))  //Find the same region.
		{
			break;
		}
		lpUsed = lpUsed->lpNext;
	}
	if(lpUsed == &lpDevMgr->UsedPortResource)    //Can not find the original request one.
	{
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		return;
	}
	//
	//Now,we have found the IO port region that equals to lpRes,so delete it from the
	//used list,and insert it into free list.
	//
	DELETE_FROM_LIST(lpUsed);                               //Delete from used list.

	INSERT_INTO_LIST(&lpDevMgr->FreePortResource,lpUsed);   //Insert into free list.

	MergeRegion(&lpDevMgr->FreePortResource);        //Do a merge operation,to combine continues
	                                                 //port region into one region.
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	return;
}

//
//The implement of CheckPortRegion.
//This routine is used to check if a block of port region is used,if used,then returns FALSE,
//else,returns TRUE.
//
static BOOL CheckPortRegion(struct __DEVICE_MANAGER* lpDevMgr, struct __RESOURCE* lpRes)
{
	struct __RESOURCE*             lpTmp             = NULL;
	DWORD                   dwFlags           = 0L;
	
	if((NULL == lpDevMgr) || (NULL == lpRes))  //Invalid parameters.
		return FALSE;
	if(RESOURCE_TYPE_IO != lpRes->dwResType)   //Also invalid parameter.
		return FALSE;

	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	lpTmp = lpDevMgr->FreePortResource.lpNext;

	while(lpTmp != &lpDevMgr->FreePortResource)  //Travel the whole free list.
	{
		if((lpTmp->IOPort.wStartPort <= lpRes->IOPort.wStartPort) && 
		   (lpTmp->IOPort.wEndPort   >= lpRes->IOPort.wEndPort))  //Not used.
		{
			__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
			return TRUE;
		}
		lpTmp = lpTmp->lpNext;
	}
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	return FALSE;
}

//
//The following is a helper routine,used to check if two device identifiers are match.
//If the second Identifier(second parameter),"include" the first Identifier(first parameter),
//then it returns TRUE,otherwise,returns FALSE.
//
static BOOL DeviceIdMatch(struct __IDENTIFIER* lpFirst, struct __IDENTIFIER* lpSecond)
{
	UCHAR                 ucMask                = 0;

	if((NULL == lpFirst) || (NULL == lpSecond)) //Parameters check.
		return FALSE;
	if(lpFirst->dwBusType != lpSecond->dwBusType)  //Bus type does not match.
		return FALSE;
	if(lpFirst->dwBusType == BUS_TYPE_NULL)  //Invalid bus type.
		return FALSE;

	switch(lpFirst->dwBusType)
	{
	/*
	case BUS_TYPE_PCI:    //PCI Identifier match.
		#define ID_MEMBER(id,mem) ((id)->PCI_Identifier.mem)
		if(ID_MEMBER(lpFirst,wVendor) == ID_MEMBER(lpSecond,wVendor))
			ucMask |= PCI_IDENTIFIER_MASK_VENDOR;
		if(ID_MEMBER(lpFirst,wDevice) == ID_MEMBER(lpSecond,wDevice))
			ucMask |= PCI_IDENTIFIER_MASK_DEVICE;
		if((lpFirst->PCI_Identifier.dwClass >> 8) == 
		   (lpSecond->PCI_Identifier.dwClass >> 8))
		    ucMask |= PCI_IDENTIFIER_MASK_CLASS;
		if(ID_MEMBER(lpFirst,ucHdrType) == ID_MEMBER(lpSecond,ucHdrType))
			ucMask |= PCI_IDENTIFIER_MASK_HDRTYPE;
		return ((lpFirst->PCI_Identifier.ucMask & ucMask) == lpFirst->PCI_Identifier.ucMask);

	case BUS_TYPE_ISA:    //ISA Identifier match.
		return (lpFirst->ISA_Identifier.dwDevice == lpSecond->ISA_Identifier.dwDevice);
	*/

	default:
		break;
	}
	return FALSE;
}

//
//The implementation of GetDevice routine.
//This routine returns the appropriate physical device object according to identifier.
//
static struct __PHYSICAL_DEVICE* GetDevice(struct __DEVICE_MANAGER*   lpDevMgr,
					DWORD               dwBusType,
					struct __IDENTIFIER*       lpId,
					struct __PHYSICAL_DEVICE*  lpStart)
{
	DWORD                           dwIndex             = 0L;
	DWORD                           dwFlags             = 0L;
	struct __PHYSICAL_DEVICE*              lpPhyDev            = NULL;

	if((NULL == lpDevMgr) || (NULL == lpId))  //Invalid parameters.
		return NULL;

	if(NULL == lpStart)    //Call this routine for the first time.
	{
		__ENTER_CRITICAL_SECTION(NULL,dwFlags);

		for(dwIndex = 0;dwIndex < MAX_BUS_NUM;dwIndex ++)
		{
			if(lpDevMgr->SystemBus[dwIndex].dwBusType != dwBusType)  //Bus type not match.
				continue;

			lpPhyDev = lpDevMgr->SystemBus[dwIndex].lpDevListHdr;
			while(lpPhyDev)
			{
				#define DEVICE_ID_MATCH(id1,id2) (DeviceIdMatch((id1),(id2)))
				if(DEVICE_ID_MATCH(lpId,&lpPhyDev->DevId))  //ID match.
				{
					__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
					return lpPhyDev;    //Find a device object statisfying the request.
				}
				lpPhyDev = lpPhyDev->lpNext;
			}
		}
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		return lpPhyDev;    //If reach here,the requested device must have not been found.
	}
	else                    //This routine is called for the second time.
	{
		dwIndex = 
			(DWORD)((DWORD)(lpStart->lpHomeBus) - 
			(DWORD)(&lpDevMgr->SystemBus[0])) / sizeof(struct __SYSTEM_BUS);
		lpPhyDev = lpStart->lpNext;

		__ENTER_CRITICAL_SECTION(NULL,dwFlags);
		while((dwIndex < MAX_BUS_NUM) || lpPhyDev)
		{
			while(lpPhyDev)
			{
				if(DEVICE_ID_MATCH(lpId,&lpPhyDev->DevId))  //Find one.
				{
					__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
					return lpPhyDev;
				}
				lpPhyDev = lpPhyDev->lpNext;  //Try to match the next one.
			}
			dwIndex += 1;
			while(dwIndex < MAX_BUS_NUM)    //Try to search next BUS.
			{
				if(lpDevMgr->SystemBus[dwIndex].dwBusType != dwBusType)
				{
					dwIndex += 1;
					continue;
				}
				lpPhyDev = lpDevMgr->SystemBus[dwIndex].lpDevListHdr;
				break;    //Break form the loop.
			}
		}
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		return lpPhyDev;  //If reach here,the routine returns NULL.
	}
	return lpPhyDev;
}

//The implementation of AppendDevice routine.
//This routine append a physical device object into a BUS.
static BOOL AppendDevice(struct __DEVICE_MANAGER* lpDevMgr,struct __PHYSICAL_DEVICE* lpDev)
{
	return FALSE;
}

//The implementation of DeleteDevice routine.
//This routine deletes one physical device from system bus.
static VOID DeleteDevice(struct __DEVICE_MANAGER* lpDevMgr,struct __PHYSICAL_DEVICE* lpDev)
{
	return;
}

/****************************************************************************************
*****************************************************************************************
*****************************************************************************************
*****************************************************************************************
****************************************************************************************/

//
//The declaration of DeviceManager object.
//
struct __DEVICE_MANAGER DeviceManager = {
	{0},                               //SystemBus array.
	{0},                               //FreePortResource.
	{0},                               //UsedPortResource.
	DevMgrInitialize,                  //Initialize.
	GetDevice,                         //GetDevice.
	AppendDevice,                      //AppendDevice.
	DeleteDevice,                      //DeleteDevice.
	CheckPortRegion,                   //CheckPortRegion.
	ReservePortRegion,                 //ReservePortRegion.
	ReleasePortRegion                  //ReleasePortRegion.
};


