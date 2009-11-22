//***********************************************************************/
//    Author          : Garry
//    Original Date   : Oct,15 2005
//    Module Name     : DEVMGR.H
//    Module Funciton : 
//            This module countains device manager object's definition
//            code.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//            1.
//            2.
//    Lines number    :
//***********************************************************************/

#ifndef __DEVMGR_H__
#define __DEVMGR_H__
#endif

#define MAX_BUS_NUM        16    //The maximal bus number supported by Hello Taiwan currently.
#define MAX_RESOURCE_NUM    7    //Maximal resource records one device can have.
#define MAX_DEV_NAME       32    //The maximal length of device's name.

//The definition of device identifier.
//BEGIN_DEFINE_OBJECT(struct __IDENTIFIER)
struct __IDENTIFIER
{
	DWORD   dwBusType;

	union
	{
		struct
		{
			UCHAR     ucMask;
			WORD      wVendor;
			WORD      wDevice;
			DWORD     dwClass;
			UCHAR     ucHdrType;
			WORD      wReserved;
		} PCI_Identifier;  //USB interface device's identifier.

		struct
		{
			DWORD     dwDevice;
		} ISA_Identifier; //GPIO bus device's identifier.
		
		//Add other bus device's identifier here.
		
	};
//END_DEFINE_OBJECT()
};

//
//The definition of resource descriptor object.
//

//BEGIN_DEFINE_OBJECT(struct __RESOURCE)
struct __RESOURCE
{
	struct __RESOURCE*         lpNext;
	struct __RESOURCE*         lpPrev;

	DWORD  dwResType;        //Resource type.

	union
	{
		struct
		{
			WORD        wStartPort;
			WORD        wEndPort;
		}IOPort;      //IO Port resource descriptor.

		struct
		{
			LPVOID      lpStartAddr;
			LPVOID      lpEndAddr;
		}MemoryRegion;          //Memory region resource descriptor.

		UCHAR ucVector;         //Interrupt vector.
	};
//END_DEFINE_OBJECT()
};

#define RESOURCE_TYPE_EMPTY      0x00000000    //No resource in resource descriptor object.
#define RESOURCE_TYPE_IO         0x00000001    //IO Port region resides in resource descriptor.
#define RESOURCE_TYPE_MEMORY     0x00000002    //Memory region resides in descriptor.
#define RESOURCE_TYPE_INTERRUPT  0x00000004    //Interrupt vector resides in descriptor.

#define MIN_IO_PORT    0x0000  //The minimal value of IO port in IA32.
#define MAX_IO_PORT    0xFFFF  //The maximal value of IO port in IA32.

//PREDECLARE_OBJECT_TYPE(struct __PHYSICAL_DEVICE);
//PREDECLARE_OBJECT_TYPE(struct __SYSTEM_BUS);
struct __PHYSICAL_DEVICE;
struct __SYSTEM_BUS;

//
//The definition of physical device object.
//This object is used to describe a abstract object resides on bus.
//

//BEGIN_DEFINE_OBJECT(struct __PHYSICAL_DEVICE)
struct __PHYSICAL_DEVICE
{
	struct __IDENTIFIER DevId;        //Device's identifier.
	UCHAR     strName[MAX_DEV_NAME];  //Device's name.
	struct __RESOURCE  Resource[MAX_RESOURCE_NUM];  //Resource descripotrs.
	struct __PHYSICAL_DEVICE*          lpNext;
	//struct __PHYSICAL_DEVICE*        lpPrev;
	struct __SYSTEM_BUS*     lpHomeBus;      //Bus where the device resides.
	struct __SYSTEM_BUS*     lpChildBus;     //Child bus,if the device is a bridge.
	LPVOID      lpPrivateInfo;  //Pointing to a bus type specific structure
	      //used to describe specific bus.
//END_DEFINE_OBJECT()
};

//
//The definition of system bus object.
//This object is used to describe a common bus.
//

//BEGIN_DEFINE_OBJECT(struct __SYSTEM_BUS)
struct __SYSTEM_BUS
{
	struct __SYSTEM_BUS*          lpParentBus;    //Parent bus.
	struct __PHYSICAL_DEVICE*     lpDevListHdr;   //List devices in this bus.
	struct __PHYSICAL_DEVICE*     lpHomeBridge;   //Bridge this bus resides.
	struct __RESOURCE   Resource;       //Resource list header in this bus.

	DWORD     dwBusNum;       //Bus number.
	DWORD     dwBusType;      //Bus type.
//END_DEFINE_OBJECT()
};

#define BUS_TYPE_NULL  0x00000000
#define BUS_TYPE_PCI   0x00000001
#define BUS_TYPE_ISA   0x00000002
#define BUS_TYPE_EISA  0x00000004
#define BUS_TYPE_USB   0x00000008

//
//The definition of device manager object.
//This object is a global object,all physical device releated operations,are offered
//by this object.
//

//BEGIN_DEFINE_OBJECT(struct __DEVICE_MANAGER)
struct __DEVICE_MANAGER
{
	struct __SYSTEM_BUS  SystemBus[MAX_BUS_NUM];    //Bus array.

	struct __RESOURCE    FreePortResource;          //Used to link free port region together.
	struct __RESOURCE    UsedPortResource;          //Links used port region together.

	BOOL     (*Initialize)(struct __DEVICE_MANAGER*);    //Initialize routine.

	struct __PHYSICAL_DEVICE*   (*GetDevice)(struct __DEVICE_MANAGER*, DWORD dwBusType, 
					struct __IDENTIFIER*  lpIdentifier, 
					struct __PHYSICAL_DEVICE*  lpStart);

	BOOL     (*AppendDevice)(struct __DEVICE_MANAGER*,
		           struct __PHYSICAL_DEVICE*);

	VOID     (*DeleteDevice)(struct __DEVICE_MANAGER*,
		           struct __PHYSICAL_DEVICE*);
	
	BOOL     (*CheckPortRegion)(struct __DEVICE_MANAGER*,
		    struct __RESOURCE*);

	BOOL     (*ReservePortRegion)(struct __DEVICE_MANAGER*,
		      struct __RESOURCE*);

	VOID     (*ReleasePortRegion)(struct __DEVICE_MANAGER*,
		      struct __RESOURCE*);

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

extern struct __DEVICE_MANAGER DeviceManager;
