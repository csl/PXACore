//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Aug,05 2005
//    Module Name               : SYN_MECH.CPP
//    Module Funciton           : 
//                                This module countains synchronization code for system kernel.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __STDAFX_H__
#include "..\INCLUDE\StdAfx.H"
#endif

//
//Atomically get.Get the atomic's value atomically.
//
INLINE DWORD AtomicGet(__ATOMIC_T* lpAtomic)
{
#ifdef __I386
	__asm
	{
		push ebx
		mov ebx,lpAtomic
		lock mov eax,dword ptr [ebx]
		pop ebx
		retn
	}
#else
#endif
}

//
//Atomically set.
//
INLINE VOID AtomicSet(__ATOMIC_T* lpAtomic,DWORD dw)
{
#ifdef __I386
	__asm
	{
		push eax
		push ebx
		mov ebx,lpAtomic
		mov eax,dw
		lock mov dword ptr [ebx],eax
		pop ebx
		pop eax
		//retn
	}
#else
#endif
}
//
//Atomically add dw to an atomic,that is,*lpAtomic += dw.
//

INLINE DWORD AtomicAdd(__ATOMIC_T* lpAtomic,DWORD dw)
{
	if(NULL == lpAtomic)    //Parameter check.
		return 0L;

#ifdef __I386
	__asm{
		push eax
		push ebx
		mov eax,dw
		mov ebx,lpAtomic
		lock add dword ptr [ebx],eax
		pop ebx
		pop eax
	}
#else
#endif
	return 0L;
}

//
//Atomically sub dw from an atomic,that is,*lpAtomic -= dw.
//

INLINE DWORD AtomicSub(__ATOMIC_T* lpAtomic,DWORD dw)
{
	if(NULL == lpAtomic)    //Parameter check.
		return 0L;

#ifdef __I386
	__asm{
		push eax
		push ebx
		mov eax,dw
		mov ebx,lpAtomic
		lock sub dword ptr [ebx],eax
		pop ebx
		pop eax
	}
#else
#endif
	return 0L;
}

//
//Increment an atomic,if the new value of *lpAtomic == 0,then returns TRUE,else,
//returns FALSE.
//

INLINE BOOL AtomicInc(__ATOMIC_T* lpAtomic)
{
	if(NULL == lpAtomic)
		return 0L;

#ifdef __I386
	__asm{
		push ebx
		mov ebx,lpAtomic
		inc dword ptr [ebx]
		jne __NO_ZERO
		mov eax,TRUE
		pop ebx
		retn
__NO_ZERO:
		pop ebx
	}
#else
#endif
	return FALSE;
}

//
//Decrement an atomic,if the result is zero,then returns TRUE,else,returns FALSE.
//

INLINE DWORD AtomicDec(__ATOMIC_T* lpAtomic)
{
	if(NULL == lpAtomic)
		return 0L;

#ifdef __I386
	__asm{
		push ebx
		mov ebx,lpAtomic
		dec dword ptr [ebx]
		jne __NO_ZERO
		mov eax,TRUE
		pop ebx
		retn
__NO_ZERO:
		pop ebx
	}
#else
#endif
	return 0L;
}

//
//Lock operation and it's definition.
//The following code is used in case of SMP,so,these routines will never be used in current
//version of Hello Taiwan.
//

#define __LOCK_T DWORD
#define __INIT_LOCK(l) (l) = 0L

//
//Test the status of a lock.If the lock is occupied,returns FALSE,else,if the lock
//is free,then get the lock,and returns TRUE.
//

INLINE BOOL TestLock(__LOCK_T* lpAtomic)
{
#ifdef __I386
	__asm{
		push ebx
		mov ebx,lpAtomic
		bts dword ptr [ebx],0
		jc __OCCUPIED
		mov eax,TRUE
		pop ebx
		retn
__OCCUPIED:
		mov eax,FALSE
		pop ebx
		retn
	}
#else
#endif
}

//
//Release lock.
//

INLINE VOID ReleaseLock(__LOCK_T* lpLock)
{
#ifdef __I386
	__asm{
		push ebx
		mov ebx,lpLock
		btr dword ptr [ebx],0
		pop ebx
		retn
	}
#else
#endif
}

//
//Try to get a lock.This routine will not return until get the lock successfully.
//

INLINE VOID TryGetLock(__LOCK_T* lpLock)
{
	while(!TestLock(lpLock));
}
