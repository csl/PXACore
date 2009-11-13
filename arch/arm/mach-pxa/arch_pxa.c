/***********************************************************************/
//    Author                    : Garry
//    Original Date             : Sep,18 2006
//    Module Name               : ARCH_X86.CPP
//    Module Funciton           : 
//                                This module countains CPU specific code,in this file,
//                                Intel X86 series CPU's specific code is included.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include "stdafx.h"
#include "l_stdio.h"

#include "pxa255.h"

static int count=0;


//#define MIN_STACK_SIZE 128
//
//This value is the default stack size of a kernel thread in IA32 platform,
//if the user does not give a stack size in CreateKernelThread calling,then
//use this value as stack size.
//
#define DEFAULT_STACK_SIZE 0x00004000 //16k bytes.
#define PXA255_TMR_CLK 3686400
#define OS_TICKS_PER_SEC 10

//
//Initializes the context of a kernel thread.
//The initialization process is different on different platforms,so
//implement this routine in ARCH directory.
//
DWORD* InitKernelThreadContext(struct __KERNEL_THREAD_OBJECT* lpKernelThread);


//
//This routine switches the current executing path to the new one identified
//by lpContext.
//
/*
__declspec(naked) VOID __SwitchTo(__KERNEL_THREAD_CONTEXT* lpContext)
{
	__asm{
		push ebp
		mov ebp,esp
		mov esp,dword ptr [ebp + 0x08]  //Restore ESP.
		pop ebp
		pop edi
		pop esi
		pop edx
		pop ecx
		pop ebx

		mov al,0x20  //Dismiss interrupt controller.
		out 0x20,al
		out 0xa0,al

		pop eax
		iretd
	}
}
*/
//
//These three global variables are used as temp variables
//by __SaveAndSwitch routine.
//
static DWORD dwTmpEip = 0;
static DWORD dwTmpEax = 0;
static DWORD dwTmpEbp = 0;

//
//This routine saves current kernel thread's context,and switch
//to the new kernel thread.
//
/*
__declspec(naked) VOID __SaveAndSwitch(__KERNEL_THREAD_CONTEXT** lppOldContext,
									   __KERNEL_THREAD_CONTEXT** lppNewContext)
{
	__asm{
		mov dwTmpEbp,esp
		pop dwTmpEip
		mov dwTmpEax,eax //Save EAX.
		pushfd           //Save EFlags.
		xor eax,eax
		mov ax,cs
		push eax         //Save CS.
		push dwTmpEip    //Save EIP.
		push dwTmpEax    //Save EAX.
		push ebx
		push ecx
		push edx
		push esi
		push edi
		push ebp

		//Now,we have build the target stack frame,then save it.
		mov ebp,dwTmpEbp
		mov ebx,dword ptr [ebp + 0x04]
		mov dword ptr [ebx],esp  //Save old stack frame.

		//Restore the new thread's context and switch to it.
		mov ebx,dword ptr [ebp + 0x08]
		mov esp,dword ptr [ebx]  //Restore new stack.
		pop ebp
		pop edi
		pop esi
		pop edx
		pop ecx
		pop ebx
		pop eax
		iretd
	}
}
*/
//
//This routine initializes a kernel thread's context.
//This routine's action depends on different platform.
//
DWORD* InitKernelThreadContext(struct __KERNEL_THREAD_OBJECT* lpKernelThread)
{
	DWORD*        lpStackPtr = NULL;

	lpStackPtr = lpKernelThread->lpInitStackPointer;
	*(lpStackPtr) = 0x0;
	*(--lpStackPtr) = (DWORD) lpKernelThread->KernelThreadRoutine; /* r15 (pc) thread address */
	*(--lpStackPtr) = (DWORD) 0x14141414L;	/* r14 (lr) */
	*(--lpStackPtr) = (DWORD) 0x12121212L;	/* r12 */
	*(--lpStackPtr) = (DWORD) 0x11111111L;	/* r11 */
	*(--lpStackPtr) = (DWORD) 0x10101010L;	/* r10 */
	*(--lpStackPtr) = (DWORD) 0x09090909L;	/* r9 */
	*(--lpStackPtr) = (DWORD) 0x08080808L;	/* r8 */
	*(--lpStackPtr) = (DWORD) 0x07070707L;	/* r7 */
	*(--lpStackPtr) = (DWORD) 0x06060606L;	/* r6 */
	*(--lpStackPtr) = (DWORD) 0x05050505L;	/* r5 */
	*(--lpStackPtr) = (DWORD) 0x04040404L;	/* r4 */
	*(--lpStackPtr) = (DWORD) 0x03030303L;	/* r3 */
	*(--lpStackPtr) = (DWORD) 0x02020202L;	/* r2 */
	*(--lpStackPtr) = (DWORD) 0x01010101L;	/* r1 */
	*(--lpStackPtr) = (DWORD) 0x0;		/* r0 */
	*(--lpStackPtr) = (DWORD) 0x13;		/* cpsr : sys_mode */

#ifdef  DEBUG
	printf("InitKernelThreadContext\n");
	printf("lpStackPtr = %x %x, KernelThreadRoutine = %x\n", lpStackPtr, lpKernelThread->lpInitStackPointer, lpKernelThread->KernelThreadRoutine);
#endif
	return lpStackPtr;
}

VOID RestoreKernelThread(struct __KERNEL_THREAD_OBJECT* lp)
{
	printf("%x %x\n", lp->lpInitStackPointer, lp->KernelThreadRoutine);

	asm ( 
		"mov sp, %0\n\t"		// restore current thread's context
		"ldr r4, [sp], #4\n\t"
		"msr SPSR_cxsf, r4\n\t"
		"mrs r4, SPSR\n\t"
		"ldmfd sp!, {r0-r12, lr, pc}^\n\t"	//jump to Kernel Thread
		: 
		: "r" (lp->lpInitStackPointer)
	);
}

//context_switch
VOID __SaveAndSwitch(struct __KERNEL_THREAD_OBJECT* lpPrev, struct __KERNEL_THREAD_OBJECT* lpNex)
{
	printf("__SaveAndSwitch\n");
	asm ( 
		"stmfd sp!, {lr}\n\t"		// save current thread's context
		"stmfd sp!, {lr}\n\t"
		"stmfd sp!, {r0-r12, lr}\n\t"
		"mrs r4, SPSR\n\t"
		"stmfd sp!, {r4}\n\t"
	
		//"ldr r4, =%0\n\t"
		"mov sp, %0\n\t"		// current_thread->stack_ptr = sp
/*	
		"ldr r4, =lpPrev\n\t"		// current_thread = next_thread
		"ldr r6, =lpNex\n\t"
		"ldr r6, [r6]\n\t"
		"str r6, [r4]\n\t"
*/
//		"ldr r4, =%1\n\t"
		"mov sp, %1\n\t"		// sp = next_thread->sp
		
		"ldmfd sp!, {r4}\n\t"		// restore next thread's context
		"msr SPSR_cxsf, r4\n\t"
		"ldmfd sp!, {r0-r12, lr, pc}^\n\t"

		: 
		: "r" (lpPrev->lpInitStackPointer),
		  "r" (lpNex->lpInitStackPointer)
	);
}


VOID __Interrupt_Handler(void)
{
	
	/*
	#define NO_INT 0xc0
	#define NO_IRQ 0x80
	#define NO_FIQ 0x40
	#define SVC32_MODE 0x13
	#define FIQ32_MODE 0x11
	#define IRQ32_MODE 0x12
	*/
	//printf("count = %d\n", count);

	asm ( 	
		//change to IRQ stack
		"msr CPSR_c, #(0xc0 | 0x12)\n\t"  //CPSR_c (0:7 bits), IRQ stack
		"stmfd sp!, {r1-r3}\n\t"	  // push working registers onto "IRQ stack"
		"mov r1, sp\n\t"	
		"add sp, sp, #12\n\t"	  //IRQ stack: sp = sp + 12 (3 items)
		"sub r2, lr, #4\n\t"	  //r2 = lr - 4 (return address)
		"mrs r3, SPSR\n\t"	  //r3 = SPSR

		//change to SVC mode
		"msr CPSR_c, #(0xc0 | 0x13)\n\t"     //sp = thread stack
		"stmfd sp!, {r2}\n\t"		     //r2 = return address (pc)
		"stmfd sp!, {lr}\n\t"		     //lr (r14)
		"stmfd sp!, {r4-r12}\n\t"

		//pop "IRQ stack" to working registers (r1-r3)
		"ldmfd r1!, {r4-r6}\n\t" //move thread'sfrom IRQ stack to SVC stack
		"stmfd sp!, {r4-r6}\n\t" //sp = thread stack (r1-r3)
		"stmfd sp!, {r0}\n\t"	 //push thread's r0 onto "thread's stack"
		"stmfd sp!, {r3}\n\t"	 //push thread's CPSR (IRQ's SPSR)
		//Determind interrupt_nesting -> FIQ
		//change to FIQ stack

/*
		"ldr r0, =interrupt_nesting\n\t"
		"ldrb r1, [r0]\n\t"		
		"cmp r1, #1\n\t"		//if (interrupt_nesting != 1)
		"bne i_r_q\n\t"		//	goto i_r_q

		"ldr r4, [%0]\n\t"		// current_thread->stack_ptr = sp
		"add r4, r4, #8\n\t"
		"str sp, [r4]\n\t"
*/
//"i_r_q:\n\t"
		// re-enable FIQ, chagen to IRQ mode
		"msr CPSR_c, #(0x80 | 0x12)\n\t"	//NO_IRQ | IRQ32_MODE
		"bl InterruptHandler\n\t"
		//chagen to SVC32
		"msr CPSR_c, #(0xc0 | 0x13)\n\t"	//NO_INT | SVC32_MODE
		//"bl ExitInterrupt\n\t"
		"ldmfd sp!, {r4}\n\t"
		"msr SPSR_cxsf, r4\n\t"
/*
		"ldmfd sp!, {r0}\n\t"
		"ldmfd sp!, {r1}\n\t"
		"ldmfd sp!, {r2}\n\t"
		"ldmfd sp!, {r3}\n\t"
		"ldmfd sp!, {r4}\n\t"
		"ldmfd sp!, {r5}\n\t"
		"ldmfd sp!, {r6}\n\t"
		"ldmfd sp!, {r7}\n\t"
		"ldmfd sp!, {r8}\n\t"
		"ldmfd sp!, {r9}\n\t"
		"ldmfd sp!, {r10}\n\t"
		"ldmfd sp!, {r11}\n\t"
		"ldmfd sp!, {r12}\n\t"
		"ldmfd sp!, {lr}\n\t"
		"ldmfd sp!, {pc}\n\t"
*/
		//"bb: b bb\n\t"
		"mov r1, %0\n\t"
		"cmp r1,#10\n\t"
		"bne bbb\n\t"
		"aaa:b aaa\n\t"
"bbb:\n\t"
		"ldmfd sp!, {r0-r12, lr, pc}^\n\t"
		: 
		: "r" (count)

	);

	
}


VOID DisableInterrupt(VOID)
{
	INT_REG(INT_ICMR) = 0;
}


VOID EnableInterrupt(VOID)
{
	//printf("EnableINterrupt\n");
	INT_REG(INT_ICLR) &= ~BIT26;
	TMR_REG(TMR_OSMR0) = PXA255_TMR_CLK / OS_TICKS_PER_SEC;
	TMR_REG(TMR_OSMR1) = 0x3FFFFFFF;
	TMR_REG(TMR_OSMR2) = 0x7FFFFFFF;
	TMR_REG(TMR_OSMR3) = 0xBFFFFFFF;
	TMR_REG(TMR_OSCR) = 0x00;
	TMR_REG(TMR_OSSR) = BIT0;
	TMR_REG(TMR_OIER) = BIT0;
	INT_REG(INT_ICMR) |= BIT26;
}

void ExitInterrupt()
{
/*	if (interrupt_nesting > 0) 
		interrupt_nesting--;

	if (current_thread->time_quantum <= 0) 
	{
		//schedule(SCHED_TIME_EXPIRE);
	}
*/
}

void InterruptHandler()
{
	//printf("interrupt_handler function\n");

	//handler timer
	if (INT_REG(INT_ICIP) & BIT26) 
	{
		TMR_REG(TMR_OSCR) = 0x00;
		count++;
		//advance_time_tick();
		TMR_REG(TMR_OSSR) = BIT0;
	}
}

/*
VOID InitKernelThreadContext(__KERNEL_THREAD_OBJECT* lpKernelThread,
			     __KERNEL_THREAD_WRAPPER lpStartAddr)
{
	DWORD*        lpStackPtr = NULL;
	DWORD         dwStackSize = 0;

	if((NULL == lpKernelThread) || (NULL == lpStartAddr))  //Invalid parameters.
	{
		return;
	}

//Define a macro to make the code readable.
#define __PUSH(stackptr,val) \
	do{  \
	(DWORD*)(stackptr) -= 1; \
	*((DWORD*)stackptr) = (DWORD)(val); \
	}while(0)

	lpStackPtr = (DWORD*)lpKernelThread->lpInitStackPointer;

	__PUSH(lpStackPtr,lpKernelThread);       //Push lpKernelThread to stack.
	__PUSH(lpStackPtr,NULL);                 //Push a new return address,simulate a call.
	__PUSH(lpStackPtr,INIT_EFLAGS_VALUE);    //Push EFlags.
	__PUSH(lpStackPtr,0x00000008);           //Push CS.
	__PUSH(lpStackPtr,lpStartAddr);  //Push start address.
	__PUSH(lpStackPtr,0L);                   //Push eax.
	__PUSH(lpStackPtr,0L);
	__PUSH(lpStackPtr,0L);
	__PUSH(lpStackPtr,0L);
	__PUSH(lpStackPtr,0L);
	__PUSH(lpStackPtr,0L);
	__PUSH(lpStackPtr,0L);

	//Save context.
	lpKernelThread->lpKernelThreadContext = (__KERNEL_THREAD_CONTEXT*)lpStackPtr;
	return;
}

//Get time stamp counter.
VOID __GetTsc(__U64* lpResult)
{
	__asm{
		push eax
		push edx
		push ebx
		rdtsc    //Read time stamp counter.
		mov ebx,dword ptr [ebp + 0x08]
		mov dword ptr [ebx],eax
		mov dword ptr [ebx + 0x04],edx
		pop ebx
		pop edx
		pop eax
	}
}

#define CLOCK_PER_MICROSECOND 1024  //Assume the CPU's clock is 1G Hz.

VOID __MicroDelay(DWORD dwmSeconds)
{
	__U64    u64CurrTsc;
	__U64    u64TargetTsc;

	__GetTsc(&u64TargetTsc);
	u64CurrTsc.dwHighPart = 0;
	u64CurrTsc.dwLowPart  = dwmSeconds;
	//u64Mul(&u64CurrTsc,CLOCK_PER_MICROSECOND);
	u64RotateLeft(&u64CurrTsc,10);
	u64Add(&u64TargetTsc,&u64CurrTsc,&u64TargetTsc);
	while(TRUE)
	{
		__GetTsc(&u64CurrTsc);
		if(MoreThan(&u64CurrTsc,&u64TargetTsc))
		{
			break;
		}
	}
	return;
}
*/
