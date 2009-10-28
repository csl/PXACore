//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct,15 2005
//    Module Name               : DEVMGR.H
//    Module Funciton           : 
//                                This module countains device manager object's definition
//                                code.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __DEVMGR_H__
#define __DEVMGR_H__
#endif

#define MAX_BUS_NUM        16    //The maximal bus number supported by Hello Taiwan currently.
#define MAX_RESOURCE_NUM    7    //Maximal resource records one device can have.
#define MAX_DEV_NAME       32    //The maximal length of device's name.

//
//The definition of device identifier.
//

//BEGIN_DEFINE_OBJECT(__IDENTIFIER)
struct __IDENTIFIER{
	DWORD             dwBusType;
	union{
		struct{
			UCHAR     ucMask;
			WORD      wVendor;
			WORD      wDevice;
			DWORD     dwClass;
			UCHAR     ucHdrType;
			WORD      wReserved;
		}PCI_Identifier;            //PCI interface device's identifier.

		struct{
			DWORD     dwDevice;
		}ISA_Identifier;           //ISA bus device's identifier.
		//
		//Add other bus device's identifier here.
		//
	};
//END_DEFINE_OBJECT()
};

//
//The definition of resource descriptor object.
//

//BEGIN_DEFINE_OBJECT(__RESOURCE)
struct __RESOURCE{
	__RESOURCE*         lpNext;
	__RESOURCE*         lpPrev;
	DWORD               dwResType;        //Resource type.
	union{
		struct{
			WORD        wStartPort;
			WORD        wEndPort;
		}IOPort;                          //IO Port resource descriptor.

		struct{
			LPVOID      lpStartAddr;
			LPVOID      lpEndAddr;
		}MemoryRegion;                    //Memory region resource descriptor.

		UCHAR ucVector;                   //Interrupt vector.
	};
//END_DEFINE_OBJECT()
};

#define RESOURCE_TYPE_EMPTY      0x00000000    //No resource in resource descriptor object.
#define RESOURCE_TYPE_IO         0x00000001    //IO Port region resides in resource descriptor.
#define RESOURCE_TYPE_MEMORY     0x00000002    //Memory region resides in descriptor.
#define RESOURCE_TYPE_INTERRUPT  0x00000004    //Interrupt vector resides in descriptor.

#define MIN_IO_PORT              0x0000  //The minimal value of IO port in IA32.
#define MAX_IO_PORT              0xFFFF  //The maximal value of IO port in IA32.

//PREDECLARE_OBJECT_TYPE(__PHYSICAL_DEVICE);
//PREDECLARE_OBJECT_TYPE(__SYSTEM_BUS);
struct __PHYSICAL_DEVICE;
struct __SYSTEM_BUS;

//
//The definition of physical device object.
//This object is used to describe a abstract object resides on bus.
//

//BEGIN_DEFINE_OBJECT(__PHYSICAL_DEVICE)
struct __PHYSICAL_DEVICE{
	__IDENTIFIER                  DevId;                  //Device's identifier.
	UCHAR                          strName[MAX_DEV_NAME];  //Device's name.
	__RESOURCE                    Resource[MAX_RESOURCE_NUM];  //Resource descripotrs.
	__PHYSICAL_DEVICE*            lpNext;
	//__PHYSICAL_DEVICE*            lpPrev;
	__SYSTEM_BUS*                 lpHomeBus;      //Bus where the device resides.
	__SYSTEM_BUS*                 lpChildBus;     //Child bus,if the device is a bridge.
	LPVOID                        lpPrivateInfo;  //Pointing to a bus type specific structure
	                                              //used to describe specific bus.
//END_DEFINE_OBJECT()
};

//
//The definition of system bus object.
//This object is used to describe a common bus.
//

//BEGIN_DEFINE_OBJECT(__SYSTEM_BUS)
struct __SYSTEM_BUS{
	__SYSTEM_BUS*                lpParentBus;    //Parent bus.
	__PHYSICAL_DEVICE*           lpDevListHdr;   //List devices in this bus.
	__PHYSICAL_DEVICE*           lpHomeBridge;   //Bridge this bus resides.
	__RESOURCE                   Resource;       //Resource list header in this bus.

	DWORD                        dwBusNum;       //Bus number.
	DWORD                        dwBusType;      //Bus type.
//END_DEFINE_OBJECT()
};

#define BUS_TYPE_NULL            0x00000000
#define BUS_TYPE_PCI             0x00000001
#define BUS_TYPE_ISA             0x00000002
#define BUS_TYPE_EISA            0x00000004
#define BUS_TYPE_USB             0x00000008

//
//The definition of device manager object.
//This object is a global object,all physical device releated operations,are offered
//by this object.
//

//BEGIN_DEFINE_OBJECT(__DEVICE_MANAGER)
struct __DEVICE_MANAGER{
	__SYSTEM_BUS                 SystemBus[MAX_BUS_NUM];    //Bus array.
	__RESOURCE                   FreePortResource;          //Used to link free port region
	                                                        //together.
	__RESOURCE                   UsedPortResource;          //Links used port region together.

	BOOL                         (*Initialize)(__DEVICE_MANAGER*);    //Initialize routine.
	__PHYSICAL_DEVICE*           (*GetDevice)(__DEVICE_MANAGER*,
		                                      DWORD               dwBusType,
											  __IDENTIFIER*       lpIdentifier,
											  __PHYSICAL_DEVICE*  lpStart);

	BOOL                         (*AppendDevice)(__DEVICE_MANAGER*,
		                                         __PHYSICAL_DEVICE*);

	VOID                         (*DeleteDevice)(__DEVICE_MANAGER*,
		                                         __PHYSICAL_DEVICE*);
	
	BOOL                         (*CheckPortRegion)(__DEVICE_MANAGER*,
		                                            __RESOURCE*);

	BOOL                         (*ReservePortRegion)(__DEVICE_MANAGER*,
		                                              __RESOURCE*);

	VOID                         (*ReleasePortRegion)(__DEVICE_MANAGER*,
		                                              __RESOURCE*);

//END_DEFINE_OBJECT()
};

/****************************************************************************************
*****************************************************************************************
*****************************************************************************************
*****************************************************************************************
****************************************************************************************/
//
//Declaration of DeviceManager object.
//

extern __DEVICE_MANAGER DeviceManager;
