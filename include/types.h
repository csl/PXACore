//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Sep,16 2005
//    Module Name               : TYPES.H
//    Module Funciton           : 
//                                This module countains the definition for several types
//                                supported by Hello Taiwan.
//                                The operating interface,i.e,the function that operate these
//                                data structures,also be defined here.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __TYPES_H__
#define __TYPES_H__

#define BOOL    unsigned long
#define DWORD   unsigned long
#define NULL    0
#define VOID    void
#define TRUE    0xFFFFFFFF
#define FALSE   0x00000000
#define LPSTR   char*
#define BYTE    char
#define WORD    unsigned short

#define LOWORD(dw)  ((unsigned short)(dw))
#define LOBYTE(wr)  ((signed char)(wr))

#define true	1
#define false	0
//
//The definition of unsigned 64 bit integer.
//
/*
#define U64_ZERO {0,0}
#define U64_MAX  {0xFFFFFFFF,0xFFFFFFFF}

struct __U64{
	unsigned long dwLowPart;
	unsigned long dwHighPart;
};

//
//Add operations for unsigned 64 bits integer.
//lpu64_result = lpu64_1 + lpu64_2.
//

VOID u64Add(__U64* lpu64_1,__U64* lpu64_2,__U64* lpu64_result);

//
//Subtract operations for U64.
//lpu64_result = lpu64_1 - lpu64_2.
//

VOID u64Sub(__U64* lpu64_1,__U64* lpu64_2,__U64* lpu64_result);

//
//Compare operations for unsigned 64 bits integer.
//

BOOL EqualTo(__U64* lpu64_1,__U64* lpu64_2);    //TRUE if lpu64_1 == lpu64_2.
BOOL LessThan(__U64* lpu64_1,__U64* lpu64_2);   //TRUE if lpu64_1 < lpu64_2.
BOOL MoreThan(__U64* lpu64_1,__U64* lpu64_2);   //TRUE if lpu64_1 > lpu64_2.
VOID u64Div(__U64*,__U64*,__U64*,__U64*);

//
//Shift operation.
//

VOID u64RotateLeft(__U64* lpu64_1,DWORD dwTimes);  //Shift dwTimes bit(s) of lpu64_1 to left.
VOID u64RotateRight(__U64* lpu64_1,DWORD dwTimes); //Shift dwTimes bit(s) of lpu64_1 to right.

//
//Multiple operations.
//

// VOID u64Mul(__U64* lpu64_1,__U64* lpu64_2,__U64* lpu64_result);

//
//Divide operations.
//

//VOID u64Div(__U64* lpu64_1,__U64* lpu64_2,__U64* lpu64_result);

BOOL u64Hex2Str(__U64* lpu64,LPSTR lpszResult);
*/
#endif
