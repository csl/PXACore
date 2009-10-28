//***********************************************************************/
//    Author                    : Garry
//    Original Date             : May,28 2004
//    Module Name               : string.h
//    Module Funciton           : 
//                                This module and string.cpp countains the
//                                string operation method.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __STRING__
#define __STRING__

#define MAX_STRING_LEN 512       //Max string length.

BOOL StrCmp(LPSTR,LPSTR);      //String compare functions.
WORD StrLen(LPSTR);            //Get the string's length.
BOOL Hex2Str(DWORD,LPSTR);     //Convert the first parameter(hex format)
                               //to string.
BOOL Str2Hex(LPSTR,DWORD*);    //Convert the string to hex number.

BOOL Str2Int(LPSTR,DWORD*);    //Convert the string to int.
BOOL Int2Str(DWORD,LPSTR);     //Convert the 32 bit int to string.

VOID PrintLine(LPSTR);         //Print the string at a new line.

VOID StrCpy(LPSTR,LPSTR);      //Copy one string to the second string.

VOID ConvertToUper(LPSTR);     //Convert the string's characters from low to uper.

INT FormString(LPSTR,LPSTR,LPVOID*);

#endif //string.h