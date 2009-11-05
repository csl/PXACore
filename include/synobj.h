//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Nov 04, 2004
//    Module Name               : synobj.h
//    Module Funciton           : 
//                                This module countains synchronization object's definition
//                                code.
//                                The following synchronization object(s) is(are) defined
//                                in this file:
//                                  1. EVENT
//                                  2. MUTEX
//                                  3. SEMAPHORE
//                                  4. TIMER
//
//                                ************
//                                This file is the most important file of Hello Taiwan.
//                                ************
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __SYNOBJ_H__
#define __SYNOBJ_H__
#endif

//
//Common synchronization object's definition.
//The common synchronization object is a abstract object,all synchronization objects,such
//as event,mutex,etc,all inherited from this object.
//

BEGIN_DEFINE_OBJECT(__COMMON_SYNCHRONIZATION_OBJECT)
    DWORD                (*WaitForThisObject)(__COMMON_SYNCHRONIZATION_OBJECT*);
END_DEFINE_OBJECT()

//
//The following macro is defined to be used by other objects to inherit from
//common synchronization object.
//
#define INHERIT_FROM_COMMON_SYNCHRONIZATION_OBJECT \
	DWORD                (*WaitForThisObject)(struct __COMMON_OBJECT*);

//
//Event object's definition.
//The event object is inherited from common object and common synchronization object.
//

BEGIN_DEFINE_OBJECT(__EVENT)
    INHERIT_FROM_COMMON_OBJECT
	INHERIT_FROM_COMMON_SYNCHRONIZATION_OBJECT
	DWORD                 dwEventStatus;
    	struct __PRIORITY_QUEUE*     lpWaitingQueue;
	DWORD                 (*SetEvent)(struct __COMMON_OBJECT*);
	DWORD                 (*ResetEvent)(struct __COMMON_OBJECT*);
	DWORD                 (*WaitForThisObjectEx)(struct __COMMON_OBJECT*,
		                                         DWORD);    //Time out waiting operation.
END_DEFINE_OBJECT()

#define EVENT_STATUS_FREE            0x00000001    //Event status.
#define EVENT_STATUS_OCCUPIED        0x00000002

//
//The following values are returned by WaitForThisObjectEx.
//
#define EVENT_WAIT_FAILED            0x00000000
#define EVENT_WAIT_RESOURCE          0x00000001
#define EVENT_WAIT_TIMEOUT           0x00000002

BOOL EventInitialize(struct __COMMON_OBJECT*);            //The event object's initializing routine
VOID EventUninitialize(struct __COMMON_OBJECT*);          //and uninitializing routine.

//
//The following routines are used by kernel thread to create event or destroy event.
//

struct __COMMON_OBJECT*    CreateEvent(BOOL bInitialState,LPVOID lpReserved);
VOID  DestroyEvent(struct __COMMON_OBJECT* lpEvent);

//--------------------------------------------------------------------------------------
//
//                                MUTEX
//
//---------------------------------------------------------------------------------------

//
//The definition of MUTEX object.
//

BEGIN_DEFINE_OBJECT(__MUTEX)
    INHERIT_FROM_COMMON_OBJECT                  //Inherit from struct __COMMON_OBJECT.
    INHERIT_FROM_COMMON_SYNCHRONIZATION_OBJECT  //Inherit from common synchronization object.
    DWORD             dwMutexStatus;
    DWORD             (*ReleaseMutex)(struct __COMMON_OBJECT* lpThis);
END_DEFINE_OBJECT()

#define MUTEX_STATUS_FREE      0x00000001
#define MUTEX_STATUS_OCCUPIED  0x00000002

//
//The initializing routine of MUTEX object and uninitializing routine.
//

BOOL MutexInitialize(struct __COMMON_OBJECT* lpThis);
VOID MutexUninitialize(struct __COMMON_OBJECT* lpThis);

//
//The following global routines are used to operate mutex object,including create a mutex
//and destroy a mutex object.
//

struct __COMMON_OBJECT*    CreateMutex(LPVOID lpReserved);
VOID                DestroyMutex(struct __COMMON_OBJECT* lpMutexObject);


//-----------------------------------------------------------------------------------
//
//                            SEMAPHORE
//
//-----------------------------------------------------------------------------------

