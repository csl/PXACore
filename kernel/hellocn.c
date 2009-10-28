//***********************************************************************/
//    Author                    : Garry
//    Original Date             : May,27 2004
//    Module Name               : hellocn.cpp
//    Module Funciton           : 
//                                This module countains the source code.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

//#ifndef __HELLO_TAIWAN__
//#include "hellocn.h"
//#endif

#ifndef __STDAFX_H__
#include "StdAfx.h"
#endif

void PrintStr(const char* pszMsg)
{
#ifdef __I386__                  //I386 implementation.
	__asm{
		push ebx
		push ecx
		mov ebx,__PRINTSTR_BASE
		mov ecx,dword ptr [ebx]
		mov eax,pszMsg
		push eax
		call ecx
		pop eax
		pop ecx
		pop ebx
	}
#else
#endif
}

void ClearScreen()
{
#ifdef __I386__
	__asm{
		push ebx
		mov ebx,__CLEARSCREEN_BASE
		call dword ptr [ebx]
		pop ebx
	}
#else
#endif
}

void PrintCh(unsigned short ch)
{
#ifdef __I386__
	__asm{
		push ebx
		mov ebx,__PRINTCH_BASE
		mov ax,word ptr [ebp + 0x08]
		push eax
		call dword ptr [ebx]
		pop eax
		pop ebx
	}
#else
#endif
}

void GotoHome()
{
#ifdef __I386__
	__asm{
		push ebx
		mov ebx,__GOTOHOME_BASE
		call dword ptr [ebx]
		pop ebx
	}
#else
#endif
}

void ChangeLine()
{
#ifdef __I386__
	__asm{
		push ebx
		mov ebx,__CHANGELINE_BASE
		call dword ptr [ebx]
		pop ebx
	}
#else
#endif
}

KEY_HANDLER SetKeyHandler(KEY_HANDLER keyHandler)
{
#ifdef __I386__
	__asm
	{
		push ebx
        push ecx
		mov ebx,__SETNOTIFYOS_BASE
		mov ecx,keyHandler
		push ecx
		call dword ptr [ebx]
		pop ecx
		pop ecx
		pop ebx
	}
#else
#endif
}

VOID GotoPrev()
{
#ifdef __I386__
	__asm{
		push ebx
		mov ebx,__GOTOPREV_BASE
		call dword ptr [ebx]
		pop ebx
	}
#else
#endif
}

INT_HANDLER SetTimerHandler(__GENERAL_INTERRUPT_HANDLER TimerHandler)
{
#ifdef __I386__
	__asm{
		push ebx
		push ecx
		mov ebx,__TIMERHANDLER_BASE
		mov eax,TimerHandler
		mov ecx,dword ptr [ebx]
		mov dword ptr [ebx],eax
		mov eax,ecx
		pop ecx
		pop ebx
	}
#else
#endif
}

WORD SetGdtEntry(DWORD dw1,DWORD dw2)
{
#ifdef __I386__
	__asm{
		push ebx
		mov ebx,__SETGDTENTRY_BASE
		push dw2
		push dw1
		call dword ptr [ebx]
		pop dw1
		pop dw2
		pop ebx
	}
#else
#endif
}

VOID MemZero(LPVOID pMemAddress,DWORD dwSize)
{
#ifdef __I386__
	__asm{
		push ecx
		push edi
		push eax
		xor al,al
		mov ecx,dwSize
		mov edi,pMemAddress
		cld
		rep stosb
		pop eax
		pop edi
		pop ecx
	}
#else
#endif
}

VOID MemCpy(LPVOID lpDes,LPVOID lpSrc,DWORD dwSize)
{
#ifdef __I386
	__asm{
		push esi
		push edi
		push ecx
		mov esi,lpSrc
		mov edi,lpDes
		mov ecx,dwSize
		rep movsb
		pop ecx
		pop edi
		pop esi
		retn
	}
#else
#endif
}

__declspec(naked) VOID ReadByteFromPort(UCHAR* pByte,WORD wPort)
{
#ifdef __I386__
	__asm{
		push ebp
		mov ebp,esp
		push edx
		push ebx
		mov ebx,dword ptr [ebp + 8]
		mov dx,word ptr [ebp + 12]
		in al,dx
		mov byte ptr [ebx],al
		pop ebx
		pop edx
		leave
		retn
	}
#else
#endif
}

__declspec(naked) VOID WriteByteToPort(UCHAR byte,WORD wPort)
{
#ifdef __I386__
	__asm{
		push ebp
		mov ebp,esp
		push edx
		mov al,byte ptr [ebp + 8]
		mov dx,word ptr [ebp + 12]
		out dx,al
		pop edx
		leave
		retn
	}
#else
#endif
}

__declspec(naked) VOID ReadByteStringFromPort(LPVOID lpBuffer,DWORD dwBufLen,WORD wPort)
{
#ifdef __I386__
	__asm{
		push ebp
		mov ebp,esp
		//push ebx
		push ecx
		push edx
		push edi
		mov edi,dword ptr [ebp + 8]
		//mov esi,dword ptr [ebx]
		mov ecx,dword ptr [ebp + 12]
		mov dx,word ptr [ebp + 16]
		rep insb
		pop edi
		pop edx
		pop ecx
		//pop ebx
		leave
		retn
	}
#else
#endif
}

__declspec(naked) VOID WriteByteStringToPort(LPVOID lpBuffer,DWORD dwBufLen,WORD wPort)
{
#ifdef __I386__
	__asm{
		push ebp
		mov ebp,esp
		push ecx
		push edx
		push esi
		mov esi,dword ptr [ebp + 8]
		mov ecx,dword ptr [ebp + 12]
		mov dx,word ptr [ebp + 16]
		rep outsb
		pop esi
		pop edx
		pop ecx
		leave
		retn
	}
#else
#endif
}

__declspec(naked) VOID ReadWordFromPort(WORD* pWord,WORD wPort)
{
#ifdef __I386__
	__asm{
		push ebp
		mov ebp,esp
		push ebx
		push edx
		mov dx,word ptr [ebp + 0x0c]
		mov ebx,dword ptr [ebp + 0x08]
		in ax,dx
		mov word ptr [ebx],ax
		pop edx
		pop ebx
		leave
		retn
	}
#else
#endif
}

__declspec(naked) VOID WriteWordToPort(WORD,WORD)
{
#ifdef __I386__
	__asm{
		push ebp
		mov ebp,esp
		push dx
		mov dx,word ptr [ebp + 0x0c]
		mov ax,word ptr [ebp + 0x08]
		out dx,ax
		pop dx
		leave
		retn
	}
#else
#endif
}

__declspec(naked) VOID ReadWordStringFromPort(LPVOID,DWORD,WORD)
{
#ifdef __I386__
	__asm{
		push ebp
		mov ebp,esp
		push ecx
		push edx
		push edi
		mov edi,dword ptr [ebp + 0x08]
		mov ecx,dword ptr [ebp + 0x0c]
		shr ecx,0x01
		mov dx,  word ptr [ebp + 0x10]
		cld
		rep insw
		pop edi
		pop edx
		pop ecx
		leave
		retn
	}
#else
#endif
}

__declspec(naked) VOID WriteWordStringToPort(LPVOID,DWORD,WORD)
{
#ifdef __I386__
	__asm{
		push ebp
		mov ebp,esp
		push ecx
		push edx
		push esi
		mov esi,dword ptr [ebp + 0x08]
		mov ecx,dword ptr [ebp + 0x0c]
		shr ecx,0x02
		mov dx,  word ptr [ebp + 0x10]
		rep outsw
		pop esi
		pop edx
		pop ecx
		leave
		retn
	}
#else
#endif
}

//
//Interrupt flag control functions.
//

__declspec(naked) VOID DisableInterrupt()
{
#ifdef __I386__
	__asm{
		cli
		retn
	}
#else
#endif
}

__declspec(naked) VOID EnableInterrupt()
{
#ifdef __I386__
	__asm{
		sti
		retn
	}
#else
#endif
}

/*
//
//64 bit unsigned number operation's implementation.
//

//Compare two U64 numbers,if the first is larger than the second,
//returns TRUE,else,returns FALSE.
BOOL U64_LARGER(__U64* lpFirst,__U64* lpSecond)
{
	BOOL        bResult          = FALSE;

	if((NULL == lpFirst) || (NULL == lpSecond))  //Parameters check.
		return FALSE;

	if(lpFirst->dwHighPart > lpSecond->dwHighPart)
		bResult = TRUE;
	else
	{
		if((lpFirst->dwHighPart == lpSecond->dwHighPart) &&
		   (lpFirst->dwHighPart >  lpSecond->dwHighPart))
		   bResult = TRUE;
		else
			bResult = FALSE;
	}
	return bResult;
}

//Compare two U64 numbers,if equal,returns TRUE,else,returns FALSE.
BOOL U64_EQUAL(__U64* lpFirst,__U64* lpSecond)
{
	if((NULL == lpFirst) || (NULL == lpSecond))
		return FALSE;

	return ((lpFirst->dwHighPart == lpSecond->dwHighPart) &&
		    (lpFirst->dwLowPart  == lpSecond->dwLowPart)) ?
           TRUE : FALSE;
}

//Increment one U64 number.
VOID U64_INCREMENT(__U64* lpNumber)
{
	if(NULL == lpNumber) //parameter check.
		return;

	if(0xFFFFFFFF == lpNumber->dwLowPart)
	{
		lpNumber->dwHighPart += 1;
		lpNumber->dwLowPart  =  0;
	}
	else
	{
		lpNumber->dwLowPart += 1;
	}
	return;
}

//Decrement one U64 number.
VOID U64_DECREMENT(__U64* lpNumber)
{
	if(NULL == lpNumber)
		return;

	if(0 == lpNumber->dwLowPart)
	{
		lpNumber->dwHighPart -= 1;
		lpNumber->dwLowPart = 0xFFFFFFFF;
	}
	else
	{
		lpNumber->dwLowPart -= 1;
	}
	return;
}

//Add two U64 number,stores the result into the third one.
VOID U64_ADD(__U64* lpFirst,__U64* lpSecond,__U64* lpResult)
{
}

//Substract the second U64 number from the first,stores the result into third one.
VOID U64_SUB(__U64* lpFirst,__U64* lpSecond,__U64* lpResult)
{
}

//Set one bit of a U64 number,it's position is indicated by second parameter.
VOID U64_SET_BIT(__U64* lpNumber,UCHAR ucPos)
{
	DWORD         dwTmp        = 0x00000001;
	UCHAR         ucTmp        = 0;

	if((NULL == lpNumber) || ucPos >= 64)  //Parameters check.
		return;

	if(ucPos < 32)
	{
		ucTmp = ucPos;
		dwTmp <<= ucTmp;
		lpNumber->dwLowPart |= dwTmp;
	}
	else
	{
		ucTmp = ucPos - 32;
		dwTmp <<= ucTmp;
		lpNumber->dwHighPart |= dwTmp;
	}
}

//Clear one bit of a U64 number.
VOID U64_CLEAR_BIT(__U64* lpNumber,UCHAR ucPos)
{
	DWORD         dwTmp         = 0x00000001;
	UCHAR         ucTmp         = 0;

	if((NULL == lpNumber) || (ucPos >= 64))
		return;

	if(ucPos < 32)
	{
		ucTmp = ucPos;
		dwTmp <<= ucTmp;
		dwTmp = ~dwTmp;
		lpNumber->dwLowPart &= dwTmp;
	}
	else
	{
		ucTmp = ucPos - 32;
		dwTmp <<= ucTmp;
		dwTmp = ~dwTmp;
		lpNumber->dwHighPart &= dwTmp;
	}
}
*/

//
//Error handling routines implementing.
//

static VOID FatalErrorHandler(DWORD dwReason,LPSTR lpszMsg)
{
	PrintLine("Error Level : FATAL(1)");
	if(lpszMsg != NULL)
		PrintLine(lpszMsg);
	return;
}

static VOID CriticalErrorHandler(DWORD dwReason,LPSTR lpszMsg)
{
	PrintLine("Error Level : CRITICAL(2)");
	if(lpszMsg != NULL)
		PrintLine(lpszMsg);
	return;
}

static VOID ImportantErrorHandler(DWORD dwReason,LPSTR lpszMsg)
{
	PrintLine("Error Level : IMPORTANT(3)");
	if(lpszMsg != NULL)
		PrintLine(lpszMsg);
	return;
}

static VOID AlarmErrorHandler(DWORD dwReason,LPSTR lpszMsg)
{
	PrintLine("Error Level : ALARM(4)");
	if(lpszMsg != NULL)
		PrintLine(lpszMsg);
	return;
}

static VOID InformErrorHandler(DWORD dwReason,LPSTR lpszMsg)
{
	PrintLine("Error Level : INFORM(5)");
	if(lpszMsg != NULL)
		PrintLine(lpszMsg);
	return;
}

VOID ErrorHandler(DWORD dwLevel,DWORD dwReason,LPSTR lpszMsg)
{
	switch(dwLevel)
	{
	case ERROR_LEVEL_FATAL:
		FatalErrorHandler(dwReason,lpszMsg);
		break;
	case ERROR_LEVEL_CRITICAL:
		CriticalErrorHandler(dwReason,lpszMsg);
		break;
	case ERROR_LEVEL_IMPORTANT:
		ImportantErrorHandler(dwReason,lpszMsg);
		break;
	case ERROR_LEVEL_ALARM:
		AlarmErrorHandler(dwReason,lpszMsg);
		break;
	case ERROR_LEVEL_INFORM:
		InformErrorHandler(dwReason,lpszMsg);
		break;
	default:
		break;
	}
}

//
//The following routine prints out bug's information.
//
VOID __BUG(LPSTR lpszFileName,DWORD dwLineNum)
{
	BYTE    strBuff[12];
	PrintLine("BUG oencountered.");
	PrintStr("File name: ");
	PrintStr(lpszFileName);
	Hex2Str(dwLineNum,strBuff);
	PrintLine("Lines: ");
	PrintStr(strBuff);
}

