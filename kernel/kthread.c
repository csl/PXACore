//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jun,28 2004
//    Module Name               : kthread.h
//    Module Funciton           : 
//                                This module countains kernal thread
//                                implementation code.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __STDAFX_H__
#include "stdafx.h"
#endif

//
//The following two global variables are used to control the 
//kernal threads.
//Once create a kernal thread,the creat rountine will add a entry
//to the __KTHREAD_CONTROL_BLOCK* array,and make the proper
//g_bKThreadQueueStatus index to 1.
//Once delete a kernal thread,onle set the g_bKThreadQueueStatus's
//indexed value to 0.
//
__KTHREAD_CONTROL_BLOCK*  g_pKThreadQueue[MAX_KTHREAD_NUM]        = {0};
BYTE                      g_bKThreadQueueStatus[MAX_KTHREAD_NUM]  = {0};

//
//The kernal thread queue pointers.
//The system maintaince some kernal thread queues,including:
//1. Ready queue,countains the kernal thread ready to run;
//2. Blocked queue,countains the blocked kernal thread;
//3. Running queue,countains the current running kernal threads;
//4. Dead queue,countains the dead kernal thread;
//5. Suspend queue,countains the suspended kernal thread.
//
static __KTHREAD_CONTROL_BLOCK*  g_pReadyQueue     = NULL;
static __KTHREAD_CONTROL_BLOCK*  g_pBlockedQueue   = NULL;
static __KTHREAD_CONTROL_BLOCK*  g_pRunningQueue   = NULL;
static __KTHREAD_CONTROL_BLOCK*  g_pDeadQueue      = NULL;
static __KTHREAD_CONTROL_BLOCK*  g_pSuspendQueue   = NULL;

//
//The current running kernal thread's ID.
//
static DWORD  g_dwCurrentKThreadID = 1;

//
//Message queue operations.
//

//If message queue is full.
BOOL KtMsgQueueFull(__KTHREAD_CONTROL_BLOCK* pControlBlock)
{
	if(NULL == pControlBlock)  //Parameter check.
		return FALSE;

	return MAX_KTHREAD_MSG_NUM == pControlBlock->wCurrentMsgNum ? TRUE : FALSE;
}

//If message queue empty.
BOOL KtMsgQueueEmpty(__KTHREAD_CONTROL_BLOCK* pControlBlock)
{
	if(NULL == pControlBlock)
		return FALSE;

	return 0 == pControlBlock->wCurrentMsgNum ? TRUE : FALSE;
}

//Add a message to the current kernal thread's message queue.
BOOL KtSendMessage(__KTHREAD_CONTROL_BLOCK* pControlBlock,
				   __KTHREAD_MSG*           pMsg)
{
	BOOL           bResult = FALSE;

	if((NULL == pControlBlock) || (NULL == pMsg)) //Parameters check.
		return bResult;

	if(KtMsgQueueFull(pControlBlock))             //If the queue is full.
		return bResult;

	//DisableInterrupt();                           //The following operation should not be
	                                              //interrupted.
	pControlBlock->ktmsg[pControlBlock->wTrial].wCommand    = pMsg->wCommand;
	pControlBlock->ktmsg[pControlBlock->wTrial].dwParam_01  = pMsg->dwParam_01;
	pControlBlock->ktmsg[pControlBlock->wTrial].dwParam_02  = pMsg->dwParam_02;

	pControlBlock->wTrial ++;
	if(MAX_KTHREAD_MSG_NUM == pControlBlock->wTrial)
		pControlBlock->wTrial = 0x0000;
	pControlBlock->wCurrentMsgNum ++;
	bResult = TRUE;
	
	//EnableInterrupt();                            //Enable interrupts.

	return bResult;
}

//Get a message from current kernal thread's message queue and delete it from the queue.
BOOL KtGetMessage(__KTHREAD_CONTROL_BLOCK* pControlBlock,
				  __KTHREAD_MSG*           pMsg)
{
	if((NULL == pControlBlock) || (NULL == pMsg))  //Parameters check.
		return FALSE;

	if(KtMsgQueueEmpty(pControlBlock))
		return FALSE;

	//DisableInterrupt();
	pMsg->wCommand     = pControlBlock->ktmsg[pControlBlock->wHeader].wCommand;
	pMsg->dwParam_01   = pControlBlock->ktmsg[pControlBlock->wHeader].dwParam_01;
	pMsg->dwParam_02   = pControlBlock->ktmsg[pControlBlock->wHeader].dwParam_02;

	pControlBlock->wHeader ++;
	if(MAX_KTHREAD_MSG_NUM == pControlBlock->wHeader)
		pControlBlock->wHeader = 0x0000;
	pControlBlock->wCurrentMsgNum --;
	//EnableInterrupt();                    //Enable the interrupt.

	return TRUE;
}

BOOL KtDispatchMessage(__KTHREAD_MSG* pMsg,
						__KTHREAD_MSG_HANDLER    msghdr)
{
	if((NULL == pMsg) || (NULL == msghdr))
		return FALSE;

	return msghdr(pMsg->wCommand,pMsg->dwParam_01,pMsg->dwParam_01);
}

//
//Global functions implementation.
//

DWORD  GetCurrentKThreadID()
{
	return g_dwCurrentKThreadID;
}

//
//Create a thread,and put the thread's control block into array g_pKThreadQueue,
//and update the corresponding queue.
//If failed,it returns 0,otherwise,returns the created kernal thread's ID.
//
DWORD CreateKThread(DWORD             dwStackSize,      //Thread's stack size.
					DWORD             dwFlags,          //Flags.
					DWORD             dwPriority,       //Priority.
					LPKTHREAD_ROUTINE pStartAddress,    //Start running address.
					LPVOID            pData,            //Parameter.
					LPVOID            /*pReserved*/)        //Reserved.
{
	DWORD                    dwKThreadID      = 0x00000000;
	__KTHREAD_CONTROL_BLOCK* pControlBlock    = NULL;
	LPVOID                   pStackPointer    = NULL;
	BOOL                     bFind            = FALSE;
	
	RoundTo4k(dwStackSize);               //Round the stack size to 4k times.
	pControlBlock = (__KTHREAD_CONTROL_BLOCK*)KMemAlloc(dwStackSize,KMEM_SIZE_TYPE_4K);
	if(NULL == pControlBlock)             //If can not allocate the memory.
	{
		return dwKThreadID;
	}
	MemZero((LPVOID)pControlBlock,dwStackSize);    //Zero the memory allocated just now.

	pControlBlock->pKThreadRoutine      =  pStartAddress;  //Members initialization.
	pControlBlock->dwKThreadPriority    =  dwPriority;
	pControlBlock->pData                =  pData;
	pControlBlock->wCurrentMsgNum       =  0x0000;
	pControlBlock->wHeader              =  0x0000;
	pControlBlock->wTrial               =  0x0000;

	//DisableInterrupt();                   //Disable interrupt.
	                                      // ****************
	for(dwKThreadID = 0;dwKThreadID < MAX_KTHREAD_NUM;dwKThreadID ++)
	{
		if(0 == g_bKThreadQueueStatus[dwKThreadID])  //Find a free control block slot.
		{
			bFind = TRUE;
			break;
		}
	}

	if(FALSE == bFind)                    //If can not find a free control block slot.
	{
		KMemFree((LPVOID)pControlBlock,KMEM_SIZE_TYPE_4K,dwStackSize);  //Free the memory.
		return 0L;
	}
	
	DisableInterrupt();                   //Disable interrupt.
	                                      //******************
	g_pKThreadQueue[dwKThreadID]  = pControlBlock;
	g_bKThreadQueueStatus[dwKThreadID] = 1;          //Set the occupied flag.
	dwKThreadID ++;                       //Increase the KThreadID.
	pControlBlock->dwKThreadID  = dwKThreadID;
	pControlBlock->dwStackSize  = dwStackSize;

	switch(dwFlags)                      //Update the proper kernal thread queues.
		                                 //Insert the created kernal thread into
										 //the status queue.
	{
	case KTHREAD_STATUS_BLOCKED:         //Insert into blocked queue.
		pControlBlock->dwKThreadStatus = KTHREAD_STATUS_BLOCKED;
		pControlBlock->pNext = g_pBlockedQueue;
		g_pBlockedQueue = pControlBlock;
		break;
	case KTHREAD_STATUS_SUSPEND:         //Insert it into suspended queue.
		pControlBlock->dwKThreadStatus = KTHREAD_STATUS_SUSPEND;
		pControlBlock->pNext = g_pSuspendQueue;
		g_pSuspendQueue = pControlBlock;
		break;
	default:                             //All other status,insert into ready queue.
	case KTHREAD_STATUS_READY:
	case KTHREAD_STATUS_RUNNING:
		pControlBlock->dwKThreadStatus = KTHREAD_STATUS_READY;
		pControlBlock->pNext = g_pReadyQueue;
		g_pReadyQueue = pControlBlock;
		break;
	}

#ifdef __I386__                           //Update the x86 CPU's context.
	pControlBlock->dwESP = (DWORD)pControlBlock + dwStackSize;
	pControlBlock->dwEIP = (DWORD)pStartAddress;
	pControlBlock->dwEFlags = 512;        //*****************************
#else
#endif
	EnableInterrupt();                    //Enable interrupt.
	                                      //*****************
	return dwKThreadID;
}

//
//The following function terminal a kernal thread roughly.
//
VOID TerminalKThread(DWORD dwKThreadID)
{
	__KTHREAD_CONTROL_BLOCK* pControlBlock = NULL;
	DWORD                    dwStackSize   = 0;

	if((dwKThreadID < 1) || (dwKThreadID > MAX_KTHREAD_NUM))  //Parameter check.
		return;
	dwKThreadID --;                       //Make sure the dwKThreadID is the index
	                                      //to allocate the system array.
	g_bKThreadQueueStatus[dwKThreadID] = 0;  //Free the control block slot.
	pControlBlock = g_pKThreadQueue[dwKThreadID];
	if(NULL != pControlBlock)
	{
		dwStackSize = pControlBlock->dwStackSize;
		KMemFree((LPVOID)pControlBlock,KMEM_SIZE_TYPE_4K,dwStackSize);  //Free the memory.
	}
	return;
}

//
//Get kernal thread's control block by it's ID.
//If failed,returns NULL,else,returns the pointer of kernal thread's
//control block.
//
__KTHREAD_CONTROL_BLOCK* GetKThreadControlBlock(DWORD dwKThreadID)
{
	__KTHREAD_CONTROL_BLOCK*  pControlBlock = NULL;

	if((0 == dwKThreadID) || (MAX_KTHREAD_NUM == dwKThreadID))  //Parameter check.
	{
		return pControlBlock;
	}
	dwKThreadID --;
	pControlBlock = g_pKThreadQueue[dwKThreadID];
	return pControlBlock;
}

//
//The following function reset the kernal thread's priority by it's ID.
//
VOID SetKThreadPriority(DWORD dwKThreadID,DWORD dwPriority)
{
	if((dwKThreadID < 1) || (dwKThreadID > MAX_KTHREAD_NUM))  //Parameter check.
		return;

	g_pKThreadQueue[dwKThreadID - 1]->dwKThreadPriority = dwPriority;
	return;
}

//
//The following function switch to the kernal thread identified by it's control
//block.
//The parameter,pContext,pointing to the CPU context information of the transmited
//kernal thread.
//

__declspec(naked) static VOID SwitchKThread(LPVOID pContext)
{
#ifdef __I386__                           //Intel's x86 CPU implementation.
	__asm{
		cli
		push ebp
		mov ebp,esp                             //Build the stack frame.
		mov eax,dword ptr [ebp + 0x08]          //Now,eax countains the pContext value.

		mov esp,dword ptr [eax + 28]            //Set the esp value to transmited kernal thread.
		push dword ptr [eax + 36]               //Push EFlags register to stack.
		xor ebx,ebx
		mov bx,0x08
		push ebx                                //Extend and push CS to stack.
		push dword ptr [eax + 32]               //Push EIP to stack.
		                                        //Now,we have built the correct stack frame.

	    push dword ptr [eax]                    //Save eax to stack.
		push dword ptr [eax + 4]                //Save ebx to stack.
		push dword ptr [eax + 8]                //ecx
		push dword ptr [eax + 12]               //edx
		push dword ptr [eax + 16]               //esi
		push dword ptr [eax + 20]               //edi
		push dword ptr [eax + 24]               //ebp

		mov ebp,dword ptr [esp]                 //Restore the ebp register.
		add esp,0x04

		mov edi,dword ptr [esp]                 //Restore edi
		add esp,0x04

		mov esi,dword ptr [esp]                 //Restore esi
		add esp,0x04

		mov edx,dword ptr [esp]                 //Restore edx
		add esp,0x04

		mov ecx,dword ptr [esp]                 //Restore ecx
		add esp,0x04

		mov ebx,dword ptr [esp]                 //Restore ebx
		add esp,0x04                            
		
		mov al,0x20
		out 0x20,al
		out 0xa0,al

		mov eax,dword ptr [esp]                 
		add esp,0x04                            //Now,the esp pointer points to the current
		                                        //position.
		sti
		iretd
		retn                                   //This instruction will never be reached.
	}
#else
#endif
}

//
//The following function save the current kernal thread's context.
//The parameter,dwEsp,is the esp register's value before the TimerHandler is called,
//the general registers' value of the interrupted kernal thread are saved into stack,
//the stack frame,before the TimerHandler is called,as following:
//EFlags / (DWORD)CS / EIP / EAX / EBX / ECX / EDX / ESI / EDI / EBP / ESP,where ESP
//is the value after the EBP is pushed.
//So,we can access the general registers' value of the current kernal thread by dwEsp
//parameter.
//

static VOID SaveKThreadContext(__KTHREAD_CONTROL_BLOCK* pControlBlock,DWORD dwEsp)
{
	DWORD* pdwEsp = (DWORD*)dwEsp;

	if(NULL == pControlBlock)             //Parameter check.
		return;

#ifdef __I386__
	pControlBlock->dwEBP = *pdwEsp;
	pdwEsp ++;
	pControlBlock->dwEDI = *pdwEsp;
	pdwEsp ++;
	pControlBlock->dwESI = *pdwEsp;
	pdwEsp ++;
	pControlBlock->dwEDX = *pdwEsp;
	pdwEsp ++;
	pControlBlock->dwECX = *pdwEsp;
	pdwEsp ++;
	pControlBlock->dwEBX = *pdwEsp;
	pdwEsp ++;
	pControlBlock->dwEAX = *pdwEsp;
	pdwEsp ++;
	pControlBlock->dwEIP = *pdwEsp;
	pdwEsp ++;
	pdwEsp ++;                            //Skip the CS's space.
	pControlBlock->dwEFlags = *pdwEsp;
	pdwEsp ++;
	pControlBlock->dwESP = (DWORD)pdwEsp;
#else
#endif
}

//
//The following function re-schedule the kernal thread.
//It saves the current kernal thread's context,
//select a proper kernal thread,and schedule it to run.
//

VOID ScheduleKThread(DWORD dwEsp)              //The dwEsp parameter is the esp value
                                               //before the TimerHandler is called.
											   //We can access the current kernal
											   //thread's context,such as general
											   //registers by this parameter.
{
	DWORD dwKThreadID = g_dwCurrentKThreadID;
	DWORD dwIndex     = 0;
	__KTHREAD_CONTROL_BLOCK* pControlBlock     = NULL;
	__KTHREAD_CONTROL_BLOCK* pCurrControlBlock = NULL;
	LPVOID                   pContext          = NULL;
	//BYTE              Buffer[12];   //---------- ** debug ** ------------
	//static DWORD             dwTmp = 0;

	//dwTmp ++;

	//DisableInterrupt();          //Disable interrupt.
	                             //******************
	if(NULL == g_pReadyQueue)    //If there is not any ready kernal thread.
		goto __TERMINAL;

	if(0 == dwKThreadID)
	{
		goto __TERMINAL;
	}

	pControlBlock = g_pKThreadQueue[dwKThreadID - 1];    //Get the current kernal thread's 
	                                                     //control block.
	pCurrControlBlock = pControlBlock;

	if(NULL == pControlBlock->pNext)
		pControlBlock = g_pReadyQueue;
	else
	{
		if(KTHREAD_STATUS_READY == pControlBlock->pNext->dwKThreadStatus)
			pControlBlock = pControlBlock->pNext;
		else
			pControlBlock = g_pReadyQueue;
	}

	if(NULL == pControlBlock)                            //If there is not any ready kernal
		                                                 //thread to schedule.
		goto __TERMINAL;

	if(KTHREAD_STATUS_READY == pControlBlock->dwKThreadStatus)
	{
		/*if(dwTmp % 10 == 0)
		{
			PrintLine("------------- ** debug ** -----------------");  //---- ** debug ** ----
		    Hex2Str((DWORD)pCurrControlBlock,Buffer);
		    PrintLine(Buffer);
		    Hex2Str((DWORD)pControlBlock,Buffer);
		    PrintLine(Buffer);
		}*/

		SaveKThreadContext(pCurrControlBlock,dwEsp);
		g_dwCurrentKThreadID = pControlBlock->dwKThreadID;
#ifdef __I386__
		pContext = (LPVOID)&pControlBlock->dwEAX;
#else
#endif
		SwitchKThread(pContext);
		goto __TERMINAL;
	}

__TERMINAL:
	//EnableInterrupt();
	return;
}

