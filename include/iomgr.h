//***********************************************************************/
//    Author                    : Garry
//    Original Date             : May,01 2005
//    Module Name               : iomgr.h
//    Module Funciton           : 
//                                This module countains the pre-definition of Input and
//                                Output(I/O) manager.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __IOMGR_H__
#define __IOMGR_H__

//
//Constants used by this module(IOManager).
//

#define MAX_DEV_NAME_LEN    256    //The maximal length of device object's name.

//
//The definition of DRCB(Device Request Control Block).
//This object is used to trace the device request operations,it is the core object
//used by IOManager and device drivers to communicate,and also the core object used
//between device drivers to communicate each other.
//

typedef DWORD (*DRCB_WAITING_ROUTINE)(struct __COMMON_OBJECT*);
typedef DWORD (*DRCB_COMPLETION_ROUTINE)(struct __COMMON_OBJECT*);
typedef DWORD (*DRCB_CANCEL_ROUTINE)(struct __COMMON_OBJECT*);

BEGIN_DEFINE_OBJECT(__DRCB)
    INHERIT_FROM_COMMON_OBJECT
    	struct __EVENT*                          lpSynObject;      //Used to synchronize the access.
    	struct __KERNEL_THREAD_OBJECT*           lpKernelThread;   //The kernel thread who originates
	                                                    //the device access.
	DWORD                             dwDrcbFlag;
	DWORD                             dwStatus;
	DWORD                             dwRequestMode;    //Read,write,io control,etc.
	DWORD                             dwCtrlCommand;    //If the request mode is io control,
	                                                    //then is variable is used to indicate
	                                                    //the control command.

	//DWORD                             dwOffset;
	DWORD                             dwOutputLen;      //Output buffer's length.
	LPVOID                            lpOutputBuffer;   //Output buffer.

	DWORD                             dwInputLen;
	LPVOID                            lpInputBuffer;

	__DRCB*                           lpNext;
	__DRCB*                           lpPrev;

	DRCB_WAITING_ROUTINE              WaitForCompletion;  //Called when waiting the current
	                                                      //device request to finish.
	DRCB_COMPLETION_ROUTINE           OnCompletion;       //Called when the current request
	                                                      //is over.
	DRCB_CANCEL_ROUTINE               OnCancel;

	//DWORD                             dwDrcbExtension[1];
	LPVOID                            lpDrcbExtension;
END_DEFINE_OBJECT()                                       //End of __DRCB's definition.

//
//Initialize routine and Uninitialize routine's definition.
//

BOOL DrcbInitialize(struct __COMMON_OBJECT*);
VOID DrcbUninitialize(struct __COMMON_OBJECT*);

//
//DRCB status definition.
//

#define DRCB_STATUS_INITIALIZED       0x00000000
#define DRCB_STATUS_FAIL              0x00000001
#define DRCB_STATUS_SUCCESS           0x00000002
#define DRCB_STATUS_PENDING           0x00000004
#define DRCB_STATUS_CANCELED          0x00000008

//
//DRCB request mode definition.
//

#define DRCB_REQUEST_MODE_READ        0x00000001        //Read operations.
#define DRCB_REQUEST_MODE_WRITE       0x00000002        //Write operations.
#define DRCB_REQUEST_MODE_OPEN        0x00000004        //Open operations.
#define DRCB_REQUEST_MODE_CLOSE       0x00000008        //Close operations.
#define DRCB_REQUEST_MODE_SEEK        0x00000010        //Seek(or SetFilePointer) operations.
#define DRCB_REQUEST_MODE_FLUSH       0x00000020        //Flush operations.
#define DRCB_REQUEST_MODE_CREATE      0x00000040        //Create operations.
#define DRCB_REQUEST_MODE_DESTROY     0x00000080        //Destroy operations.
#define DRCB_REQUEST_MODE_IOCTRL      0x00000100        //IOControl operations.

//
//The following definitions are used to indicate the appropriate I/O control command.
//

#define IOCONTROL_GET_READ_BLOCK_SIZE    0x00000001  //Get read block size.
#define IOCONTROL_GET_WRITE_BLOCK_SIZE   0x00000002  //Get write block size.

//
//The definition of __RESOURCE_DESCRIPTOR.
//This object is used to describe system resource,such as input/output port,DMA channel,
//interrupt vector,etc.
//

BEGIN_DEFINE_OBJECT(__RESOURCE_DESCRIPTOR)
    __RESOURCE_DESCRIPTOR*    lpNext;      //Pointes to next descriptor.
    __RESOURCE_DESCRIPTOR*    lpPrev;      //Pointes to previous descriptor.

    DWORD              dwStartPort;
    DWORD              dwEndPort;
	DWORD              dwDmaChannel;
	DWORD              dwInterrupt;
	LPVOID             lpMemoryStartAddr;
	DWORD              dwMemoryLen;
END_DEFINE_OBJECT()  //End definition of __RESOURCE_DESCRIPTOR.

//
//The definition of __DRIVER_OBJECT.
//

BEGIN_DEFINE_OBJECT(__DRIVER_OBJECT)
    INHERIT_FROM_COMMON_OBJECT

	__DRIVER_OBJECT*            lpPrev;
    __DRIVER_OBJECT*            lpNext;

	DWORD          (*DeviceRead)(struct __COMMON_OBJECT*  lpDrv,    //Read routine.
	                             struct __COMMON_OBJECT*  lpDev,
								 __DRCB*           lpDrcb);

    DWORD          (*DeviceWrite)(struct __COMMON_OBJECT*  lpDrv,   //Write routine.
		                          struct __COMMON_OBJECT*  lpDev,
								  __DRCB*           lpDrcb);

	DWORD          (*CreateFileSystem)(struct __COMMON_OBJECT*  lpDrv,    //CreateFileSystem.
		                               struct __COMMON_OBJECT*  lpDev,
									   __DRCB*           lpDrcb);

	DWORD          (*DeviceCtrl)(struct __COMMON_OBJECT*  lpDrv,
		                         struct __COMMON_OBJECT*  lpDev,
								 __DRCB*           lpDrcb);

	DWORD          (*DeviceFlush)(struct __COMMON_OBJECT*  lpDrv,
		                          struct __COMMON_OBJECT*  lpDev,
								  __DRCB*           lpDrcb);

	DWORD          (*DeviceSeek)(struct __COMMON_OBJECT*   lpDrv,
		                         struct __COMMON_OBJECT*   lpDev,
								 __DRCB*            lpDrcb);

	DWORD          (*DeviceOpen)(struct __COMMON_OBJECT*   lpDrv,
		                         struct __COMMON_OBJECT*   lpDev,
								 __DRCB*            lpDrcb);

	DWORD          (*DeviceClose)(struct __COMMON_OBJECT*  lpDrv,
		                          struct __COMMON_OBJECT*  lpDev,
								  __DRCB*           lpDrcb);

	DWORD          (*DeviceCreate)(struct __COMMON_OBJECT*  lpDrv,
		                           struct __COMMON_OBJECT*  lpDev,
								   __DRCB*           lpDrcb);

	DWORD          (*DeviceDestroy)(struct __COMMON_OBJECT* lpDrv,
		                            struct __COMMON_OBJECT* lpDev,
									__DRCB*          lpDrcb);

END_DEFINE_OBJECT() //End definition of __DRIVER_OBJECT.

//
//Initialize routine and Uninitialize routine's definition.
//

BOOL DrvObjInitialize(struct __COMMON_OBJECT*);
VOID DrvObjUninitialize(struct __COMMON_OBJECT*);

//
//The definition of __DEVICE_OBJECT.
//

BEGIN_DEFINE_OBJECT(__DEVICE_OBJECT)
    INHERIT_FROM_COMMON_OBJECT

	__DEVICE_OBJECT*            lpPrev;
    __DEVICE_OBJECT*            lpNext;

	UCHAR                       DevName[MAX_DEV_NAME_LEN + 1];
    //DWORD                       dwDevType;
	DWORD                       dwAttribute;
	DWORD                       dwBlockSize;
	DWORD                       dwMaxReadSize;
	DWORD                       dwMaxWriteSize;

	__DRIVER_OBJECT*            lpDriverObject;    //Point back to this device's driver.
	//DWORD                       dwRefCounter;      //Reference counter,record how many
	                                               //kernel thread open this device object.

	DWORD                       dwStartPort;       //Start port of this device used.
	DWORD                       dwEndPort;         //End port of this device used.

	DWORD                       dwDmaChannel;      //DMA channel number is device used.
	DWORD                       dwInterrupt;       //Interrupt vector number.

	LPVOID                      lpMemoryStartAddr; //Start address of the device mapped to
	                                               //system memory space.
	DWORD                       dwMemLen;

	//DWORD                       DevExtension[1];   //Device's extension.
	LPVOID                      lpDevExtension;
END_DEFINE_OBJECT()     //__DEVICE_OBJECT.

//
//Initialize routine and Uninitialize routine's definition.
//

BOOL DevObjInitialize(struct __COMMON_OBJECT*);
VOID DevObjUninitialize(struct __COMMON_OBJECT*);

//
//Device type definition.
//

#define DEVICE_TYPE_NORMAL             0x00000000         //Normal devices.
#define DEVICE_TYPE_FILE_SYSTEM        0x00000001         //File system devices.
#define DEVICE_TYPE_STORAGE            0x00000002         //Storage devices.
#define DEVICE_TYPE_FILE               0x00000004         //File.

//
//Driver entry routine.
//All device drivers must implement this routine,this routine is the entry point
//of a device driver.
//
typedef BOOL (*__DRIVER_ENTRY)(__DRIVER_OBJECT*);

//
//The following is the definition of __IO_MANAGER.
//This object is one of the core object in Hello Taiwan,it is used to manage all device(s) 
//and device driver(s) in the system,and it also supplies interface to user kernel thread 
//to access system device.
//

BEGIN_DEFINE_OBJECT(__IO_MANAGER)
    __DEVICE_OBJECT*               lpDeviceRoot;      //Pointes to device object's list.
    __DRIVER_OBJECT*               lpDriverRoot;      //Pointes to driver object's list.

	__RESOURCE_DESCRIPTOR*         lpResDescriptor;   //Pointes to resource descriptor's list.

	BOOL                 (*Initialize)(struct __COMMON_OBJECT*    lpThis);  //Initialize routine.

	//
	//The following routines are implemented to user kernel thread.
	//
	struct __COMMON_OBJECT*     (*CreateFile)(struct __COMMON_OBJECT*    lpThis,
		                               LPSTR               lpszFileName,
									   DWORD               dwAccessMode,
									   DWORD               dwShareMode,
									   LPVOID              lpReserved);  //CreateFile routine.

	BOOL                 (*ReadFile)(struct __COMMON_OBJECT*   lpThis,
		                             struct __COMMON_OBJECT*   lpFileObject,
									 DWORD              dwByteSize,
									 LPVOID             lpBuffer,
									 DWORD*             lpReadSize);

	BOOL                 (*WriteFile)(struct __COMMON_OBJECT*  lpThis,
		                              struct __COMMON_OBJECT*  lpFileObject,
									  DWORD             dwWriteSize,
									  LPVOID            lpBuffer,
									  DWORD*            lpWrittenSize);

	VOID                 (*CloseFile)(struct __COMMON_OBJECT*  lpThis,
		                              struct __COMMON_OBJECT*  lpFileObject);

	BOOL                 (*IOControl)(struct __COMMON_OBJECT*  lpThis,
		                              struct __COMMON_OBJECT*  lpFileObject,
									  DWORD             dwCommand,
									  LPVOID            lpParameter,
									  DWORD             dwOutputBufLen,
									  LPVOID            lpOutBuffer,
									  DWORD*            lpdwOutFilled);

	BOOL                 (*SetFilePointer)(struct __COMMON_OBJECT*  lpThis,
		                                   struct __COMMON_OBJECT*  lpFileObject,
										   DWORD             dwWhereBegin,
										   INT               nOffSet);

	BOOL                 (*FlushFile)(struct __COMMON_OBJECT*  lpThis,
		                              struct __COMMON_OBJECT*  lpFileObject);

	//The following routines are called by device driver(s).

	__DEVICE_OBJECT*     (*CreateDevice)(struct __COMMON_OBJECT*  lpThis,
		                                 LPSTR             lpszDevName,
										 DWORD             dwAttribute,
										 DWORD             dwBlockSize,
										 DWORD             dwMaxReadSize,
										 DWORD             dwMaxWriteSize,
										 LPVOID            lpDevExtension,
										 __DRIVER_OBJECT*  lpDrvObject);

	VOID                 (*DestroyDevice)(struct __COMMON_OBJECT* lpThis,
		                                  __DEVICE_OBJECT* lpDevObj);

	BOOL                 (*ReserveResource)(struct __COMMON_OBJECT*  lpThis,
		                                    __RESOURCE_DESCRIPTOR*
											                  lpResDesc);
	BOOL                 (*LoadDriver)(__DRIVER_ENTRY DrvEntry);

END_DEFINE_OBJECT()    //End of __IO_MANAGER.

//
//The following macros are used by CreateFile.
//

#define FILE_ACCESS_READ         0x00000001    //Read access.
#define FILE_ACCESS_WRITE        0x00000002    //Write access.
#define FILE_ACCESS_READWRITE    0x00000003    //Read and write access.
#define FILE_ACCESS_CREATE       0x00000004    //Create a new file.

//
//The following macros are used by SetFilePointer routine.
//

#define SET_FILE_POINTER_FROM_BEGIN        0x00000001
#define SET_FILE_POINTER_FROM_CURRENT      0x00000002

/*************************************************************************
**************************************************************************
**************************************************************************
**************************************************************************
*************************************************************************/

//
//The declaration of IOManager object.
//

extern __IO_MANAGER IOManager;

#endif    //iomgr.h
