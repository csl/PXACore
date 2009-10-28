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

#include "StdAfx.h"
#include "extcmd.h"
#include "fibonacci.h"

__EXTERNAL_COMMAND ExtCmdArray[] = 
{
	{"fibonacci",NULL,FALSE,Fibonacci},
	{NULL,NULL,FALSE,NULL}
};

