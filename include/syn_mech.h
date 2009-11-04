/***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jul,26 2005
//    Module Name               : SYN_MECH.H
//    Module Funciton           : 
//                                This module countains synchronization code for system kernel.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __SYN_MECH_H__
#define __SYN_MECH_H__
#endif


#define __ENTER_CRITICAL_SECTION(lpObj,dwFlags)

#define __LEAVE_CRITICAL_SECTION(lpObj,dwFlags)

/*
#ifdef __I386

	#define __ENTER_CRITICAL_SECTION(lpObj,dwFlags) \
		__asm push eax             \
		__asm pushfd               \
		__asm pop eax              \
		__asm mov dwFlags,eax      \
		__asm pop eax              \
		__asm cli                  \


	#define __LEAVE_CRITICAL_SECTION(lpObj,dwFlags) \
	    __asm push dwFlags \
	    __asm popfd                         
#endif
*/
#define _ENTER_CRITICAL_SECTION(lpObj)    \
    //
    //SpinLock code here.
    //

#define _LEAVE_CRITICAL_SECTION(lpObj)    \
    //
    //SpinLock code here.
    //

#define __BARRIER(ptr)      //Barrier operation.

//
//This macros is used to flush cache's content to memory.
//
/*
#ifdef __I386
	#define FLUSH_CACHE()  \
		__asm wbinvd
	#else
	#define FLUSH_CACHE
#endif
*/

//#define SYNCHRONIZE_MEMORY() FLUSH_CACHE()

//
//This macros is used as a barrier.
//In some CPU,the write operations may not be committed into memory or cache,but 
//be queued in WRITE BUFFER on the CPU,so,in order to commit the write operation
//immediately,the following macro should be used.
//For example,the following code:
// ... ...
// WriteMemory(0xCFFFFFFF,90);
// WriteMemory(0xCFFFFFFC,90);
// ... ...
//0xCFFFFFFF and 0xCFFFFFFC are device mapped control port,in order to commit the
//writing operations into device immediately,the following macro must be called.
//
#ifdef __I386
	#define BARRIER() \
		__asm LOCK add dword ptr [esp],0
	#else
	#define BARRIER()
#endif


/*#define ENTER_CRITICAL_SECTION() \
    DWORD       dwFlags = 0L;    \
    __ENTER_CRITICAL_SECTION(NULL,dwFlags)

#define LEAVE_CRITICAL_SECTION() \
    __LEAVE_CRITICAL_SECTION(NULL,dwFlags)*/

//BEGIN_DEFINE_OBJECT(__ATOMIC_T)
//    DWORD          dwCounter;
//END_DEFINE_OBJECT()

#define __ATOMIC_T DWORD

#define __INIT_ATOMIC(t) (t) = 0

/*#ifdef __INLINE_ENABLE
#define INLINE inline
#else
#define INLINE
#endif*/

#define INLINE
#define __INLINE inline

//
//The definition of atomic operating routines.
//
INLINE DWORD AtomicGet(__ATOMIC_T* lpAtomic);
INLINE VOID AtomicSet(__ATOMIC_T* lpAtomic,DWORD dw);
INLINE DWORD AtomicAdd(__ATOMIC_T* lpAtomic,DWORD dw);
INLINE DWORD AtomicSub(__ATOMIC_T* lpAtomic,DWORD dw);
INLINE BOOL AtomicInc(__ATOMIC_T* lpAtomic);
INLINE DWORD AtomicDec(__ATOMIC_T* lpAtomic);

//
//Spin lock's definition.
//

#ifdef __SMP
#define spin_lock(lock) \
	TryGetLock(&(lock))
#define spin_unlock(lock) \
	ReleaseLock(&(lock))
#else
#define spin_lock(lock)
#define spin_unlock(lock)
#endif

