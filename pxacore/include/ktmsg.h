/***********************************************************************/
//    Author                    : Garry
//    Original Date             : 2004-07-05
//    Module Name               : ktmsg.h
//    Module Funciton           : 
//                                This module countains kernal thread message
//                                structures defination and functions defination.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __KTMSG_H__
#define __KTMSG_H__

//
//Kernal thread message structure.
//
struct __KTHREAD_MSG{
	WORD         wCommand;                //Message command.
	WORD         wReserved;
	DWORD        dwParam_01;              //First parameter of the message.
	DWORD        dwParam_02;              //Second parameter of the message.
};

#ifndef MAX_KTHREAD_MSG_NUM
#define MAX_KTHREAD_MSG_NUM  0x20    //Now,we define the max message number one
                                     //message queue can countains to 32.
#endif

//
//Common message command defination.
//


#define KTMSG_KEY_DOWN              0x0001
#define KTMSG_KEY_UP                0x0002
#define KTMSG_TIMER                 0x0003
#define KTMSG_THREAD_TERMINAL       0x0004
#define KERNEL_MESSAGE_TERMINAL     0x0005
#define KERNEL_MESSAGE_TIMER        0x0006


#endif //ktmsg.h