/***********************************************************************/
//    Author                    : Garry
//    Original Date             : Aug,28 2005
//    Module Name               : COMQUEUE.H
//    Module Funciton           : 
//                                This module countains common queue's definition.
//                                The common queue is a FIFO queue,the queue element can
//                                be anything,i.e,the type is not limited.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __COMQUEUE_H__
#define __COMQUEUE_H__

//
//The definition of common queue element.
//
BEGIN_DEFINE_OBJECT(__COMMON_QUEUE_ELEMENT)
    struct __COMMON_QUEUE_ELEMENT*        lpNext;
    struct __COMMON_QUEUE_ELEMENT*        lpPrev;
    LPVOID                         lpObject;
END_DEFINE_OBJECT()

//
//This macro used to initialize a common queue element.
//
#define INIT_COMMON_QUEUE_ELEMENT(ele) \
	(ele)->lpNext    = (ele);          \
	(ele)->lpPrev    = (ele);          \
	(ele)->lpObject  = NULL;

//
//The following macros are used to allocate queue element or destroy
//queue element.
//
#define ALLOCATE_QUEUE_ELEMENT() \
	(struct __COMMON_QUEUE_ELEMENT*)KMemAlloc(sizeof(struct __COMMON_QUEUE_ELEMENT), \
	KMEM_SIZE_TYPE_ANY);

#define FREE_QUEUE_ELEMENT(eleptr) \
	KMemFree((LPVOID)(eleptr),KMEM_SIZE_TYPE_ANY,0L);

//
//The definition of common queue.
//This kind of queue is used to contain any type object.
//
BEGIN_DEFINE_OBJECT(__COMMON_QUEUE)
    INHERIT_FROM_COMMON_OBJECT
	struct __COMMON_QUEUE_ELEMENT             QueueHdr;          //Queue's header.
	DWORD                              dwQueueLen;        //Queue length.
	DWORD                              dwCurrentLen;      //Current length.

	BOOL                               (*InsertIntoQueue)(struct __COMMON_OBJECT* lpThis,
		                                                  LPVOID lpObject);
	LPVOID                             (*GetFromQueue)(struct __COMMON_OBJECT* lpThis);
	BOOL                               (*QueueEmpty)(struct __COMMON_OBJECT* lpThis);
	BOOL                               (*QueueFull)(struct __COMMON_OBJECT* lpThis);
	DWORD                              (*SetQueueLength)(struct __COMMON_OBJECT* lpThis,
		                                              DWORD dwNewLen);
	DWORD                              (*GetQueueLength)(struct __COMMON_OBJECT* lpThis);
	DWORD                              (*GetCurrLength)(struct __COMMON_OBJECT* lpThis);
END_DEFINE_OBJECT()

#define DEFAULT_COMMON_QUEUE_LEN  32   //The default length of a common queue object.

//
//The definition of CommQueueInitialize routine.
//This routine is used to initialize common queue object.
//
BOOL CommQueueInit(struct __COMMON_OBJECT* lpThis);

//
//The definition of CommQueueUninitialize routine.
//
VOID CommQueueUninit(struct __COMMON_OBJECT* lpThis);

#endif  //End of COMQUEUE.H

