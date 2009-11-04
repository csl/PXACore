//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct,18 2004
//    Module Name               : objqueue.cpp
//    Module Funciton           : 
//                                This module countains Object Queue's implementation code.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include "stdafx.h"

//
//Insert an element into Priority Queue.
//This routine insert an common object into priority queue,it's position in the queue is
//determined by the object's priority(dwPriority parameter).
//

static BOOL InsertIntoQueue(struct __COMMON_OBJECT* lpThis,struct __COMMON_OBJECT* lpObject,DWORD dwPriority)
{
    if((NULL == lpThis) || (NULL == lpObject)) //Invalid parameters.
    {
        return FALSE;
    }

    struct __PRIORITY_QUEUE_ELEMENT* lpElement = NULL;
    struct __PRIORITY_QUEUE_ELEMENT* lpTmpElement = NULL;
    struct __PRIORITY_QUEUE*         lpQueue   = (struct __PRIORITY_QUEUE*)lpThis;
    DWORD                     dwFlags   = 0L;
    
    //Allocate a queue element,and initialize it.
    lpElement = (struct __PRIORITY_QUEUE_ELEMENT*)
		KMemAlloc(sizeof(struct __PRIORITY_QUEUE_ELEMENT),KMEM_SIZE_TYPE_ANY);
    if(NULL == lpElement)  //Can not allocate the memory.
    {
        return FALSE;
    }

    lpElement->lpObject       = lpObject;
    lpElement->dwPriority     = dwPriority;

    //Now,insert the element into queue list.
    __ENTER_CRITICAL_SECTION(NULL,dwFlags);  //Atomic operation.
    __BARRIER(NULL);  //Barrier operation.
    lpQueue->dwCurrElementNum ++;  //Increment element number.
    lpTmpElement = lpQueue->ElementHeader.lpPrevElement;

    //Find the appropriate position according to priority to insert.
    while((lpTmpElement->dwPriority < dwPriority) &&
          (lpTmpElement != &lpQueue->ElementHeader))
    {
        lpTmpElement = lpTmpElement->lpPrevElement;
    }
    //Insert the element into list.
    lpElement->lpNextElement   = lpTmpElement->lpNextElement;
    lpElement->lpPrevElement   = lpTmpElement;
    lpTmpElement->lpNextElement->lpPrevElement = lpElement;
    lpTmpElement->lpNextElement = lpElement;
    __BARRIER(NULL);  //Flush CPU's internal content to memory.
    __LEAVE_CRITICAL_SECTION(NULL,dwFlags);

    return TRUE;

}

//
//Delete an element from Priority Queue.
//This routine searchs the queue,to find the object to be deleted,
//if find,delete the object from priority queue,returns TRUE,else,
//returns FALSE.
//If the object is inserted into this queue for many times,this
//operation only deletes one time.
//

static BOOL DeleteFromQueue(struct __COMMON_OBJECT* lpThis,struct __COMMON_OBJECT* lpObject)
{
    if((NULL == lpThis) || (NULL == lpObject))  //Invalid parameters.
    {
        return FALSE;
    }

    struct __PRIORITY_QUEUE*         lpQueue = (struct __PRIORITY_QUEUE*)lpThis;
    struct __PRIORITY_QUEUE_ELEMENT* lpElement = NULL;
    DWORD                     dwFlags;

    __ENTER_CRITICAL_SECTION(NULL,dwFlags);
    __BARRIER(NULL);
	lpElement = lpQueue->ElementHeader.lpNextElement;

    while((lpElement->lpObject != lpObject) && (lpElement != &lpQueue->ElementHeader))
    {
        lpElement = lpElement->lpNextElement;
    }
    if(lpObject == lpElement->lpObject)  //Found,delete it.
    {
        lpQueue->dwCurrElementNum --;
        lpElement->lpNextElement->lpPrevElement = lpElement->lpPrevElement;
        lpElement->lpPrevElement->lpNextElement = lpElement->lpNextElement;
        __BARRIER(NULL);  //Commit the change.
        __LEAVE_CRITICAL_SECTION(NULL,dwFlags);
        KMemFree(lpElement,KMEM_SIZE_TYPE_ANY,0L);  //Free memory.
        return TRUE;
    }
    //Not found the target object to delete.
    __LEAVE_CRITICAL_SECTION(NULL,dwFlags);  //No need barrier.
    return FALSE;
}

//
//Get the header element from Priority Queue.
//This routine get the first(header) object of the priority queue,
//and release the memory this element occupies.
//

static struct __COMMON_OBJECT* GetHeaderElement(struct __COMMON_OBJECT* lpThis,DWORD* lpdwPriority)
{
	if(NULL == lpThis)
    {
        return FALSE;
    }

    struct __PRIORITY_QUEUE*          lpQueue = (struct __PRIORITY_QUEUE*)lpThis;
    struct __PRIORITY_QUEUE_ELEMENT*  lpElement = NULL;
    struct __COMMON_OBJECT*           lpCommObject = NULL;
    DWORD                      dwFlags;

    __ENTER_CRITICAL_SECTION(NULL,dwFlags);
    __BARRIER(NULL);
	lpElement = lpQueue->ElementHeader.lpNextElement;

    if(lpElement == &lpQueue->ElementHeader)  //Queue empty.
    {
        __LEAVE_CRITICAL_SECTION(NULL,dwFlags);
        return NULL;
    }

    //Queue not empty,delete this element.
    lpQueue->dwCurrElementNum -= 1;
    lpElement->lpNextElement->lpPrevElement = lpElement->lpPrevElement;
    lpElement->lpPrevElement->lpNextElement = lpElement->lpNextElement;
    __BARRIER(NULL);
    __LEAVE_CRITICAL_SECTION(NULL,dwFlags);

    lpCommObject = lpElement->lpObject;
    if(lpdwPriority)  //Should return the priority value.
    {
        *lpdwPriority = lpElement->dwPriority;
    }
    KMemFree(lpElement,KMEM_SIZE_TYPE_ANY,0L);  //Release the element.

    return lpCommObject;
}

//Initialize routine of the Priority Queue.
BOOL PriQueueInitialize(struct __COMMON_OBJECT* lpThis)
{
	struct __PRIORITY_QUEUE*                lpPriorityQueue     = NULL;

	if(NULL == lpThis)           //Parameter check.
	{
		return FALSE;
	}

	//Initialize the Priority Queue.
	lpPriorityQueue = (struct __PRIORITY_QUEUE*)lpThis;
	lpPriorityQueue->ElementHeader.lpObject = NULL;
	lpPriorityQueue->ElementHeader.dwPriority = 0L;
	lpPriorityQueue->ElementHeader.lpNextElement = &lpPriorityQueue->ElementHeader;
	lpPriorityQueue->ElementHeader.lpPrevElement = &lpPriorityQueue->ElementHeader;
	lpPriorityQueue->dwCurrElementNum = 0L;
	lpPriorityQueue->InsertIntoQueue  = InsertIntoQueue;
	lpPriorityQueue->DeleteFromQueue  = DeleteFromQueue;
	lpPriorityQueue->GetHeaderElement = GetHeaderElement;

	return TRUE;
}

//
//Uninitialize routine of Priority Queue.
//This routine frees all memory this priority queue occupies.
//

VOID PriQueueUninitialize(struct __COMMON_OBJECT* lpThis)
{
	struct __PRIORITY_QUEUE_ELEMENT*    lpElement    = NULL;
	struct __PRIORITY_QUEUE_ELEMENT*    lpTmpElement   = NULL;
	struct __PRIORITY_QUEUE*            lpPriorityQueue   = NULL;

	if(NULL == lpThis)
	{
		return;
	}

	lpPriorityQueue = (struct __PRIORITY_QUEUE*)lpThis;
	lpElement = lpPriorityQueue->ElementHeader.lpNextElement;
	//Delete all queue element(s).
	while(lpElement != &lpPriorityQueue->ElementHeader)
	{
		lpTmpElement = lpElement;
		lpElement = lpElement->lpNextElement;
		KMemFree(lpTmpElement,KMEM_SIZE_TYPE_ANY,0L);
	}
}
