//***********************************************************************/
//    Author                    : Garry
//    Original Date             : May,27 2004
//    Module Name               : shell.h
//    Module Funciton           : 
//                                This module countains the definations,
//                                data types,and procedures.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __SHELL__
#define __SHELL__

#define MAX_BUFFER_LEN 256       //The max length of command buffer.
#define MAX_CMD_LEN    16        //The max length of one command.

//
//The event handler,it handles all the system event,and dispatch event.
//
BOOL EventHandler(WORD wCommand,DWORD dwParam,DWORD dwParam2);


//
//Shell data structures.
//

typedef VOID (*__CMD_HANDLER)(LPSTR);  //Command handler.

struct __CMD_OBJ{
	LPSTR          CmdStr;               //Command text.
	//__tagCMD_OBJ*  pPrev;                //Point to the previous command object.
	//__tagCMD_OBJ*  pNext;                //Point to the next.
	//__tagCMD_OBJ*  pParent;              //Point to the parent.
	__CMD_HANDLER  CmdHandler;           //Command handler.
};


#define CMD_PARAMETER_LEN 31            //The max length of one parameter.

//
//Command paramter object,countains the parameters and function labels of one
//command.
//

struct __CMD_PARA_OBJ{
	BYTE               byFunctionLabel;        //The function label.
	BYTE               byParameterNum;         //How many parameters  followed.
	WORD               wReserved;
	__CMD_PARA_OBJ*    pNext;                  //Points to the next command paramter object.
	BYTE               Parameter[0][CMD_PARAMETER_LEN + 1]; //The parameters.
};

//
//The following macro calculates the next parameter object's address.
//
#define NextParaAddr(currentaddr,paranum)  (LPVOID)((BYTE*)currentaddr + 8 + (CMD_PARAMETER_LEN + 1) * paranum)

//
//Form the parameter objects link from a command line string.
//
__CMD_PARA_OBJ* FormParameterObj(LPSTR);

//
//Release the command line object created by FormParameterObj routine.
//
VOID ReleaseParameterObj(__CMD_PARA_OBJ* lpParamObj);

#endif //shell.h