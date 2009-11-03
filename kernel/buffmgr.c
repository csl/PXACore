//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Aug,25 2004
//    Module Name               : buffmgr.cpp
//    Module Funciton           : 
//                                This module countains the buffer manager
//                                implementation code.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include "stdafx.h"

//
//Create buffer routine,used by buffer manager to allocate a new buffer pool,and
//initializes the buffer control block.
//

//The following routine create a buffer pool by calling KMemAlloc.
static BOOL CreateBuffer1(struct __BUFFER_CONTROL_BLOCK* pControlBlock,DWORD dwPoolSize)
{
	BOOL                     bResult      = FALSE;
	LPVOID                   lpBufferPool = NULL;
	DWORD                    dwSize       = 0L;
	struct __FREE_BUFFER_HEADER*    pFreeHdr     = NULL;

	dwSize = RoundTo4k(dwPoolSize);
	lpBufferPool = KMemAlloc(dwPoolSize,KMEM_SIZE_TYPE_4K);
	if(NULL == lpBufferPool)                  //Fail to allocate memory.
		goto __TERMINAL;

	pControlBlock->dwPoolSize         = dwSize;  //Initialize the buffer control block.
	pControlBlock->dwFlags           |= CREATED_BY_SELF;
	pControlBlock->dwFlags           |= POOL_INITIALIZED;  //Set the pool initialized bit.
	pControlBlock->lpPoolStartAddress = lpBufferPool;
	pControlBlock->dwFreeSize = dwSize;

	pFreeHdr = (struct __FREE_BUFFER_HEADER*)lpBufferPool;
	pControlBlock->lpFreeBufferHeader = pFreeHdr;
	pFreeHdr->dwFlags = BUFFER_STATUS_FREE;
	pFreeHdr->lpNextBlock = NULL;
	pFreeHdr->lpPrevBlock = NULL; /*pControlBlock->lpFreeBufferHeader;*/
	pFreeHdr->dwBlockSize = dwSize - sizeof(struct __FREE_BUFFER_HEADER);
	pControlBlock->dwFreeSize = pFreeHdr->dwBlockSize;

	bResult = TRUE;

__TERMINAL:

	return bResult;
}

//
//The following routine create a buffer pool by using client created memory,and initialize
//the buffer control block.
//

static BOOL CreateBuffer2(struct __BUFFER_CONTROL_BLOCK* pControlBlock,
						  LPVOID lpBufferPool,
						  DWORD dwPoolSize)
{
	BOOL bResult      = FALSE;
	DWORD dwSize       = 0L;
	struct __FREE_BUFFER_HEADER*    pFreeHdr     = NULL;

	dwSize = dwPoolSize;

	pControlBlock->dwPoolSize         = dwSize;  //Initialize the buffer control block.
	pControlBlock->dwFlags           |= CREATED_BY_CLIENT;
	pControlBlock->dwFlags           |= POOL_INITIALIZED;  //Set the pool initialized bit.
	pControlBlock->lpPoolStartAddress = lpBufferPool;

	pFreeHdr = (struct __FREE_BUFFER_HEADER*)lpBufferPool;
	pControlBlock->lpFreeBufferHeader = pFreeHdr;
	pFreeHdr->dwFlags = BUFFER_STATUS_FREE;
	pFreeHdr->lpNextBlock = NULL;
	pFreeHdr->lpPrevBlock = NULL; /*pControlBlock->lpFreeBufferHeader;*/
	pFreeHdr->dwBlockSize = dwSize - sizeof(struct __FREE_BUFFER_HEADER);
	pControlBlock->dwFreeSize = pFreeHdr->dwBlockSize;

	bResult = TRUE;

	return bResult;
}

//
//The following is a helper function,it combines continuous free blocks into one
//big free block,and update the other free blocks's data structure,such as lpNextBlock,
//lpPrevBlock.
//

static VOID CombineFreeBlock(struct __BUFFER_CONTROL_BLOCK* lpControlBlock,  //The control block.
			     LPVOID lpStartCombine)  

//The first block's address where to 											  start combine.
																	  //This function scans
																	  //the buffer pool
																	  //from this position
																	  //to end.
																	  //If this parameter
																	  //is NULL,then start
																	  //at the pool's original
																	  //address,which can
																	  //be get from
																	  //lpControlBlock's member.
{
	LPVOID                   lpEndAddr               = NULL;
	struct __FREE_BUFFER_HEADER*    lpFreeHeader            = NULL;
	struct __FREE_BUFFER_HEADER*    lpNextHeader            = NULL;
	struct __USED_BUFFER_HEADER*    lpUsedHeader            = NULL;
	DWORD	                 dwFlags                 = 0L;

	if(NULL == lpControlBlock)    //Parameters check.
	{
		return;
	}

	//ENTER_CRITICAL_SECTION();    //The following code must not be interrupted.
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);

	lpFreeHeader = (NULL == lpStartCombine) ? (struct __FREE_BUFFER_HEADER*)(lpControlBlock->lpPoolStartAddress) : 
	                                          (struct __FREE_BUFFER_HEADER*)lpStartCombine;

    lpEndAddr = (LPVOID)((DWORD)lpControlBlock->lpPoolStartAddress + lpControlBlock->dwPoolSize);

	lpNextHeader = lpFreeHeader ? (struct __FREE_BUFFER_HEADER*)((DWORD)lpFreeHeader + 
	                         sizeof(struct __FREE_BUFFER_HEADER) + 
			         lpFreeHeader->dwBlockSize) : NULL;

	while(TRUE)
	{
		if(lpEndAddr == (LPVOID)lpNextHeader)  //Now,seek to the end of the buffer pool.
			break;

		if(lpFreeHeader->dwFlags & BUFFER_STATUS_USED)  //The current block is used.
		{
			lpFreeHeader = lpNextHeader;
			lpNextHeader = (struct __FREE_BUFFER_HEADER*)((DWORD)lpFreeHeader + 
                        sizeof(struct __USED_BUFFER_HEADER) + lpFreeHeader->dwBlockSize); 
			//Skip the current block.
			continue;   //Continue to check next block.
		}

		if(lpNextHeader->dwFlags & BUFFER_STATUS_USED) //If the next block is used,then
			                                           //skip two blocks.
		{
			lpFreeHeader = (struct __FREE_BUFFER_HEADER*)((DWORD)lpNextHeader + 
                                   sizeof(struct __FREE_BUFFER_HEADER) + 					   lpNextHeader->dwBlockSize);

			if((LPVOID)lpFreeHeader == lpEndAddr)  
			//Seek to the end of the buffer pool.
				break;

			lpNextHeader = (struct __FREE_BUFFER_HEADER*)((DWORD)lpFreeHeader +
			       sizeof(struct __FREE_BUFFER_HEADER) +    lpFreeHeader->dwBlockSize);
			//lpNextHeader = NULL;
			continue;
		}
		else  //The next block is free too,so we combine the two blocks,and update the
			  //free block list.
		{
			if(NULL == lpNextHeader->lpPrevBlock) //The next block is the first free block.
			{
				lpNextHeader->lpNextBlock->lpPrevBlock = NULL;
				lpControlBlock->lpFreeBufferHeader = lpNextHeader->lpNextBlock;
			}
			else  //The next block is not the first block.
			{
				if(NULL == lpNextHeader->lpNextBlock) //Is the last block.
				{
					lpNextHeader->lpPrevBlock->lpNextBlock = NULL;
				}
				else  //Not the last block.
				{
				lpNextHeader->lpNextBlock->lpPrevBlock = lpNextHeader->lpPrevBlock;
				lpNextHeader->lpPrevBlock->lpNextBlock = lpNextHeader->lpNextBlock;
				}
			}

			//Combine the two block.
			lpFreeHeader->dwBlockSize += sizeof(struct __FREE_BUFFER_HEADER);  
			lpFreeHeader->dwBlockSize += lpNextHeader->dwBlockSize;

			lpNextHeader = (struct __FREE_BUFFER_HEADER*)((DWORD)lpFreeHeader + 
			sizeof(struct __FREE_BUFFER_HEADER) + lpFreeHeader->dwBlockSize);

		}
	}

	//LEAVE_CRITICAL_SECTION();
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
}

//
//Allocate buffer routine,this routine allocates a buffer from buffer pool,according
//the size of the client request,and returns the start address of the buffer.
//

static LPVOID Allocate(struct __BUFFER_CONTROL_BLOCK* pControlBlock,DWORD dwSize)
{
	LPVOID                   lpBuffer         = NULL;
	struct __FREE_BUFFER_HEADER*    lpFreeHeader     = NULL;
	struct __FREE_BUFFER_HEADER*    lpTmpHeader      = NULL;
	struct __USED_BUFFER_HEADER*    lpUsedHeader     = NULL;
	DWORD                    dwBoundrySize    = 0L;
	BOOL                     bFind            = FALSE;
	DWORD                    dwFlags          = 0L;

	if((NULL == pControlBlock) || (0 == dwSize))  //Parameters check.
		goto __TERMINAL;

__BEGIN:

	if(dwSize < MIN_BUFFER_SIZE)
		dwSize = MIN_BUFFER_SIZE;

	dwBoundrySize = dwSize + MIN_BUFFER_SIZE + sizeof(struct __FREE_BUFFER_HEADER);

	//ENTER_CRITICAL_SECTION();    //The following operation must not be interrupted.
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);

	lpFreeHeader = pControlBlock->lpFreeBufferHeader;
	while(lpFreeHeader)
	{
		if(lpFreeHeader->dwBlockSize < dwSize)   //The current block is not fitable.
		{
			lpFreeHeader = lpFreeHeader->lpNextBlock;
			continue;
		}

		bFind = TRUE;        //Found one block fit the request.

		if(lpFreeHeader->dwBlockSize <= dwBoundrySize)  //Allocate the whole free block.
		{
			if(NULL == lpFreeHeader->lpPrevBlock)  //This block is the first free block.
			{
				if(NULL == lpFreeHeader->lpNextBlock)  //This is the last free block.
				{
					pControlBlock->lpFreeBufferHeader = NULL;
				}
				else    //This is not the last free block.
				{
					lpFreeHeader->lpNextBlock->lpPrevBlock = NULL;
					pControlBlock->lpFreeBufferHeader = lpFreeHeader->lpNextBlock;
				}
			}
			else        //This block is not the first free block.
			{
				if(NULL == lpFreeHeader->lpNextBlock)  //This is the last block.
				{
					lpFreeHeader->lpPrevBlock->lpNextBlock = NULL;
				}
				else    //This is not the last free block.
				{
					lpFreeHeader->lpNextBlock->lpPrevBlock = lpFreeHeader->lpPrevBlock;
					lpFreeHeader->lpPrevBlock->lpNextBlock = lpFreeHeader->lpNextBlock;
				}
			}
			lpUsedHeader = (struct __USED_BUFFER_HEADER*)lpFreeHeader;
			lpUsedHeader->dwFlags &= ~BUFFER_STATUS_FREE;  //Clear the free bit.
			lpUsedHeader->dwFlags |= BUFFER_STATUS_USED;   //Set the used bit.
			                                               //The used block's size
			                                               //is the same as when the
			                                               //block is free.

            lpBuffer = (LPVOID)((UCHAR*)lpUsedHeader + sizeof(struct __USED_BUFFER_HEADER));
			break;
		}
		else      //Can not allocate the whole free block,because it's size is enough to
			      //separate into two blocks,one block is allocated to client,and another
				  //is reserved free.
		{
			//The lpTmpHeader will be the header pointer of new separated free block.
            lpTmpHeader = (struct __FREE_BUFFER_HEADER*)((DWORD)lpFreeHeader + sizeof(struct __FREE_BUFFER_HEADER) + dwSize);
			lpTmpHeader->lpPrevBlock = lpFreeHeader->lpPrevBlock;
			lpTmpHeader->lpNextBlock = lpFreeHeader->lpNextBlock;
			lpTmpHeader->dwBlockSize = lpFreeHeader->dwBlockSize - sizeof(struct __FREE_BUFFER_HEADER)- dwSize;
			lpTmpHeader->dwFlags = 0L;
			lpTmpHeader->dwFlags |= BUFFER_STATUS_FREE;  //Set the free bit.

			//Now,the following code update the neighbor free blocks's member.
			if(NULL == lpFreeHeader->lpPrevBlock) //This block is the first free block.
			{
				if(NULL == lpFreeHeader->lpNextBlock)  //This is the last free block.
				{
					pControlBlock->lpFreeBufferHeader = lpTmpHeader;
				}
				else //This is not the last free block.
				{
					pControlBlock->lpFreeBufferHeader = lpTmpHeader;
					lpFreeHeader->lpNextBlock->lpPrevBlock = lpTmpHeader;
				}
			}
			else    //This is not the first free block.
			{
				if(NULL == lpFreeHeader->lpNextBlock)  //This is the last free block.
				{
					lpFreeHeader->lpPrevBlock->lpNextBlock = lpTmpHeader;
				}
				else  //This is not the first,and also not the last free block.
				{
					lpFreeHeader->lpPrevBlock->lpNextBlock = lpTmpHeader;
					lpFreeHeader->lpNextBlock->lpPrevBlock = lpTmpHeader;
				}
			}

			lpUsedHeader = (struct __USED_BUFFER_HEADER*)lpFreeHeader;
			lpUsedHeader->dwFlags &= ~BUFFER_STATUS_FREE;  //Clear the free bit.
			lpUsedHeader->dwFlags |= BUFFER_STATUS_USED;   //Set the used bit.
			lpUsedHeader->dwBlockSize = dwSize;

            lpBuffer = (LPVOID)((UCHAR*)lpUsedHeader + sizeof(struct __USED_BUFFER_HEADER));
			break;
		}
	}

	//LEAVE_CRITICAL_SECTION();
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

	if(!bFind)    //If can not find the proper free block,then do a combine action,and
		          //refind again.
	{
		bFind = TRUE;
		CombineFreeBlock(pControlBlock,NULL);
		goto __BEGIN;
	}

__TERMINAL:

	return lpBuffer;
}

//
//Free buffer routine,this routine returns the buffer to buffer pool.
//

static VOID Free(struct __BUFFER_CONTROL_BLOCK* pControlBlock,LPVOID lpBuffer)
{
	struct __FREE_BUFFER_HEADER*       lpFreeHeader  = NULL;
	DWORD                       dwFlags       = 0L;

	lpFreeHeader = (struct __FREE_BUFFER_HEADER*)((DWORD)lpBuffer - sizeof(struct __FREE_BUFFER_HEADER));

	//ENTER_CRITICAL_SECTION();
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);

	lpFreeHeader->dwFlags &= ~BUFFER_STATUS_USED;  //Clear the used bit.
	lpFreeHeader->dwFlags |= BUFFER_STATUS_FREE;  //Set the free bit.
	lpFreeHeader->lpNextBlock = pControlBlock->lpFreeBufferHeader;  //Insert the current
	                                                                //free blocks into free list.
	lpFreeHeader->lpPrevBlock = NULL;
	pControlBlock->lpFreeBufferHeader = lpFreeHeader;
	if(NULL != lpFreeHeader->lpNextBlock)
	{
		lpFreeHeader->lpNextBlock->lpPrevBlock = lpFreeHeader;
	}

	//LEAVE_CRITICAL_SECTION();
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

	CombineFreeBlock(pControlBlock,(LPVOID)lpFreeHeader);  //Combine the free blocks.
}

//
//Buffer flags operating routine,these routine set or get the buffer's flags.
//

static DWORD GetBufferFlag(struct __BUFFER_CONTROL_BLOCK* pControlBlock,LPVOID lpBuffer)
{
	DWORD                  dwFlag       = 0L;
	struct __USED_BUFFER_HEADER*  lpUsedHeader = NULL;

	if(NULL == lpBuffer)  //Parameter check.
		return dwFlag;

	lpUsedHeader = (struct __USED_BUFFER_HEADER*)((DWORD)lpBuffer - sizeof(struct __USED_BUFFER_HEADER));
	dwFlag = lpUsedHeader->dwFlags;

	return dwFlag;
}

//
//NOTICE : The following routine reset the used buffer's flags,overwrite the previous flags.
//

static BOOL SetBufferFlag(struct __BUFFER_CONTROL_BLOCK* pControlBlock,LPVOID lpBuffer,DWORD dwFlag)
{
	BOOL                   bResult      = FALSE;
	struct __USED_BUFFER_HEADER*  lpUsedHeader = NULL;

	if(NULL == lpBuffer)
		return bResult;

	lpUsedHeader = (struct __USED_BUFFER_HEADER*)((DWORD)lpBuffer - sizeof(struct __USED_BUFFER_HEADER));
	lpUsedHeader->dwFlags = dwFlag;
	bResult = TRUE;

	return bResult;
}

//
//Destroy buffer pool routine.
//

static VOID DestroyBuffer(struct __BUFFER_CONTROL_BLOCK* pControlBlock)
{
	if(pControlBlock->dwFlags | CREATED_BY_SELF)
	{
		KMemFree((LPVOID)pControlBlock->lpPoolStartAddress,
			KMEM_SIZE_TYPE_4K,
			4096);
		pControlBlock->dwFlags            = 0L;
		pControlBlock->dwFreeSize         = 0L;
		pControlBlock->dwPoolSize         = 0L;
		pControlBlock->lpFreeBufferHeader = NULL;
		pControlBlock->lpPoolStartAddress = NULL;
	}
}

//
//The following routine get the buffer control block's flags.
//

static DWORD GetControlBlockFlag(struct __BUFFER_CONTROL_BLOCK* pControlBlock)
{
	return pControlBlock->dwFlags;
}

//
//The global routine used to initialize a buffer manager.
//This routine set the buffer manager's operating routine.
//

BOOL InitBufferMgr(struct __BUFFER_CONTROL_BLOCK* pControlBlock)
{
	BOOL                  bResult = FALSE;

	if(NULL == pControlBlock)
		goto __TERMINAL;

	pControlBlock->BufferOperations.lpCreateBuffer1 = CreateBuffer1;
	pControlBlock->BufferOperations.lpCreateBuffer2 = CreateBuffer2;
	pControlBlock->BufferOperations.lpAllocate      = Allocate;
	pControlBlock->BufferOperations.lpFree          = Free;
	pControlBlock->BufferOperations.lpGetBufferFlag = GetBufferFlag;
	pControlBlock->BufferOperations.lpSetBufferFlag = SetBufferFlag;
	pControlBlock->BufferOperations.lpDestroyBuffer = DestroyBuffer;

	pControlBlock->GetControlBlockFlag = GetControlBlockFlag;

	pControlBlock->lpFreeBufferHeader  = NULL;
	pControlBlock->lpPoolStartAddress  = NULL;
	pControlBlock->dwFlags             = 0L;
	pControlBlock->dwFlags             |= OPERATIONS_INITIALIZED;
	pControlBlock->dwFreeSize          = 0L;
	pControlBlock->dwPoolSize          = 0L;
	bResult = TRUE;

__TERMINAL:

	return bResult;
}
