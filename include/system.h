//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Nov,06 2004
//    Module Name               : system.h
//    Module Funciton           : 
//                                This module countains system mechanism releated objects's
//                                definition.
//                                Including the following aspect:
//                                1. Interrupt object and interrupt management code;
//                                2. Timer object and timer management code;
//                                3. System level parameters management coee,such as
//                                   physical memory,system time,etc;
//                                4. Other system mechanism releated objects.
//
//                                ************
//                                This file is one of the most important file of Hello Taiwan.
//                                ************
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __SYSTEM_H__
#define __SYSTEM_H__

//
//Interrupt object's definition.
//The interrupt object is used to manage system interrupt.
//

typedef BOOL (*__INTERRUPT_HANDLER)(LPVOID lpEsp,LPVOID);    //Interrupt handler's pro-type.

#define MAX_INTERRUPT_VECTOR  256               //Maximal interrupt vector supported now.

#ifdef __I386__                                 //Interrupt vector value's definition.

#define INTERRUPT_VECTOR_TIMER         0x20
#define INTERRUPT_VECTOR_KEYBOARD      0x21
#define INTERRUPT_VECTOR_MOUSE         0x22
#define INTERRUPT_VECTOR_COM1          0x23
#define INTERRUPT_VECTOR_COM2          0x24
#define INTERRUPT_VECTOR_CLOCK         0x25
#define INTERRUPT_VECTOR_IDE           0x26

#else
#endif

BEGIN_DEFINE_OBJECT(__INTERRUPT_OBJECT)
    INHERIT_FROM_COMMON_OBJECT
    struct __INTERRUPT_OBJECT*    lpPrevInterruptObject;
    struct __INTERRUPT_OBJECT*    lpNextInterruptObject;
    UCHAR                         ucVector;
    BOOL                          (*InterruptHandler)(LPVOID lpParam,LPVOID lpEsp);
    LPVOID                        lpHandlerParam;
END_DEFINE_OBJECT()

BOOL InterruptInitialize(struct __COMMON_OBJECT* lpThis);    //Interrupt object's initializing routine.
VOID InterruptUninitialize(struct __COMMON_OBJECT* lpThis);  //Uninitializing routine.

//
//Timer object's definition.
//The timer object is used to manage the system timer.
//

typedef DWORD    (*__DIRECT_TIMER_HANDLER)(LPVOID);    //Timer handler's protype.

BEGIN_DEFINE_OBJECT(__TIMER_OBJECT)
    INHERIT_FROM_COMMON_OBJECT
	//__TIMER_OBJECT*             lpPrevTimerObject;
        //__TIMER_OBJECT*             lpNextTimerObject;
	DWORD                       dwTimerID;            //Timer ID,one kernel thread may set
	                                                  //several timers,this is it's ID.
	DWORD                       dwTimeSpan;           //Timer span in millisecond.
	struct __KERNEL_THREAD_OBJECT* lpKernelThread;       //The kernel thread who set the timer.
	LPVOID                      lpHandlerParam;
	DWORD                       (*DirectTimerHandler)(LPVOID);       //lpHandlerParam is it's parameter.
	DWORD                       dwTimerFlags;
END_DEFINE_OBJECT()

BOOL  TimerInitialize(struct __COMMON_OBJECT* lpThis);    //Initializing routine of timer object.
VOID  TimerUninitialize(struct __COMMON_OBJECT* lpThis);  //Uninitializing routine of timer object.

//
//The following is the definition of system object.
//

BEGIN_DEFINE_OBJECT(__SYSTEM)
	struct __INTERRUPT_OBJECT*                   lpInterruptVector[MAX_INTERRUPT_VECTOR];
	struct __PRIORITY_QUEUE*                     lpTimerQueue;

	DWORD                                 dwClockTickCounter;    //Records how many clock
				                                     //tickes have occured since
				                                     //system start.
	DWORD                                 dwNextTimerTick;       //When dwClockTickCounter
				                                     //reaches this number,
				                                     //one or many timer event
				                                     //set by kernel thread
				                                     //should be processed.
	UCHAR                                 ucIntNestLevel;        //Interrupt nesting level.
	#define IN_INTERRUPT()  (System.ucIntNestLevel)                  //Current context is interrupt.
	#define IN_KERNELTHREAD() (System.ucIntNestLevel == 0)           //Current context is process.

	UCHAR                                 ucReserved1;           //Align to DWORD.
	UCHAR                                 ucReserved2;
	UCHAR                                 ucReserved3;

	DWORD                                 dwPhysicalMemorySize;

	BOOL                                  (*Initialize)(struct __COMMON_OBJECT* lpThis);
	DWORD                                 (*GetClockTickCounter)(struct __COMMON_OBJECT* lpThis);
	DWORD                                 (*GetPhysicalMemorySize)(struct __COMMON_OBJECT* lpThis);
	VOID                                  (*DispatchInterrupt)(struct __COMMON_OBJECT* lpThis,
					                               LPVOID           lpEsp,
					                               UCHAR            ucVector);

	struct __COMMON_OBJECT*               (*ConnectInterrupt)(struct __COMMON_OBJECT* lpThis,
					                              __INTERRUPT_HANDLER InterruptHandler,	  										LPVOID           lpHandlerParam,
									UCHAR            ucVector,
									UCHAR            ucReserved1,
									UCHAR            ucReserved2,
	 	 							UCHAR            ucInterruptMode,
									BOOL             bIfShared,
									DWORD            dwCPUMask);

	VOID                                  (*DisconnectInterrupt)( struct __COMMON_OBJECT* lpThis,
					                              struct __COMMON_OBJECT* lpIntObj);


	struct __COMMON_OBJECT*               (*SetTimer)(struct __COMMON_OBJECT*         lpThis,
									__KERNEL_THREAD_OBJECT*  lpKernelThread,
									DWORD                    dwTimerID,
									DWORD                    dwTimeSpan,
									__DIRECT_TIMER_HANDLER DirectTimerHandler, 
									LPVOID 			 lpHandlerParam,
									DWORD                    dwTimerFlags
												  );
	VOID                                  (*CancelTimer)(struct __COMMON_OBJECT* lpThis,
					                         struct __COMMON_OBJECT* lpTimer);
END_DEFINE_OBJECT()



#define TIMER_FLAGS_ONCE        0x00000001    //Set a timer with this flags,the timer only
                                              //apply once,i.e,the kernel thread who set
					//the timer can receive timer message only
					//once.
#define TIMER_FLAGS_ALWAYS      0x00000002    //Set a timer with this flags,the timer will
					//availiable always,only if the kernel thread
					//cancel the timer by calling CancelTimer.


/**************************************************************************************
***************************************************************************************
***************************************************************************************
***************************************************************************************
**************************************************************************************/

extern struct __SYSTEM System;    //Declares a global object--System.

extern __PERF_RECORDER  TimerIntPr;    //Performance recorder object used to mesure
                                       //the performance of timer interrupt.

//-------------------------------------------------------------------------------------
//
//        General Interrupt Handler's declaration.
//
//-------------------------------------------------------------------------------------

//typedef VOID (*__GENERAL_INTERRUPT_HANDLER)(DWORD,LPVOID);    //General interrupt handler's
                                                              //protype.

VOID GeneralIntHandler(DWORD dwVector,LPVOID lpEsp);

#endif
