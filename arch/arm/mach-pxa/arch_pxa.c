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
	/* r15 (pc) thread address */
	*(--lpStackPtr) = (DWORD) lpKernelThread->KernelThreadRoutine; 
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

#ifdef  DEBUG
	printf("lpInitStackPointer = %x, KernelThreadRoutine = %x\n", lp->lpInitStackPointer, lp->KernelThreadRoutine);
#endif

	asm ( 
		"mov sp, %0\n\t"		// restore current thread's context
		"ldr r4, [sp], #4\n\t"
		"msr SPSR_cxsf, r4\n\t"
		//"mrs r4, SPSR\n\t"
		"ldmfd sp!, {r0-r12, lr, pc}^\n\t"	//jump to Kernel Thread
		: 
		: "r" (lp->lpInitStackPointer)
	);
}

/*
//context_switch
VOID __SaveAndSwitch(struct __KERNEL_THREAD_OBJECT* lpPrev, struct __KERNEL_THREAD_OBJECT* lpNex)
{
	asm ( 
		"stmfd sp!, {lr}\n\t"		// save current thread's context
		"stmfd sp!, {lr}\n\t"
		"stmfd sp!, {r0-r12, lr}\n\t"
		"mrs r4, SPSR\n\t"
		"stmfd sp!, {r4}\n\t"
	
		//"ldr r4, =%0\n\t"
		"mov sp, %0\n\t"		// current_thread->stack_ptr = sp
	
		"ldr r4, =lpPrev\n\t"		// current_thread = next_thread
		"ldr r6, =lpNex\n\t"
		"ldr r6, [r6]\n\t"
		"str r6, [r4]\n\t"

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
*/

void SetSP(LPVOID sp_addr)
{
#ifdef  DEBUG
	printf("\nsp: new:%x old: %x\n", sp_addr,KernelThreadManager.lpCurrentKernelThread->lpInitStackPointer);
#endif
	KernelThreadManager.lpCurrentKernelThread->lpInitStackPointer = sp_addr;
	
}

VOID DisableInterrupt(VOID)
{
	INT_REG(INT_ICMR) = 0;
}

VOID EnableInterrupt(VOID)
{
	//printf("Enable Interrupt\n");
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
/*
	if (interrupt_nesting > 0) 
		interrupt_nesting--;
*/
#ifdef  DEBUG
	printf("\ntimer count = %d\n", KernelThreadManager.lpCurrentKernelThread->dwTotalRunTime);
#endif
	
	KernelThreadManager.lpCurrentKernelThread->dwTotalRunTime++;
	if (KernelThreadManager.lpCurrentKernelThread->dwTotalRunTime % 100 == 0) 
	{
		KernelThreadManager.ScheduleFromInt();
	}

}

void InterruptHandler()
{
	//hander for timer
	if (INT_REG(INT_ICIP) & BIT26) 
	{
		TMR_REG(TMR_OSCR) = 0x00;
		
		TMR_REG(TMR_OSSR) = BIT0;
	}
}

/*
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
