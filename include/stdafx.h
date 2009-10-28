#ifndef __STDAFX_H__
#define __STDAFX_H__

#ifndef __I386
#define __I386
#endif
/*
#ifndef __HELLO_TAIWAN__
#include "hellocn.h"
#endif
*/
#ifndef __STRING__
#include "string.h"    //Different folder.
#endif

#ifndef __TYPES_H__
#include "types.H"
#endif

#ifndef __PERF_H__
#include "perf.H"
#endif

#ifndef __COMMOBJ_H__
#include "commobj.h"
#endif

#ifndef __SYN_MECH_H__
#include "syn_mech.H"
#endif

#ifndef __COMQUEUE_H__
#include "comqueue.H"
#endif

#ifndef __OBJQUEUE_H__
#include "objqueue.h"
#endif

#ifndef __KTMGR_H__
#include "ktmgr.h"
#endif

//#ifndef __SYNOBJ_H__
//#include "synobj.h"
//#endif

#ifndef __SYSTEM_H__
#include "system.h"
#endif

#ifndef __DEVMGR_H__
#include "devmgr.H"
#endif

#ifndef __MAILBOX_H__
#include "mailbox.h"
#endif

#ifndef __DIM_H__
#include "dim.h"
#endif

#ifndef __MEMMGR_H__
#include "memmgr.h"
#endif

#ifndef __PAGEIDX_H__
#include "pagedix.H"
#endif

#ifndef __VMM_H__
#include "vmm.H"
#endif

#ifndef __HEAP_H__
#include "heap.H"
#endif

#ifndef __TASK_CTRL__
#include "taskctrl.h"
#endif

#ifndef __IOMGR_H__
#include "iomgr.h"
#endif

#ifndef __BUFFMGR_H__
#include "buffmgr.h"
#endif

#ifndef __IDEHDDRV_H__
#include "idehddrv.h"
#endif

#ifndef __SHELL__
#include "shell.h"
#endif

#ifndef __TIMER__
#include "timer.h"
#endif

#ifndef __KMEMMGR__
#include "kmemmgr.h"
#endif

#ifndef __KTMSG_H__
#include "ktmsg.h"
#endif

#ifndef __KTHREAD_H__
#include "kthread.h"
#endif

#ifndef __GLOBAL_VAR__
#include "globvar.h"
#endif

////////////////////////////////////////////////////////////////////////////////
//
//                   Debug mechanism supporting code.
//
////////////////////////////////////////////////////////////////////////////////

#define __ASSERT()
#define __DEBUG_ASSERT()

#endif  //StdAfx.h
