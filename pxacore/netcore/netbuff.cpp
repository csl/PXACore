//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Aug,06 2005
//    Module Name               : NETBUFF.CPP
//    Module Funciton           : 
//                                This module countains net buffer's
//                                implementation code.
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

#ifndef __DEFINES_H__
#include "DEFINES.H"
#endif

#ifndef __NETBUFF_H__
#include "NETBUFF.H"
#endif

/*--------------- ** Some important hints about net buffer object ** -------------------
  The net buffer object is used by network entity,such as IP kernel thread,net interface
card,to store data or packets.When you use the net buffer object,the following rules must
be obeied:
  1. When you transfer a net buffer object as a parameter to another kernel thread,you 
     must first increment the reference counter by calling NetBufferGet routine;
  2. When you finished to use a net buffer object,you must call NetBufferFree routine to
     release the memory or decrement the reference counter,if the net buffer object is created
	 by yourself,if the object is not created by your self,but is transfered by another 
	 kernel thread,you must call NetBufferFree routine too;
  3. All net buffer operations are interrupt safe,so you can call the operations anywhere.
---------------------------------------------------------------------------------------*/

//
//The implementation of net buffer queue.
//This is a FIFO queue,can be used by any entity,such as IP kernel thread.
//

//
//The implementation of InsertIntoQueue.
//This routine adds the net buffer object to the end of the queue.
//If the current element number is not exceed the max number,then insert it,and
//returns TRUE,otherwise,returns FALSE.
//

static BOOL InsertIntoQueue(__NET_BUFFER_QUEUE* lpQueue,__NET_BUFFER* lpNetBuffer)
{
	DWORD                   dwFlags        = 0L;

	__ENTER_CRITICAL_SECTION(NULL,dwFlags);  //The following operation should not be
	                                         //interrupted.
	if(lpQueue->dwQueueNum == lpQueue->dwMaxNum)  //The queue is full.
	{
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		return FALSE;
	}
	//
	//The following code insert the net buffer object into queue's tail.
	//
	if(NULL == lpQueue->lpQueueTail)    //There is not any element in the queue.
	{
		lpQueue->lpQueueHdr  = lpNetBuffer;
		lpQueue->lpQueueTail = lpNetBuffer;
		lpNetBuffer->lpNext  = NULL;
		lpNetBuffer->lpPrev  = NULL;
		lpQueue->dwQueueNum ++;
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		return TRUE;
	}
	else    //The queu is not empty.
	{
		lpQueue->lpQueueTail->lpNext  = lpNetBuffer;
		lpNetBuffer->lpPrev           = lpQueue->lpQueueTail;
		lpNetBuffer->lpNext           = NULL;
		lpQueue->lpQueueTail          = lpNetBuffer;
		lpQueue->dwQueueNum ++;
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		return TRUE;
	}
	return TRUE;
}

//
//The implementation of DeleteFromQueue.
//This routine first check if the object is in the queue,if not,returns FALSE,
//else,deletes it and returns TRUE.
//

static BOOL DeleteFromQueue(__NET_BUFFER_QUEUE*,__NET_BUFFER*)
{
	BOOL                    bResult             = FALSE;
	return bResult;
}

//
//The implementation of GetFromQueue.This routine gets the first element of this queue.
//

static __NET_BUFFER* GetFromQueue(__NET_BUFFER_QUEUE* lpQueue)
{
	__NET_BUFFER*                 lpNetBuffer         = NULL;
	DWORD                         dwFlags             = 0L;

	if(NULL == lpQueue)    //Parameter check.
		return NULL;
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	if(NULL == lpQueue->lpQueueHdr)    //The current queue is empty.
	{
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		return NULL;
	}
	if(lpQueue->lpQueueHdr == lpQueue->lpQueueTail)  //There is only one element in the queue.
	{
		lpNetBuffer = lpQueue->lpQueueHdr;
		lpQueue->lpQueueHdr  = NULL;
		lpQueue->lpQueueTail = NULL;
		lpNetBuffer->lpPrev  = NULL;
		lpNetBuffer->lpNext  = NULL;
		lpQueue->dwQueueNum--;
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		return lpNetBuffer;
	}
	else    //There are at least two elements in the current queue.
	{
		lpNetBuffer = lpQueue->lpQueueHdr;
		lpQueue->lpQueueHdr = lpNetBuffer->lpNext;
		lpQueue->lpQueueHdr->lpPrev = NULL;
		lpQueue->dwQueueNum --;
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		return lpNetBuffer;
	}
	return NULL;    //This instruction should never be executed.
}

//
//The implementation of GetQueueLength.
//

static DWORD GetQueueLen(__NET_BUFFER_QUEUE* lpQueue)
{
	if(NULL == lpQueue)
		return 0L;
	return lpQueue->dwQueueNum;
}

//
//The implementation of SetMaxLen.
//

static VOID SetMaxLen(__NET_BUFFER_QUEUE* lpQueue,DWORD dwMaxLen)
{
	DWORD             dwFlags             = 0L;

	if(NULL == lpQueue)    //Parameter check.
		return;
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	lpQueue->dwMaxNum = dwMaxLen;
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	return;
}

//
//The implementation of Initialize.
//

static VOID QueueInitialize(__NET_BUFFER_QUEUE* lpQueue)
{
	if(NULL == lpQueue)
		return;

	lpQueue->lpQueueHdr        = NULL;
	lpQueue->lpQueueTail       = NULL;
	lpQueue->dwQueueNum        = 0L;
	lpQueue->dwMaxNum          = 0L;

	lpQueue->Initialize        = QueueInitialize;
	lpQueue->SetMaxLen         = SetMaxLen;
	lpQueue->GetQueueLen       = GetQueueLen;
	lpQueue->GetFromQueue      = GetFromQueue;
	lpQueue->DeleteFromQueue   = DeleteFromQueue;
	lpQueue->InsertIntoQueue   = InsertIntoQueue;

	return;
}

BOOL NetBufferQueueInit(__NET_BUFFER_QUEUE* lpQueue)    //Global routine.
{
	if(NULL == lpQueue)
		return FALSE;
	QueueInitialize(lpQueue);
	return TRUE;
}

//--------------------------------------------------------------------------------------

//
//The implementation of net buffer manager.
//

//
//Pre-declaration of routines.
//
static VOID NetBufferFreeData(__NET_BUFFER_MANAGER*,__NET_BUFFER*);

//
//A helper routine,used to get a free net buffer from free list.
//
static __NET_BUFFER* __GetFreeNetBuffer(__NET_BUFFER_MANAGER* lpMgr)
{
	__NET_BUFFER*                     lpNetBuffer           = NULL;
	
	lpNetBuffer = ALLOC_BUFF_MEMORY(sizeof(__NET_BUFFER));
	if(NULL == lpNetBuffer)
		return NULL;

#define SET(member,value) lpNetBuffer->member = value    //This macro initializes a member.

	SET(lpPrev,          NULL);
	SET(lpNext,          NULL);
	SET(lpSocket,        NULL);
	SET(Time,            NULL);
	SET(lpNetDevice,     NULL);
	SET(TransLayerHdr.lpTransLayerData, NULL);
	SET(NetLayerHdr.lpNetLayerData,     NULL);
	SET(LinkLayerHdr.lpLinkLayerData,   NULL);
	SET(dwDataLen,       0L);
	SET(dwTotalLen,      0L);
	SET(dwCheckSum,      0L);
	SET(dwPktType,       PACKET_TYPE_UNKNOWN);
	SET(bCloned,         FALSE);
	SET(ucProtocol,      PACKET_PROTOCOL_UNKNOWN);
	SET(ucReserved1,     0);
	SET(ucReserved2,     0);
	SET(ucReserved3,     0);
	SET(lpBufferHdr,     NULL);
	SET(lpBufferEnd,     NULL);
	SET(lpDataHdr,       NULL);
	SET(lpDataEnd,       NULL);

#undef SET    //Finish to initialize the net buffer allocated just now.
	__INIT_ATOMIC(lpNetBuffer->Users);
	AtomicSet(&lpNetBuffer->Users,1);
	return lpNetBuffer;
}

//
//A helper routine,used to free a net buffer object.
//

static VOID __FreeNetBuffer(__NET_BUFFER_MANAGER* lpMgr,__NET_BUFFER* lpNetBuffer)
{
	if(NULL == lpNetBuffer)  //Parameter check.
		return;
	FREE_BUFF_MEMORY(lpNetBuffer);    //Release the net buffer memory.
	return;
}

//
//The implementation of NetBufferAlloc.
//

static __NET_BUFFER* NetBufferAlloc(__NET_BUFFER_MANAGER* lpManager,DWORD dwBuffLen)
{
	__NET_BUFFER*                   lpNetBuffer                     = NULL;
	DWORD                           dwFlags                         = 0L;
	UCHAR*                          lpDataBuff                      = NULL;
	__FRAG_DATA*                    lpFragData                      = NULL;

	if((NULL == lpManager) || (0 == dwBuffLen)) //Parameter check.
		return NULL;
	lpNetBuffer = __GetFreeNetBuffer(lpManager);
	if(NULL == lpNetBuffer)    //Can not get a net buffer object,no memory may be the 
		                       //most reasonable reason.
		return NULL;

	lpDataBuff = ALLOC_DATA_MEMORY(dwBuffLen);
	if(NULL == lpDataBuff)     //Can not allocate memory to hold data.
	{
		__FreeNetBuffer(lpManager,lpNetBuffer);
		return NULL;
	}
	//
	//We have get a net buffer object and allocate a data buffer successfully,then,
	//we should initialize the net buffer object according to data buffer.
	//
	lpNetBuffer->lpBufferHdr    = lpDataBuff;
	lpNetBuffer->lpBufferEnd    = lpDataBuff + dwBuffLen;
	lpNetBuffer->lpDataHdr      = lpDataBuff;
	lpNetBuffer->lpDataEnd      = lpDataBuff;

	lpFragData = NET_BUFFER_FRAG_DATA(lpNetBuffer);  //Get the fragment data.
	NetBufferQueueInit(&lpFragData->FragQueue);      //Initialize the frag data queue.
	__INIT_ATOMIC(lpFragData->SharedCounter);        //Initialize the shared counter.
	AtomicSet(&lpFragData->SharedCounter,1);         //Set the shared counter to 1.

	return lpNetBuffer;
}

//
//The implementation of NetBufferFree.
//This routine decrement the reference counter of net buffer object,if the reference
//counter hits zero,then release the net buffer object and it's data buffer,else,
//only return.
//

static VOID NetBufferFree(__NET_BUFFER_MANAGER* lpManager,__NET_BUFFER* lpNetBuffer)
{
	if((NULL == lpManager) || (NULL == lpNetBuffer)) //parameter check.
		return;
	if(AtomicDec(&lpNetBuffer->Users))  //The reference counter hit zero.
	{
		NetBufferFreeData(lpManager,lpNetBuffer);  //Release the data buffer.
		__FreeNetBuffer(lpManager,lpNetBuffer);    //Release the net buffer object.
	}
	else    //The reference counter does not hit zero,it means,that there also some other
		    //entities are using the net buffer object.
		return;
}

//
//The implementation of NetBufferFreeData.
//

static VOID NetBufferFreeData(__NET_BUFFER_MANAGER* lpMgr,__NET_BUFFER* lpNetBuffer)
{
	__FRAG_DATA*              lpFragData                  = NULL;
	__NET_BUFFER*             lpNetBuff                   = NULL;

	if((NULL == lpMgr) || (NULL == lpNetBuffer)) //Parameter check.
		return;
	lpFragData = NET_BUFFER_FRAG_DATA(lpNetBuffer);
	if(AtomicDec(&lpFragData->SharedCounter))    //Share counter hits zero,should release
		                                         //all data.
	{
		lpNetBuff = lpFragData->FragQueue.GetFromQueue(&lpFragData->FragQueue);
		while(lpNetBuff)    //There is fragment data.
		{
			NetBufferFree(lpMgr,lpNetBuff);  //Release the fragment data.
			lpNetBuff = lpFragData->FragQueue.GetFromQueue(&lpFragData->FragQueue);
		}
		//__FreeNetBuffer(lpMgr,lpNetBuff);    //Release the memory the data buffer occupied.
		FREE_DATA_MEMORY(lpNetBuffer->lpBufferHdr);  //Release the data buffer.
	}
	else  //The shared counter is not zero,it means there is other entity is using the data
		  //buffer.
	{
		DWORD dwLoop = lpFragData->FragQueue.GetQueueLen(&lpFragData->FragQueue);
		for(DWORD i = 0;i < dwLoop;i ++)  //Decrement the reference counter of all fragment.
			                              //The reference counter of all fragments should
										  //never less than the reference counter of data
										  //buffer.
										  //This can be ensure by NetBufferClone routine,
										  //this routine will increment the reference counter
										  //of data buffer and all fragments.
										  //So,NetBufferFree routine only decrement the
										  //reference counter of all fragments.
		{
			lpNetBuff = lpFragData->FragQueue.GetFromQueue(&lpFragData->FragQueue);
			NetBufferFree(lpMgr,lpNetBuff);
			lpFragData->FragQueue.InsertIntoQueue(&lpFragData->FragQueue,lpNetBuff);
		}
	}
	return;
}

//
//The implementation of NetBufferGet.
//

static __NET_BUFFER* NetBufferGet(__NET_BUFFER* lpNetBuffer)
{
	if(NULL == lpNetBuffer)    //Parameter check.
		return NULL;
	AtomicInc(&lpNetBuffer->Users);    //Increment the reference counter.
	return lpNetBuffer;
}

//
//The implementation of NetBufferPull.
//

static UCHAR* NetBufferPull(__NET_BUFFER* lpNetBuffer,INT nOffset)
{
	DWORD                   dwFlags                   = 0L;

	if(NULL == lpNetBuffer)    //Parameter check.
		return NULL;
	if(NET_BUFFER_SHARED(lpNetBuffer)) //The net buffer is shared by other entities.
		return NULL;
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	lpNetBuffer->lpDataHdr += nOffset;
	if(lpNetBuffer->lpDataHdr < lpNetBuffer->lpBufferHdr)  //The nOffset is negative,and
		                                                   //it exceed the headroom.
	{
		lpNetBuffer->lpDataHdr -= nOffset;
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		return NULL;
	}
	if(lpNetBuffer->lpDataHdr >= lpNetBuffer->lpDataEnd)  //Exceed valid range.
	{
		lpNetBuffer->lpDataHdr -= nOffset;
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		return NULL;
	}
	lpNetBuffer->dwDataLen  += nOffset;    //Update the data length.
	lpNetBuffer->dwTotalLen += nOffset;    //Update the tatol length.
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	return lpNetBuffer->lpDataHdr;             //Returns the start address of data area.
}

//
//The implementation of NetBufferPush.
//

static UCHAR* NetBufferPush(__NET_BUFFER* lpNetBuffer,INT nOffset)
{
	DWORD                   dwFlags                   = 0L;

	if(NULL == lpNetBuffer)    //Parameter check.
		return NULL;
	if(NET_BUFFER_SHARED(lpNetBuffer)) //The net buffer is shared by other entities.
		return NULL;
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	lpNetBuffer->lpDataHdr -= nOffset;
	if(lpNetBuffer->lpDataHdr < lpNetBuffer->lpBufferHdr)  //The nOffset is larger than
		                                                   //data buffer's headroom.
	{
		lpNetBuffer->lpDataHdr += nOffset;
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		return NULL;
	}
	if(lpNetBuffer->lpDataHdr >= lpNetBuffer->lpDataEnd)  //The nOffset is negative,and
		                                                  //it's absolute value is larger than
														  //data area.
	{
		lpNetBuffer->lpDataHdr += nOffset;
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		return NULL;
	}
	lpNetBuffer->dwDataLen  -= nOffset;
	lpNetBuffer->dwTotalLen -= nOffset;
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	return lpNetBuffer->lpDataHdr;             //Returns the start address of data area.
}

//
//The implementation of NetBufferPut.
//

static UCHAR* NetBufferPut(__NET_BUFFER* lpNetBuffer,INT nOffset)
{
	DWORD                  dwFlags                   = 0L;
	UCHAR*                 lpTmp                     = NULL;

	if(NULL == lpNetBuffer)    //Parameter check.
		return NULL;
	if(NET_BUFFER_SHARED(lpNetBuffer)) //The net buffer object is shared by other entity.
		return NULL;

	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	lpTmp = lpNetBuffer->lpDataEnd;
	lpNetBuffer->lpDataEnd += nOffset;    //Add the offset.
	if(lpNetBuffer->lpDataEnd <= lpNetBuffer->lpDataHdr)  //Exceed the data area.
	{
		lpNetBuffer->lpDataEnd -= nOffset; //Restore the initial value.
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		return NULL;
	}
	if(lpNetBuffer->lpDataEnd > lpNetBuffer->lpBufferEnd) //The nOffset larger than tailroom.
	{
		lpNetBuffer->lpDataEnd -= nOffset;
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		return NULL;
	}
	lpNetBuffer->dwDataLen  += nOffset;
	lpNetBuffer->dwTotalLen += nOffset;
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	return lpTmp;
}

//
//The implementation of NetBufferReserve.
//

static BOOL NetBufferReserve(__NET_BUFFER* lpNetBuffer,INT nLen)
{
	DWORD                    dwFlags                   = 0L;

	if(NULL == lpNetBuffer)  //Parameter check.
		return FALSE;
	if(NET_BUFFER_SHARED(lpNetBuffer))  //The net buffer object is shared by other entity.
		return FALSE;
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	lpNetBuffer->lpDataHdr  += nLen;
	lpNetBuffer->lpDataEnd  += nLen;
	if((lpNetBuffer->lpDataHdr <  lpNetBuffer->lpBufferHdr) ||
	   (lpNetBuffer->lpDataEnd >= lpNetBuffer->lpBufferEnd))    //Exceed valid range.
	{
		lpNetBuffer->lpDataEnd -= nLen;
		lpNetBuffer->lpDataHdr -= nLen;
		//__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		return NULL;
	}
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	return TRUE;
}

//
//The implementation of NetBufferStore.
//In current implementation,this routine can be replaced by NetBufferPut routine,because
//the NetBufferStore routine does not process the fragment data.
//

static BOOL NetBufferStore(__NET_BUFFER* lpNetBuffer,INT nOffset,UCHAR* lpSrc,DWORD dwLen)
{
	return FALSE;
}

//
//The implementation of NetBufferSave.
//

static BOOL NetBufferSave(__NET_BUFFER* lpNetBuffer,INT nOffset,UCHAR* lpDes,DWORD dwLen)
{
	return FALSE;
}

//
//The implementation of NetBufferExpand.
//This routine create a new data buffer,copy the old data buffer's data into the new one,
//and append all fragment data of old data buffer to new one,then release the old data
//buffer.
//If the net buffer or data buffer is shared,then only returns FALSE.
//

static BOOL NetBufferExpand(__NET_BUFFER* lpNetBuffer,DWORD dwHeadroom,DWORD dwTailroom)
{
	__FRAG_DATA*           lpFragData                 = NULL;
	__NET_BUFFER*          lpNetBuff                  = NULL;
	DWORD                  dwFlags                    = 0L;
	DWORD                  dwBuffSize                 = 0L;
	UCHAR*                 lpTmpBuff                  = NULL;
	UCHAR*                 lpDataStart                = NULL;

	if(NULL == lpNetBuffer) //Parameter check.
		return FALSE;
	if(NET_BUFFER_SHARED(lpNetBuffer)) //The net buffer is shared.
		return FALSE;
	lpFragData = NET_BUFFER_FRAG_DATA(lpNetBuffer);  //Get fragment data.
	if(AtomicGet(&lpFragData->SharedCounter) != 1)   //The data buffer is shared also.
		return FALSE;
	dwBuffSize = NET_BUFFER_DATA_LEN(lpNetBuffer) + dwHeadroom + dwTailroom;
	lpTmpBuff  = ALLOC_DATA_MEMORY(dwBuffSize);
	if(NULL == lpTmpBuff)    //Can not allocate data memory.
		return FALSE;
	lpDataStart = lpTmpBuff + dwHeadroom;
	MemCpy(lpDataStart,lpNetBuffer->lpDataHdr,NET_BUFFER_DATA_LEN(lpNetBuffer));  //Copy data.
	lpDataStart += NET_BUFFER_DATA_LEN(lpNetBuffer);
	lpDataStart += dwTailroom;
	MemCpy(lpDataStart,lpNetBuffer->lpBufferEnd,FRAG_DATA_SIZE);  //Copy fragment data.
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	FREE_DATA_MEMORY(lpNetBuffer->lpBufferHdr);    //Release the old data buffer.
	lpNetBuffer->lpBufferHdr  = lpTmpBuff;
	lpNetBuffer->lpBufferEnd  = lpDataStart;
	lpNetBuffer->lpDataHdr    = lpTmpBuff + dwHeadroom;
	lpNetBuffer->lpDataEnd    = lpDataStart - dwTailroom;
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	return TRUE;
}

//
//The implementation of NetBufferAppendFrag.
//This routine add a fragment data to the net buffer's frag data list.
//If the current data buffer or the net buffer object is shared,this 
//routine will do nothing,only returns FALSE.
//Parameters:
// @lpNetBuffer : The net buffer object which fragment data will be appended to.
// @lpAppended  : The net buffer object which will be appended.
//That is,this routine will put lpAppended to lpNetBuffer's fragment data.
//

static BOOL NetBufferAppendFrag(__NET_BUFFER* lpNetBuffer,__NET_BUFFER* lpAppended)
{
	__FRAG_DATA*                lpFragData                = NULL;
	DWORD                       dwFlags                   = 0L;

	if((NULL == lpNetBuffer) || (NULL == lpAppended)) //Parameter check.
		return FALSE;
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	if(NET_BUFFER_SHARED(lpNetBuffer))    //The target net buffer is shared.
	{
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		return FALSE;
	}
	lpFragData = NET_BUFFER_FRAG_DATA(lpNetBuffer);   //Get the fragment control block.
	if(AtomicGet(&lpFragData->SharedCounter) != 1)    //The data buffer is shared.
	{
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		return FALSE;
	}
	//
	//The following code append the lpAppended to the fragment list.
	//
	NetBufferGet(lpAppended);    //Increment the reference counter.
	lpFragData->FragQueue.InsertIntoQueue(&lpFragData->FragQueue,lpAppended);  //Insert into queue.
	lpNetBuffer->dwTotalLen += NET_BUFFER_DATA_LEN(lpAppended);  //Update total length.
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	return TRUE;
}

//
//The implementation of NetBufferGetFrag.
//This routine gets the first fragment data from fragment list.
//If the net buffer object is shared,or the data buffer is shared,
//this routine will do nothing,only returns a NULL.
//

static __NET_BUFFER* NetBufferGetFrag(__NET_BUFFER* lpNetBuffer)
{
	return NULL;
}

//
//The implementation of NetBufferCopy.
//

static __NET_BUFFER* NetBufferCopy(__NET_BUFFER* lpNetBuffer)
{
	return NULL;
}

//
//The implementation of NetBufferSharedCopy.
//

static __NET_BUFFER* NetBufferSharedCopy(__NET_BUFFER* lpNetBuffer)
{
	return NULL;
}

//
//The implementation of NetBufferPackCopy.
//This routine allocates a new net buffer object and a new data buffer,
//then copies the data buffer,including fragment data,into the new data buffer,
//and returns the new created net buffer object.
//

static __NET_BUFFER* NetBufferPackCopy(__NET_BUFFER_MANAGER* lpManager,__NET_BUFFER* lpNetBuffer)
{
	__NET_BUFFER*                     lpNetBuff      = NULL;
	__NET_BUFFER*                     lpFragBuff     = NULL;
	__FRAG_DATA*                      lpFragData     = NULL;
	DWORD                             dwFlags        = 0L;
	DWORD                             dwBuffSize     = 0L;
	UCHAR*                            lpDataBuff     = NULL;

	if(NULL == lpNetBuffer)    //Parameter check.
		return NULL;
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	dwBuffSize  = NET_BUFFER_TOTAL_LEN(lpNetBuffer);  //Get the total length.
	dwBuffSize += NET_BUFFER_HEADROOM(lpNetBuffer);
	dwBuffSize += NET_BUFFER_TAILROOM(lpNetBuffer);
	lpNetBuff  = NetBufferAlloc(lpManager,dwBuffSize);  //Allocate a new net buffer object.
	if(NULL == lpNetBuff)     //Can not allocate net buffer object,may caused by less memory.
	{
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		return NULL;
	}
	NetBufferReserve(lpNetBuff,NET_BUFFER_HEADROOM(lpNetBuffer));  //Reserve head room.
	MemCpy((LPVOID)NetBufferPut(lpNetBuff,NET_BUFFER_DATA_LEN(lpNetBuffer)),
		lpNetBuffer->lpDataHdr,NET_BUFFER_DATA_LEN(lpNetBuffer));  //Copy data.
	lpFragData = NET_BUFFER_FRAG_DATA(lpNetBuffer);
	DWORD dwQueueLen = lpFragData->FragQueue.GetQueueLen(&lpFragData->FragQueue);
	for(DWORD dwLoop = 0;dwLoop < dwQueueLen;dwLoop ++)  //Copy fragment data.
	{
		lpFragBuff = lpFragData->FragQueue.GetFromQueue(&lpFragData->FragQueue);
		MemCpy((LPVOID)NetBufferPut(lpNetBuff,NET_BUFFER_DATA_LEN(lpFragBuff)),
			lpFragBuff->lpDataHdr,NET_BUFFER_DATA_LEN(lpFragBuff));
		lpFragData->FragQueue.InsertIntoQueue(&lpFragData->FragQueue,
			lpFragBuff);    //Re-insert into the frag queue again.
	}
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	return lpNetBuff;
}

//
//The implementation of NetBufferClone.
//

static __NET_BUFFER* NetBufferClone(__NET_BUFFER* lpNetBuffer)
{
	return NULL;
}

//
//The implementation of Initialize.
//

static BOOL ManagerInitialize(__NET_BUFFER_MANAGER* lpManager)
{
	if(NULL == lpManager)    //Parameter check.
		return FALSE;

	lpManager->FreeBufferLink.lpNext       = NULL;
	lpManager->FreeBufferLink.dwLinkLen    = 0L;
	lpManager->Initialize                  = ManagerInitialize;
	lpManager->NetBufferAlloc              = NetBufferAlloc;
	lpManager->NetBufferFreeData           = NetBufferFreeData;
	lpManager->NetBufferFree               = NetBufferFree;
	lpManager->NetBufferGet                = NetBufferGet;
	lpManager->NetBufferPull               = NetBufferPull;
	lpManager->NetBufferPush               = NetBufferPush;
	lpManager->NetBufferPut                = NetBufferPut;
	lpManager->NetBufferReserve            = NetBufferReserve;
	lpManager->NetBufferStore              = NetBufferStore;
	lpManager->NetBufferSave               = NetBufferSave;
	lpManager->NetBufferExpand             = NetBufferExpand;
	lpManager->NetBufferAppendFrag         = NetBufferAppendFrag;
	lpManager->NetBufferGetFrag            = NetBufferGetFrag;
	lpManager->NetBufferCopy               = NetBufferCopy;
	lpManager->NetBufferSharedCopy         = NetBufferSharedCopy;
	lpManager->NetBufferPackCopy           = NetBufferPackCopy;
	lpManager->NetBufferClone              = NetBufferClone;

	return TRUE;
}

/*************************************************************************************
**************************************************************************************
**************************************************************************************
**************************************************************************************
*************************************************************************************/

//
//Global object : NetBufferManager.
//This object is used by all network entitis to operate net buffers.
//

__NET_BUFFER_MANAGER NetBufferManager;
