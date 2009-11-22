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

//#include "COMMDRV.H"

__DRIVER_ENTRY DriverEntryArray[] = 
{
	Com1DrvEntry,
	Com2DrvEntry,
	Com3DrvEntry,
	Com4DrvEntry,
	Com5DrvEntry,
	Com6DrvEntry,
	Com7DrvEntry,
	Com8DrvEntry,
	NULL
};

