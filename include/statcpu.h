/***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct,22 2006
//    Module Name               : STATCPU.H
//    Module Funciton           : 
//                                Countains CPU overload ratio statistics related
//                                data structures and routines.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#define GET_KERNEL_MEMORY(size)   KMemAlloc(size,KMEM_SIZE_TYPE_ANY)
#define FREE_KERNEL_MEMORY(p)     KMemFree(p,KMEM_SIZE_TYPE_ANY,0L)

#define MAX_STAT_PERIOD        300
#define PERIOD_TIME            1000
#define ONE_MINUTE_PERIOD      60

struct __THREAD_STAT_OBJECT{
	__THREAD_STAT_OBJECT*     lpPrev;
	__THREAD_STAT_OBJECT*     lpNext;

	__U64                     TotalCpuCycle;
	__U64                     CurrPeriodCycle;
	__U64                     PreviousTsc;

	__KERNEL_THREAD_OBJECT*   lpKernelThread;

	WORD                      wQueueHdr;
	WORD                      wQueueTail;
	WORD                      RatioQueue[MAX_STAT_PERIOD];
	WORD                      wOneMinuteRatio;  //CPU ratio in last 1 minute.
	WORD                      wMaxStatRatio;    //CPU ratio in last maximal statistics
	                                            //period,such as 5 minutes.
	WORD                      wCurrPeriodRatio;
	WORD                      wReserved;
};

struct __STAT_CPU_OBJECT{
	//__KERNEL_THREAD_OBJECT*   lpStatKernelThread;
	__U64                     PreviousTsc;       //Previous time stamp counter.
	__U64                     CurrPeriodCycle;   //CPU cycle conter in this stat period.
	__U64                     TotalCpuCycle;     //Total CPU cycle counter since
	                                             //system startup.
	__THREAD_STAT_OBJECT      IdleThreadStatObj;

	BOOL                     (*Initialize)(__STAT_CPU_OBJECT*);    //Initialize routine.
	__THREAD_STAT_OBJECT*    (*GetFirstThreadStatObj)();
	__THREAD_STAT_OBJECT*    (*GetNextThreadSTatObj)(__THREAD_STAT_OBJECT*);
	VOID                     (*DoStat)();
	VOID                     (*ShowStat)();
};

extern __THREAD_HOOK_ROUTINE lpCreateHook;
extern __THREAD_HOOK_ROUTINE lpBeginScheduleHook;
extern __THREAD_HOOK_ROUTINE lpEndScheduleHook;
extern __THREAD_HOOK_ROUTINE lpTerminalHook;

extern __STAT_CPU_OBJECT  StatCpuObject;

