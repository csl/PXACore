#ifndef __STDAFX_H__
#include "..\INCLUDE\StdAfx.h"
#endif

#include "COMMDRV.H"

BOOL Com1DrvEntry(__DRIVER_OBJECT* lpDrvObj)
{
	__DEVICE_OBJECT*  lpDevObject = NULL;

	lpDevObject = IOManager.CreateDevice((__COMMON_OBJECT*)&IOManager,
		"COM1",
		2048,
		1024,
		32,
		32,
		NULL,
		lpDrvObj);
	if(NULL == lpDevObject)  //Failed to create device object.
	{
		PrintLine("COM Driver: Failed to create device object for COM1.");
		return FALSE;
	}
	return TRUE;
}

BOOL Com2DrvEntry(__DRIVER_OBJECT* lpDrvObj)
{
	__DEVICE_OBJECT*  lpDevObject = NULL;

	lpDevObject = IOManager.CreateDevice((__COMMON_OBJECT*)&IOManager,
		"COM2",
		2048,
		1024,
		32,
		32,
		NULL,
		lpDrvObj);
	if(NULL == lpDevObject)  //Failed to create device object.
	{
		PrintLine("COM Driver: Failed to create device object for COM2.");
		return FALSE;
	}
	return TRUE;

}

BOOL Com3DrvEntry(__DRIVER_OBJECT* lpDrvObj)
{
	__DEVICE_OBJECT*  lpDevObject = NULL;

	lpDevObject = IOManager.CreateDevice((__COMMON_OBJECT*)&IOManager,
		"COM3",
		2048,
		1024,
		32,
		32,
		NULL,
		lpDrvObj);
	if(NULL == lpDevObject)  //Failed to create device object.
	{
		PrintLine("COM Driver: Failed to create device object for COM3.");
		return FALSE;
	}
	return TRUE;
}

BOOL Com4DrvEntry(__DRIVER_OBJECT* lpDrvObj)
{
	__DEVICE_OBJECT*  lpDevObject = NULL;

	lpDevObject = IOManager.CreateDevice((__COMMON_OBJECT*)&IOManager,
		"COM4",
		2048,
		1024,
		32,
		32,
		NULL,
		lpDrvObj);
	if(NULL == lpDevObject)  //Failed to create device object.
	{
		PrintLine("COM Driver: Failed to create device object for COM4.");
		return FALSE;
	}
	return TRUE;
}

BOOL Com5DrvEntry(__DRIVER_OBJECT* lpDrvObj)
{
	__DEVICE_OBJECT*  lpDevObject = NULL;

	lpDevObject = IOManager.CreateDevice((__COMMON_OBJECT*)&IOManager,
		"COM5",
		2048,
		1024,
		32,
		32,
		NULL,
		lpDrvObj);
	if(NULL == lpDevObject)  //Failed to create device object.
	{
		PrintLine("COM Driver: Failed to create device object for COM5.");
		return FALSE;
	}
	return TRUE;
}

BOOL Com6DrvEntry(__DRIVER_OBJECT* lpDrvObj)
{
	__DEVICE_OBJECT*  lpDevObject = NULL;

	lpDevObject = IOManager.CreateDevice((__COMMON_OBJECT*)&IOManager,
		"COM6",
		2048,
		1024,
		32,
		32,
		NULL,
		lpDrvObj);
	if(NULL == lpDevObject)  //Failed to create device object.
	{
		PrintLine("COM Driver: Failed to create device object for COM6.");
		return FALSE;
	}
	return TRUE;
}

BOOL Com7DrvEntry(__DRIVER_OBJECT* lpDrvObj)
{
	__DEVICE_OBJECT*  lpDevObject = NULL;

	lpDevObject = IOManager.CreateDevice((__COMMON_OBJECT*)&IOManager,
		"COM7",
		2048,
		1024,
		32,
		32,
		NULL,
		lpDrvObj);
	if(NULL == lpDevObject)  //Failed to create device object.
	{
		PrintLine("COM Driver: Failed to create device object for COM7.");
		return FALSE;
	}
	return TRUE;
}

BOOL Com8DrvEntry(__DRIVER_OBJECT* lpDrvObj)
{
	__DEVICE_OBJECT*  lpDevObject = NULL;

	lpDevObject = IOManager.CreateDevice((__COMMON_OBJECT*)&IOManager,
		"COM8",
		2048,
		1024,
		32,
		32,
		NULL,
		lpDrvObj);
	if(NULL == lpDevObject)  //Failed to create device object.
	{
		PrintLine("COM Driver: Failed to create device object for COM8.");
		return FALSE;
	}
	return TRUE;
}

