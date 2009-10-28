//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jun,24 2006
//    Module Name               : EXTCMD.CPP
//    Module Funciton           : 
//                                This module countains Hello Taiwan's External command's
//                                implementation.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include ".\INCLUDE\StdAfx.h"
#include "EXTCMD.H"
#include "FIBONACCI.H"

__EXTERNAL_COMMAND ExtCmdArray[] = {
	{"fibonacci",NULL,FALSE,Fibonacci},
	{NULL,NULL,FALSE,NULL}
};

