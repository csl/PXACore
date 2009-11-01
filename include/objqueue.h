//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Sep,03 2004
//    Module Name               : objqueue.h
//    Module Funciton           : 
//                                This module countains Object Queue structure's definition.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __OBJQUEUE_H__
#define __OBJQUEUE_H__

//Priority Queue's element.

BEGIN_DEFINE_OBJECT(__PRIORITY_QUEUE_ELEMENT)
    struct __COMMON_OBJECT*          lpObject;
    DWORD                     dwPriority;
    struct __PRIORITY_QUEUE_ELEMENT* lpNextElement;
    struct __PRIORITY_QUEUE_ELEMENT* lpPrevElement;
END_DEFINE_OBJECT()

//The definition of Priority Queue.

BEGIN_DEFINE_OBJECT(__PRIORITY_QUEUE)
    INHERIT_FROM_COMMON_OBJECT                            //Inherit from __COMMON_OBJECT.
    	struct __PRIORITY_QUEUE_ELEMENT     ElementHeader;
	DWORD                        dwCurrElementNum;
	BOOL  (*InsertIntoQueue)(struct __COMMON_OBJECT* lpThis,  
				  struct __COMMON_OBJECT* lpObject, DWORD  dwPriority);
	BOOL  (*DeleteFromQueue)(struct __COMMON_OBJECT* lpThis, struct __COMMON_OBJECT* lpObject);
	struct __COMMOfN_OBJECT*  (*GetHeaderElement)(struct __COMMON_OBJECT* lpThis, DWORD* lpPriority);
END_DEFINE_OBJECT()

//Initialize routine and Uninitialize's definition.

BOOL PriQueueInitialize(struct __COMMON_OBJECT* lpThis);
VOID PriQueueUninitialize(struct __COMMON_OBJECT* lpThis);

#endif
