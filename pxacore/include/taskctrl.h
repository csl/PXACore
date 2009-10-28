//***********************************************************************/
//    Author                    : Garry
//    Original Date             : May,27 2004
//    Module Name               : taskctrl.h
//    Module Funciton           : 
//                                This module countains task control data
//                                structures and procedures.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __TASK_CTRL__
#define __TASK_CTRL__


#define MAX_MESSAGE_NUM        0x20

//
//Message control structures and operation.
//

//If I change the following defination as follows:
//typedef struct __tagMSG{
//    WORD     wCommand;
//    DWORD    dwParam;
//    WORD     wReserved;
//}__MSG;
//It can not work.
//Fuck you Bill Gates! So poor compiler!!
//

struct __MSG{
	WORD     wCommand;           //Message type,such as WM_KEYDOWN,WM_KEYUP,etc.
	WORD     wReserved;
	DWORD    dwParam;            //Parameter assigned with the command.
};

//Message command defination.
#define MSG_KEY_DOWN            0x0001
#define MSG_KEY_UP              0x0002
#define MSG_LEFT_MOUSE_UP       0x0003
#define MSG_MIDD_MOUSE_UP       0x0004
#define MSG_RIGHT_MOUSE_UP      0x0005
#define MSG_LEFT_MOUSE_DOWN     0x0006
#define MSG_MIDD_MOUSE_DOWN     0x0007
#define MSG_RIGHT_MOUSE_DOWN    0x0008

#define MSG_SYS_TERMINAL        0x0009  //If OS want to task terminal,
                                        //it sends this message to the task.
#define MSG_TIMER               0x000a  //Time message.

//
//Task control block defination,the first part of this structure is TSS,
//and the following part of the TCB is operating system defined fields.
//
typedef struct __TCB{
	WORD     wPrevTSS;
	WORD     wReserved1;         //Reserved part.

	DWORD    dwEsp0;             //Level 0's stack pointer.
	WORD     wSs0;               //Level 0's stack segment selector.
	WORD     wReserved2;         //Reserved.

	DWORD    dwEsp1;             //Level 1's stack pointer.
	WORD     wSs1;               //Level 1's segment selector.
	WORD     wReserved3;

	DWORD    dwEsp2;             //Level 2's stack pointer.
	WORD     wSs2;               //Level 2's segment selector.
	WORD     wReserved4;

	DWORD    dwCr3;              //Page table's base address.

	DWORD    dwEip;              //Program counter of the task.
	DWORD    dwEflags;           //Flags.

	DWORD    dwEax;              //General registers.
	DWORD    dwEcx;
	DWORD    dwEdx;
	DWORD    dwEbx;
	DWORD    dwEsp;
	DWORD    dwEbp;
	DWORD    dwEsi;
	DWORD    dwEdi;

	WORD     wEs;                //Segments's selector of the task.
	WORD     wReserved5;
	WORD     wCs;
	WORD     wReserved6;
	WORD     wSs;
	WORD     wReserved7;
	WORD     wDs;
	WORD     wReserved8;
	WORD     wFs;
	WORD     wReserved9;
	WORD     wGs;
	WORD     wReserved10;

	WORD     wLdtSelector;       //LDT table's selector.
	DWORD    TrapSign : 1;       //Trap sign.
	DWORD    Reserved11 : 15;
	WORD     wIoMapAddr;         //IO map's base address.
	                             //End of the TSS segment.

	DWORD    dwTaskID;           //Task ID.

	__TCB*   Prev;               //Point to the next TCB.
	__TCB*   Next;
	__TCB*   Parent;

    __MSG    MsgQueue[MAX_MESSAGE_NUM];  //The current task's message queue.
	WORD     wHeader;                    //Message queue's header.
	WORD     wTrial;                     //Message queue's trial.
	WORD     wCurrentMsgCounter;         //Message queue's length.

	WORD     wTaskStatus;                //Task status.
	WORD     wTimeCounter;               //Time counters,every clock interrupt,
	                                     //system decrement this variable,if
	                                     //this counter reachs 0,then schedule
	                                     //another task.
	DWORD    dwRunTime;                  //Total running times since task start.

	DWORD    dwReserved;
}__TASK_CTRL_BLOCK;

#define TASK_STATUS_RUNNING    0x0001
#define TASK_STATUS_READY      0x0002
#define TASK_STATUS_BLOCKING   0x0003
#define TASK_STATUS_TERMINAL   0x0004

BOOL SendMessage(__TASK_CTRL_BLOCK*,WORD,DWORD); //Add a message to the task's msg
                                            //queue.
BOOL GetMessage(__TASK_CTRL_BLOCK*,__MSG*); //Get a message from the task
                                            //identified by __TASK_CTRL_BLOCK
BOOL MsgQueueFull(__TASK_CTRL_BLOCK*);
BOOL MsgQueueEmpty(__TASK_CTRL_BLOCK*);

typedef DWORD (*_EVENT_HANDLER)(WORD,DWORD,DWORD);  //Message handler type.

//BOOL DispatchMessage(__MSG*,_EVENT_HANDLER);


#endif //taskctrl.h