//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Dec,23 2006
//    Module Name               : DRVENTRY.CPP
//    Module Funciton           : 
//                                This file countains a driver entry array,each
//                                device driver embedded in OS kernel,must have
//                                one entry in this array.OS kernel will load the
//                                drivers using this array.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include "stdafx.h"
#include "comdrv.h"

__DRIVER_ENTRY DriverEntryArray[] = 
{
	Com1DrvInitial,
	Com2DrvEntry,
	NULL
};

