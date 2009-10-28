#ifndef __STDAFX_H__
#include "StdAfx.h"
#endif

#ifndef __HELLO_TAIWAN__
#include "hellocn.h"
#endif

#ifndef __TASK_CTRL__
#include "taskctrl.h"
#endif

VOID _KeyHandler(DWORD);

char* pszStartMsg = "Hello Taiwan running now.If you have any question,please send email to : garryxin@yahoo.com.cn.";
char* pszCmdTitle = "[mycomputer-view]";

KEY_HANDLER g_keyHandler = _KeyHandler;

void DeadLoop()
{
	while(true);
}

VOID _KeyHandler(DWORD dwParam)
{
	WORD wr = 0x0700;

	if(KeyUpEvent(dwParam))
		return;
	if(IsExtendKey(dwParam))
		return;
	if(VK_RETURN == LOBYTE(LOWORD(dwParam)))
	{
		ChangeLine();
		GotoHome();
		return;
	}
	if(VK_BACKSPACE == LOBYTE(LOWORD(dwParam)))
	{
		GotoPrev();
	}
	else
	{
		wr += (WORD)LOBYTE(LOWORD(dwParam));
		PrintCh(wr);
	}
	return;
}

void __init()
{
	ClearScreen();
	PrintStr(pszStartMsg);
	ChangeLine();
	GotoHome();
	PrintStr(pszCmdTitle);
	g_keyHandler = SetKeyHandler(_KeyHandler);
	DeadLoop();
}
