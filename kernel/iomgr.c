//***********************************************************************/
//    Author                    : Garry
//    Original Date             : May,02 2005
//    Module Name               : iomgr.cpp
//    Module Funciton           : 
//                                This module countains the implementation code of
//                                I/O Manager.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include "stdafx.h"
#include "l_stdio.h"

//
//In front of this file,we implement three call back routines first,these three
//call back routines are called by device driver(s) to report some events to IOManager.
//These three routines are members of DRCB object,i.e,their base addresses are countained 
//in DRCB object.
//The first routine is WaitForCompletion,this routine is called when device driver(s) 
//submit a device operation transaction,such as READ or WRITE,and to wait the operation
//over,in this situation,device driver(s) calls this this routine,put the current kernel
//thread to BLOCKED queue.
//The second routine is OnCompletion,this routine is called when device request operation
//over,to indicate the IOManager this event,and wakeup the kernel thread which is blocked
//in WaitForCompletion routine.
//The third routine is OnCancel,which is called when an IO operation is canceled.
//

//
//The implementation of WaitForCompletion.
//This routine does the following:
// 1. Check the validation of parameter(s);
// 2. Block the current kernel thread.
//

static DWORD WaitForCompletion(struct __COMMON_OBJECT* lpThis)
{
	struct __DRCB* lpDrcb           = NULL;
	struct __EVENT* lpEvent          = NULL;

	if(NULL == lpThis) //Invalid parameter.
		return 0L;

	lpDrcb   = (struct __DRCB*)lpThis;
	lpEvent  = lpDrcb->lpSynObject;
	
	//Block the current kernel thread.
	lpEvent->WaitForThisObject((struct __COMMON_OBJECT*) lpEvent);  

	return 1L;
}

//
//The implementation of OnCompletion.
//This routine does the following:
// 1. Check the parameter's validation;
// 2. Wakeup the kernel thread who waiting for the current device operation.

static DWORD OnCompletion(struct __COMMON_OBJECT* lpThis)
{
	struct __EVENT*              lpEvent          = NULL;

	if(NULL == lpThis)
		return 0L;

	lpEvent = ((struct __DRCB*)lpThis)->lpSynObject;
	lpEvent->SetEvent((struct __COMMON_OBJECT*)lpEvent);  //Wakeup kernel thread.
	return 1L;
}

//The implementation of OnCancel.
//This routine does the following:
// 1. 
static DWORD OnCancel(struct __COMMON_OBJECT* lpThis)
{
	if(NULL == lpThis)    //Parameter check.
		return 0L;

	return 1L;
}

//
//The Initialize routine and UnInitialize routine of DRCB.
//

BOOL DrcbInitialize(struct __COMMON_OBJECT*  lpThis)
{
	struct __EVENT*          lpSynObject     = NULL;
	struct __DRCB*           lpDrcb          = NULL;
	DWORD             dwFlags         = 0L;

	if(NULL == lpThis)
		return FALSE;

	lpDrcb = (struct __DRCB*)lpThis;

	lpSynObject = (struct __EVENT*)ObjectManager.CreateObject(
		&ObjectManager,
		NULL,
		OBJECT_TYPE_EVENT);

	if(NULL == lpSynObject)    //Failed to create event object.
		return FALSE;

	if(!lpSynObject->Initialize((struct __COMMON_OBJECT*) lpSynObject)) //Failed to initialize.
	{
		ObjectManager.DestroyObject(&ObjectManager,
			(struct __COMMON_OBJECT*)lpSynObject);
		return FALSE;
	}

	lpDrcb->lpSynObject        = lpSynObject;

	//ENTER_CRITICAL_SECTION();
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	lpDrcb->lpKernelThread     = KernelThreadManager.lpCurrentKernelThread;
	//LEAVE_CRITICAL_SECTION();
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

	lpDrcb->dwDrcbFlag         = 0L;
	lpDrcb->dwStatus           = DRCB_STATUS_INITIALIZED;
	lpDrcb->dwRequestMode      = 0L;
	lpDrcb->dwCtrlCommand      = 0L;

	lpDrcb->dwOutputLen        = 0L;
	lpDrcb->lpOutputBuffer     = NULL;
	lpDrcb->dwInputLen         = 0L;
	lpDrcb->lpInputBuffer      = NULL;

	lpDrcb->lpNext             = NULL;
	lpDrcb->lpPrev             = NULL;

	lpDrcb->WaitForCompletion  = NULL;
	lpDrcb->OnCompletion       = NULL;
	lpDrcb->OnCancel           = NULL;

	lpDrcb->lpDrcbExtension    = NULL;
	return TRUE;
}

//The Uninitialize of DRCB.
VOID DrcbUninitialize(struct __COMMON_OBJECT*  lpThis)
{
	struct __DRCB*           lpDrcb            = NULL;

	if(NULL == lpThis)
		return;

	lpDrcb = (struct __DRCB*)lpThis;

	if(lpDrcb->lpSynObject != NULL)
		ObjectManager.DestroyObject(&ObjectManager,
		(struct __COMMON_OBJECT*)(lpDrcb->lpSynObject));

	return;
}

//The implementation of driver object's initialize routine.
BOOL DrvObjInitialize(struct __COMMON_OBJECT*  lpThis)
{
	struct __DRIVER_OBJECT*       lpDrvObj     = NULL;

	if(NULL == lpThis)
		return FALSE;

	lpDrvObj = (struct __DRIVER_OBJECT*)lpThis;

	lpDrvObj->lpPrev            = NULL;
	lpDrvObj->lpNext            = NULL;

	return TRUE;
}

//The implementation of driver object's Uninitialize routine.
VOID DrvObjUninitialize(struct __COMMON_OBJECT* lpThis)
{
	return;
}

//The implementation of device object's initialize routine.
BOOL DevObjInitialize(struct __COMMON_OBJECT* lpThis)
{
	struct __DEVICE_OBJECT*          lpDevObject = NULL;

	if(NULL == lpThis)
		return FALSE;

	lpDevObject = (struct __DEVICE_OBJECT*)lpThis;

	lpDevObject->lpPrev           = NULL;
	lpDevObject->lpNext           = NULL;

	lpDevObject->DevName[0]       = 0;
	//lpDevObject->dwDevType        = DEVICE_TYPE_NORMAL;
	lpDevObject->lpDriverObject   = NULL;

	lpDevObject->dwStartPort      = 0L;
	lpDevObject->dwEndPort        = 0L;

	lpDevObject->dwDmaChannel     = 0L;
	lpDevObject->dwInterrupt      = 0L;

	lpDevObject->lpMemoryStartAddr = NULL;
	lpDevObject->dwMemLen          = NULL;
	lpDevObject->lpDevExtension    = NULL;

	return TRUE;
}

//
//Device object's Uninitialize routine.
//

VOID DevObjUninitialize(struct __COMMON_OBJECT* lpThis)
{
	return;
}


//
//The implementation of IOManager.
//

//
//The initialize routine of IOManager.
//This routine does the following:
// 1. 
//

static BOOL IOManagerInitialize(struct __COMMON_OBJECT* lpThis)
{
	BOOL                       bResult         = FALSE;
	struct __IO_MANAGER*              lpIoManager     = NULL;

	if(NULL == lpThis)    //Parameter check.
		return bResult;

	bResult = TRUE;

	return bResult;
}

//
//The CreateFile routine's implementation of IOManager.
//Several tips about this routine:
// 1. In current version implementation of Hello Taiwan,all devices are treated as files,
//    so,if users want to access device,he or she can open the target device by calling
//    this routine;
// 2. One file or device can be opend as READ ONLY,WRITE ONLY,or READ WRITE,the dwAccessMode
//    parameter of this routine indicates the opening mode;
// 3. In current version,one file or device can be opend more than one time,so,the
//    dwShareMode indicates the re-open mode of the currently opend file,for example,if one
//    kernel thread opens a file as READ WRITE mode,and also indicates the OS that this
//    file can only be re-opend as READ mode(by seting the appropriate value of the
//    dwShareMode parameter),so,if there is another kernel thread want to open the file
//    as READ WRITE mode or WRITE mode,it will fail,by contraries,if the second kernel
//    thread want to open the file only in READ ONLY mode,it will success.
// 4. The last parameter,lpReserved,is a reserved parameter that may be used in the future.
//Once success,this routine returns the base address of the device object that opend,
//otherwise,it will return a NULL value to indicate the failure,user can determine the
//failing reason by calling GetLastError routine.
//
//The routine does the following:
// 1. 
//

static struct __COMMON_OBJECT* CreateFile(struct __COMMON_OBJECT*  lpThis,
								   LPSTR             lpszFileName,
								   DWORD             dwAccessMode,
								   DWORD             dwShareMode,
								   LPVOID            lpReserved)
{
	struct __COMMON_OBJECT*               lpFileObj         = NULL;

	if((NULL == lpThis)
	   || (NULL == lpszFileName)) //Parameters check.
	   return lpFileObj;

	return lpFileObj;
}

//
//The ReadFile's implementation.
//The routine does the following:
// 1. Create a DRCB object,and initializes it;
// 2. Get the read block size of this device;
// 3. According to the block size,submit the read quest;
// 4. According to the result,set appropriate return value(s).
//

static BOOL  ReadFile(struct __COMMON_OBJECT*    lpThis,
		    struct __COMMON_OBJECT*    lpFileObj,
		    DWORD               dwByteSize,
		    LPVOID              lpBuffer,
		    DWORD*              lpdwReadSize)
{

	BOOL              bResult             = FALSE;
	struct __DRCB*           lpDrcb              = NULL;
	struct __IO_MANAGER*     lpIoManager         = NULL;
	struct __DEVICE_OBJECT*  lpDevObject         = NULL;
	struct __DRIVER_OBJECT*  lpDrvObject         = NULL;
	DWORD             dwReadBlockSize     = 0L;
	DWORD             dwTotalSize         = 0L;

	if((NULL == lpThis)
	   || (NULL == lpFileObj)
	   || (0 == dwByteSize)
	   || (NULL == lpBuffer))    //Parameters check.
	   goto __TERMINAL;

	lpIoManager = (struct __IO_MANAGER*)lpThis;
	lpDevObject = (struct __DEVICE_OBJECT*)lpFileObj;
	lpDrvObject = lpDevObject->lpDriverObject;

	lpDrcb = (struct __DRCB*) ObjectManager.CreateObject(&ObjectManager,
		NULL,
		OBJECT_TYPE_DRCB);

	if(NULL == lpDrcb)        //Failed to create DRCB object.
		goto __TERMINAL;

	if(!lpDrcb->Initialize((struct __COMMON_OBJECT*)lpDrcb))  //Failed to initialize.
		goto __TERMINAL;

	//
	//The following code gets the read block size by calling DeviceCtrl.
	//
	lpDrcb->dwRequestMode    = DRCB_REQUEST_MODE_IOCTRL;
	lpDrcb->dwCtrlCommand    = IOCONTROL_GET_READ_BLOCK_SIZE;
	lpDrcb->dwStatus         = DRCB_STATUS_INITIALIZED;
	lpDrcb->dwOutputLen      = sizeof(DWORD);
	lpDrcb->lpOutputBuffer   = (LPVOID)&dwReadBlockSize;    //When returned,the read block
	                                                        //size will be countained in
	                                                        //dwReadBlockSize variable.

	lpDrcb->WaitForCompletion = WaitForCompletion;          //Initializes the control routine.
	lpDrcb->OnCompletion      = OnCompletion;
	lpDrcb->OnCancel          = OnCancel;

	lpDrvObject->DeviceCtrl((struct __COMMON_OBJECT*)lpDrvObject,
		(struct __COMMON_OBJECT*)lpDevObject,
		lpDrcb);

	if(DRCB_STATUS_SUCCESS != lpDrcb->dwStatus)    //Can not get read block size successfully.
		goto __TERMINAL;

	//
	//The following code commits the read operations by calling DeviceRead routine.
	//
	dwTotalSize = dwByteSize;
	lpDrcb->dwRequestMode = DRCB_REQUEST_MODE_READ;
	lpDrcb->dwStatus      = DRCB_STATUS_INITIALIZED;

	while(TRUE)
	{
		lpDrcb->dwOutputLen    = 
			dwTotalSize > dwReadBlockSize ? dwReadBlockSize : dwTotalSize;
		lpDrcb->lpOutputBuffer = 
			lpBuffer;
		lpDrvObject->DeviceRead((struct __COMMON_OBJECT*)lpDrvObject,
			(struct __COMMON_OBJECT*)lpDevObject,
			lpDrcb);
		if(DRCB_STATUS_SUCCESS != lpDrcb->dwStatus)  //Can not read successfully.
			goto __TERMINAL;    //Exit the read process.
		if(lpDrcb->dwOutputLen < dwReadBlockSize)  //This suituation indicates the following
			                                       //case:
		//1. At the end of the target file,but
		//   the original request data size is
		//   not satisfied;
		//2. The read transaction is over.
		
		{
			*lpdwReadSize  = dwByteSize - dwTotalSize;
			*lpdwReadSize += lpDrcb->dwOutputLen;  //Set the actually read byte size.
			bResult = TRUE;
			goto __SUCCESSFUL;
		}

		dwTotalSize -= dwReadBlockSize;
		if(0 == dwTotalSize)
		{
			*lpdwReadSize = dwByteSize;
			bResult = TRUE;
			goto __SUCCESSFUL;
		}
		lpBuffer     = (LPVOID)((DWORD)lpBuffer + dwReadBlockSize);
	}

__SUCCESSFUL:

	ObjectManager.DestroyObject(&ObjectManager,
		(struct __COMMON_OBJECT*)lpDrcb);    //Destroy the DRCB object.

__TERMINAL:
	if(!bResult)    //This routine failed.
	{
		if(lpDrcb != NULL)    //Destroy the DRCB object.
		{
			ObjectManager.DestroyObject(&ObjectManager,
				(struct __COMMON_OBJECT*)lpDrcb);
		}
	}
	return bResult;
}

//
//The WriteFile's implementation.
//The routine does the following:
// 1. Create a DRCB object,and initialize it;
// 2. Commit the write transaction by calling DeviceWrite routine;
// 3. According to the result,set appropriate return value(s).
//

static BOOL WriteFile(struct __COMMON_OBJECT*  lpThis,
		   struct __COMMON_OBJECT*  lpFileObj,
		   DWORD             dwWriteSize,
		   LPVOID            lpBuffer,
		   DWORD*            lpWrittenSize)
{
	BOOL              bResult           = FALSE;
	struct __DRCB*           lpDrcb            = NULL;
	struct __DEVICE_OBJECT*  lpDevObject       = NULL;
	struct __DRIVER_OBJECT*  lpDrvObject       = NULL;
	DWORD             dwWriteBlockSize  = 0L;
	DWORD             dwTotalSize       = 0L;

	if((NULL == lpThis) ||
	  (NULL == lpFileObj) ||
	  (0 == dwWriteSize) ||
	  (NULL == lpBuffer))    //Parameters check.
	  return bResult;

	lpDevObject = (struct __DEVICE_OBJECT*)lpFileObj;
	lpDrvObject = lpDevObject->lpDriverObject;

	lpDrcb = (struct __DRCB*)ObjectManager.CreateObject(&ObjectManager,
		NULL,
		OBJECT_TYPE_DRCB);

	if(NULL == lpDrcb)  //Failed to create DRCB object.
		goto __TERMINAL;

	if(!lpDrcb->Initialize((struct __COMMON_OBJECT*)lpDrcb)) //Failed to initialize.
		goto __TERMINAL;

	//
	//The following code gets the write block size of this device object.
	//
	lpDrcb->dwRequestMode     = DRCB_REQUEST_MODE_IOCTRL;
	lpDrcb->dwCtrlCommand     = IOCONTROL_GET_WRITE_BLOCK_SIZE;
	lpDrcb->dwStatus          = DRCB_STATUS_INITIALIZED;
	lpDrcb->dwOutputLen       = sizeof(DWORD);
	lpDrcb->lpOutputBuffer    = (LPVOID)&dwWriteBlockSize;

	lpDrcb->WaitForCompletion = WaitForCompletion;
	lpDrcb->OnCancel          = OnCancel;
	lpDrcb->OnCompletion      = OnCompletion;

	lpDrvObject->DeviceCtrl((struct __COMMON_OBJECT*)lpDrvObject,
		(struct __COMMON_OBJECT*)lpDevObject,
		lpDrcb);    //Get the write block size.
	if(DRCB_STATUS_SUCCESS != lpDrcb->dwStatus)    //Failed to get the write block size.
		goto __TERMINAL;

	//
	//Now,the variable dwWriteBlockSize countains the write block size.
	//The following code,we commit the write transaction to device driver object
	//by calling DeviceWrite.
	//

	dwTotalSize           = dwWriteSize;
	lpDrcb->dwRequestMode = DRCB_REQUEST_MODE_WRITE;
	lpDrcb->dwStatus      = DRCB_STATUS_INITIALIZED;

	while(TRUE)
	{
		lpDrcb->dwInputLen  = dwTotalSize > dwWriteBlockSize ? dwWriteBlockSize : dwTotalSize;
		lpDrcb->lpInputBuffer  = lpBuffer;

		lpDrvObject->DeviceWrite((struct __COMMON_OBJECT*)lpDrvObject,
			(struct __COMMON_OBJECT*)lpDevObject,
			lpDrcb);  //Commit the write transaction.

		if(DRCB_STATUS_SUCCESS != lpDrcb->dwStatus)  //Failed to write.
		{
			goto __TERMINAL;
		}
		if(dwTotalSize <= dwWriteBlockSize)    //This indicates the write transaction is over.
		{
			*lpWrittenSize = dwWriteSize;
			bResult = TRUE;
			goto __TERMINAL;
		}
		dwTotalSize -= dwWriteBlockSize;
		lpBuffer = (LPVOID) ((DWORD)lpBuffer + dwWriteBlockSize);  //Adjust the buffer.
	}

__TERMINAL:
	if(lpDrcb != NULL)    //Destroy the DRCB object.
	{
		ObjectManager.DestroyObject(&ObjectManager,
			(struct __COMMON_OBJECT*)lpDrcb);
	}
	return bResult;
}

//
//The implementation of CloseFile.
//This routine does the following:
// 1. Check the validation of the parameters;
// 2. Check the device type of the target device object;
// 3. If the device to closed is a normal file,then destroy the object;
// 4. If the target deivce is not a normal file,then reduce the
//    reference counter,and return.
//

static VOID CloseFile(struct __COMMON_OBJECT*  lpThis, struct __COMMON_OBJECT*  lpFileObj)
{
	struct __IO_MANAGER*              lpIoManager        = NULL;
	struct __DEVICE_OBJECT*           lpDevObject        = NULL;

	if((NULL == lpThis) || (NULL == lpFileObj)) //Parameter check.
		return;

	lpIoManager = (struct __IO_MANAGER*)lpThis;
	lpDevObject = (struct __DEVICE_OBJECT*)lpFileObj;

	/*if(lpDevObject->dwDevType != DEVICE_TYPE_FILE)  //Not a normal file.
	{
		//ENTER_CRITICAL_SECTION();
		//lpDevObject->dwRefCounter --;
		//LEAVE_CRITICAL_SECTION();
		return;
	}*/

	//
	//If control flow reaches here,it indicates that the device to be closed is
	//a normal file,then,we will destroy this file object by calling DestroyDevice
	//routine of IOManager object.
	//
	//lpIoManager->DestroyDevice(lpThis,lpFileObj);  //Destroy the file object.
}

//
//The implementation of IOControl.
//This routine does the following:
// 1. 
//

static BOOL IOControl(struct __COMMON_OBJECT* lpThis, struct __COMMON_OBJECT* lpFileObj,
					  DWORD            dwCommand,
					  LPVOID           lpParameter,
					  DWORD            dwOutputBufLen,
					  LPVOID           lpOutputBuffer,
					  DWORD*           lpdwOutputFilled)
{
	BOOL              bResult          = FALSE;

	if((NULL == lpThis) ||
	   (NULL == lpFileObj))  //Parameters check.
	   return bResult;

	return bResult;
}

//
//The implementation of SetFilePointer.
//This routine does the following:
// 1. 
//

static BOOL SetFilePointer(struct __COMMON_OBJECT* lpThis,
						   struct __COMMON_OBJECT* lpFileObj,
						   DWORD            dwWhereBegin,
						   int              nOffset)
{
	BOOL                   bResult          = FALSE;

	if((NULL == lpThis) || (NULL == lpFileObj)) //Parameters check.
		return bResult;

	return bResult;
}

//
//The implementation of FlushFile.
//This routine does the following:
// 1. 
//

static BOOL FlushFile(struct __COMMON_OBJECT*  lpThis,
					  struct __COMMON_OBJECT*  lpFileObj)
{
	BOOL              bResult           = FALSE;

	if((NULL == lpThis) || (NULL == lpFileObj))  //Parameters check.
		return bResult;

	return bResult;
}

//
//The implementation of CreateDevice,this routine is called by device
//driver(s) to create device object.Generally,this routine is called in
//DriverEntry of device driver(s).
//This routine does the following:
// 1. Creates a device object by calling ObjectManager's interface;
// 2. Initializes the device object;
// 3. Allocates a block of memory as device object's extension;
// 4. Inserts the device object into device object's list.
//

static struct __DEVICE_OBJECT* CreateDevice(struct __COMMON_OBJECT*  lpThis,
									 LPSTR             lpszDevName,
									 DWORD             dwAttribute,
									 DWORD             dwBlockSize,
									 DWORD             dwMaxReadSize,
									 DWORD             dwMaxWriteSize,
									 LPVOID            lpDevExtension,
									 struct __DRIVER_OBJECT*  lpDrvObject)
{
	struct __DEVICE_OBJECT*                 lpDevObject       = NULL;
	struct __IO_MANAGER*                    lpIoManager       = (struct __IO_MANAGER*)lpThis;
	DWORD                            dwFlags           = 0L;

	//Check the parameters.
	if((NULL == lpThis) || (NULL == lpszDevName) || (NULL == lpDrvObject))
	{
		BUG();
		return NULL;
	}

	//if(strlen(lpszDevName) > MAX_DEV_NAME_LEN)  //The device's name is too long.
	//{
	//	return NULL;
	//}
	
	lpDevObject    = (struct __DEVICE_OBJECT*)ObjectManager.CreateObject(
		&ObjectManager,
		NULL,
		OBJECT_TYPE_DEVICE);
	if(NULL == lpDevObject)      //Failed to create device object.
	{
		return NULL;
	}

	if(!lpDevObject->Initialize((struct __COMMON_OBJECT*)lpDevObject)) //Failed to initialize the object.
	{
		ObjectManager.DestroyObject(&ObjectManager,
			(struct __COMMON_OBJECT*)lpDevObject);
		return NULL;
	}

	//Initialize the device object's members.
	lpDevObject->lpDevExtension = lpDevExtension;
	lpDevObject->lpDriverObject = lpDrvObject;
	lpDevObject->dwAttribute    = dwAttribute;
	lpDevObject->dwBlockSize    = dwBlockSize;
	lpDevObject->dwMaxWriteSize = dwMaxWriteSize;
	lpDevObject->dwMaxReadSize  = dwMaxReadSize;

	//Need Implement strcpy
	//strcpy(lpszDevName,(LPSTR)&lpDevObject->DevName[0]);

	//
	//The following code add the device object into device object's list.
	//
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	if(NULL == lpIoManager->lpDeviceRoot)  //This is the first object.
	{
		lpIoManager->lpDeviceRoot = lpDevObject;
	}
	else    //This is not the first object.
	{
		lpDevObject->lpNext                  = lpIoManager->lpDeviceRoot;
		lpDevObject->lpPrev                  = NULL;
		lpIoManager->lpDeviceRoot->lpPrev = lpDevObject;
		lpIoManager->lpDeviceRoot         = lpDevObject;
	}
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

	return lpDevObject;
}

//
//The implementation of DestroyDevice.
//

static VOID DestroyDevice(struct __COMMON_OBJECT* lpThis,
						  struct __DEVICE_OBJECT* lpDeviceObject)
{
	struct __IO_MANAGER*         lpIoManager    = (struct __IO_MANAGER*)lpThis;
	DWORD                 dwFlags        = 0L;

	if((NULL == lpThis) || (NULL == lpDeviceObject)) //Parameters check.
	{
		BUG();
		return;
	}

	//
	//The following code deletes the device object from system list.
	//
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	if(NULL == lpDeviceObject->lpPrev)    //This is the first object.
	{
		if(NULL == lpDeviceObject->lpNext)  //This is the last object.
		{
			lpIoManager->lpDeviceRoot = NULL;
		}
		else    //This is not the last object.
		{
			lpDeviceObject->lpNext->lpPrev = NULL;
			lpIoManager->lpDeviceRoot      = lpDeviceObject->lpNext;
		}
	}
	else    //This is not the first object.
	{
		if(NULL == lpDeviceObject->lpNext)  //This is the last object.
		{
			lpDeviceObject->lpPrev->lpNext = NULL;
		}
		else    //This is not the last object.
		{
			lpDeviceObject->lpPrev->lpNext = lpDeviceObject->lpNext;
			lpDeviceObject->lpNext->lpPrev = lpDeviceObject->lpPrev;
		}
	}
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

	//Destroy the device object.
	ObjectManager.DestroyObject(&ObjectManager,
		(struct __COMMON_OBJECT*)lpDeviceObject);

	return;
}

//
//The implementation of ReserveResource.
//This routine does the following:
// 1. 
//

static BOOL ReserveResource(struct __COMMON_OBJECT*    lpThis, struct __RESOURCE_DESCRIPTOR* lpResDesc)
{
	BOOL                    bResult             = FALSE;

	if((NULL == lpThis) || (NULL == lpResDesc)) //Parameters check.
		return bResult;

	return bResult;
}

//
//The implementation of LoadDriver.
//This routine is used by OS loader to load device drivers.
//
static BOOL LoadDriver(__DRIVER_ENTRY DrvEntry)
{
	struct __DRIVER_OBJECT*   lpDrvObject  = NULL;

	if(NULL == DrvEntry)  //Invalid parameter.
	{
		BUG();
		return FALSE;
	}
	
	//Request Object for Driver
	lpDrvObject = (struct __DRIVER_OBJECT*)ObjectManager.CreateObject(
		&ObjectManager,
		NULL,
		OBJECT_TYPE_DRIVER);

	if(NULL == lpDrvObject)  //Can not create driver object.
	{
		return FALSE;
	}

	if(!lpDrvObject->Initialize((struct __COMMON_OBJECT*) lpDrvObject)) //Initialize failed.
	{
		return FALSE;
	}

	//Call the driver entry.
	if(DrvEntry(lpDrvObject))
	{
		return TRUE;
	}

	//Failed to call DrvEntry routine,so release the driver object.
	ObjectManager.DestroyObject(
		&ObjectManager,
		(struct __COMMON_OBJECT*)lpDrvObject);

	return FALSE;
}

/*************************************************************************
**************************************************************************
**************************************************************************
**************************************************************************
*************************************************************************/

//
//The following code defines one of the global objects in Hello Taiwan - IOManager.
//This object is a global object,and only one in the whole system life-cycle.
//

struct __IO_MANAGER IOManager = {
	NULL,                                   //lpDeviceRoot.
	NULL,                                   //lpDriverRoot.
	NULL,                                   //lpResDescriptor.
	IOManagerInitialize,                    //Initialize.
	CreateFile,                             //CreateFile.
	ReadFile,                               //ReadFile.
	WriteFile,                              //WriteFile.
	CloseFile,                              //CloseFile.
	IOControl,                              //IOControl.
	SetFilePointer,                         //SetFilePointer.
	FlushFile,                              //FileFlush.

	CreateDevice,                           //CreateDevice.
	DestroyDevice,                          //DestroyDevice.
	ReserveResource,                        //ReserveResource.
	LoadDriver
};
