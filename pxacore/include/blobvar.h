//***********************************************************************/
//    Author                    : Garry
//    Original Date             : May,27 2004
//    Module Name               : globvar.h
//    Module Funciton           : 
//                                This module countains the defination of
//                                global variables.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __GLOBAL_VAR__
#define __GLOBAL_VAR__

extern __TASK_CTRL_BLOCK*        g_pCurrentTask;
extern __KTHREAD_CONTROL_BLOCK*  g_pShellCtrlBlock;

extern __KERNEL_THREAD_OBJECT*   g_lpShellThread;

extern BYTE                      HostName[16];
extern __TASK_CTRL_BLOCK         tcbShell;

extern __KTHREAD_CONTROL_BLOCK*  g_pKThreadQueue[MAX_KTHREAD_NUM];
extern DWORD EntryPoint();
extern VOID  _KeyHandler(DWORD);

#endif //globvar.h