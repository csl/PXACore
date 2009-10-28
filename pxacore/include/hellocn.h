//***********************************************************************/
//    Author                    : Garry
//    Original Date             : May,27 2004
//    Module Name               : hellocn.h
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

#ifndef __HELLO_TAIWAN__
#define __HELLO_TAIWAN__

#ifndef __I386__                 //The current version of Hello Taiwan only
                                 //face the I386 platform.
#define __I386__
#endif


#define __MINIKER_BASE      0x00100000    //Mini-kernal's base address.
                                                                
#define __MINIKER_LEN       48*1024       //The mini-kernal's length.
#define __MASTER_BASE       0x00110000    //The master's base address.

//PrintStr's base address.
#define __PRINTSTR_BASE     __MINIKER_LEN + __MINIKER_BASE - 0x04   
//ChangeAttr's base address.
#define __CHANGEATTR_BASE   __MINIKER_LEN + __MINIKER_BASE - 0x08   
//ClearScreen's base address.
#define __CLEARSCREEN_BASE  __MINIKER_LEN + __MINIKER_BASE - 0x0c   
//PrintCh's base address.
#define __PRINTCH_BASE      __MINIKER_LEN + __MINIKER_BASE - 0x10
//GotoHome's base address.
#define __GOTOHOME_BASE     __MINIKER_LEN + __MINIKER_BASE - 0x14
//ChangeLine's base address.
#define __CHANGELINE_BASE   __MINIKER_LEN + __MINIKER_BASE - 0x18

#define __HEXTOSTR_BASE     __MINIKER_LEN + __MINIKER_BASE - 0x1c
#define __STRCPY_BASE       __MINIKER_LEN + __MINIKER_BASE - 0x20
#define __STRLEN_BASE       __MINIKER_LEN + __MINIKER_BASE - 0x24

//SetKeyHandler's base address.
#define __SETNOTIFYOS_BASE  __MINIKER_LEN + __MINIKER_BASE - 0x28

//GotoPrev's base address.
#define __GOTOPREV_BASE     __MINIKER_LEN + __MINIKER_BASE - 0x2c

//Timer handler's base address
#define __TIMERHANDLER_BASE __MINIKER_LEN + __MINIKER_BASE - 0x30

//Set GDT entry handler's base address.
#define __SETGDTENTRY_BASE  __MINIKER_LEN + __MINIKER_BASE - 0x34

//Clear GDT entry handler's base address.
#define __CLGDTENTRY_BASE   __MINIKER_LEN + __MINIKER_BASE - 0x38


//
//Hello Taiwan's basic data type.
//
#define BYTE                char
#define CHAR                char
#define UCHAR               unsigned char
#define UBYTE               unsigned char
#define WORD                unsigned short
#define UWORD               unsigned short
#define DWORD               unsigned long
#define LPSTR               char*
#define INT                 int
#define UINT                unsigned int
#define FLOAT               float
#define DOUBLE              double

#define __U8                unsigned char
#define __U16               unsigned short
#define __U32               unsigned int

#define BOOL                DWORD
#define FALSE               0x00000000
#define TRUE                0x00000001
#define NULL                0x00000000
#define MAX_DWORD_VALUE     0xFFFFFFFF
#define MAX_WORD_VALUE      0xFFFF
#define MAX_BYTE_VALUE      0xFF
#define MAX_QWORD_VALUE     0xFFFFFFFFFFFFFFFF

#define VOID                void
#define LPVOID              void*

#define LOWORD(dw)          WORD(dw)
#define HIWORD(dw)          WORD(dw >> 16)

#define LOBYTE(wr)          BYTE(wr)
#define HIBYTE(wr)          BYTE(wr >> 16)

/*
//
//64 bits unsigned integer definition and operations.
//
struct U64{
	DWORD        dwHighPart;
	DWORD        dwLowPart;
};

#define __U64 U64

BOOL U64_LARGER(__U64*,__U64*);  //If first U64 number is larger than the second,returns
                                 //TRUE,else,return FALSE.
BOOL U64_EQUAL(__U64*,__U64*);   //If the two numbers are equal,returns TRUE,else returns
                                 //FALSE.
VOID U64_INCREMENT(__U64*);      //Increment the U64 number.
VOID U64_DECREMENT(__U64*);      //Decrement the U64 number.

VOID U64_ADD(__U64*,__U64*,__U64*);    //Add two U64 numbers,store the result into third one.
VOID U64_SUB(__U64*,__U64*,__U64*);    //Substract the second U64 number from first U64 number,
                                 //stores result into third one.
VOID U64_SET_BIT(__U64*,UCHAR);  //Set one bit of a U64 number.
VOID U64_CLEAR_BIT(__U64*,UCHAR); //Clear one bit of a U64 number.

VOID U64_MUL(__U64*,__U64*,__U64*);    //Multiple two U64 numbers,stores result into third one.
VOID U64_DIV(__U64*,__U64*,__U64*);    //Divide the first U64 number by the second,
                                       //stores result into third one.

*/


//
//Handler procedure's defination.
//The key driver will call this procedure when a key down/up event occured.
//

typedef VOID (*KEY_HANDLER)(DWORD);    //In this procedure,the parameter
                                       //is a double word,it can be split
                                       //into two word,the high word is
                                       //reserved,not used,and the low
                                       //word is used as following:
                                       // low byte  : code,if this key
                                       //             event is a extend
                                       //             key,then it's the
                                       //             key's scan code,if
                                       //             the key is a ascii
                                       //             key,then it's the
                                       //             key's ascii code.
                                       // high byte : interrupt as following:
                                       // bit 1     : 1 if Shift key down
                                       // bit 2     : 1 if alt key down
                                       // bit 3     : 1 if ctrl key down
                                       // bit 4     : 1 if capslock key down
                                       // bit 5     : reserved
                                       // bit 6     : reserved
                                       // bit 7     : 1 if is a key up event
                                       // bit 8     : 1 if is a extend key.

typedef VOID(*INT_HANDLER)(DWORD);          //General interrupt handler.

//
//The following are some extend key's scan code.
//
#define VK_ESC             0x01
#define VK_BACKSPACE       0x0e
#define VK_TAB             0x0f
#define VK_RETURN          0x0a
#define VK_LEFT_CTRL       0x1d
#define VK_SHIFT           0x2a
#define VK_LEFT_ALT        0x38
#define VK_CAPS_LOCK       0x3a

#define VK_F1              0x3b
#define VK_F2              0x3c
#define VK_F3              0x3d
#define VK_F4              0x3e
#define VK_F5              0x3f
#define VK_F6              0x40
#define VK_F7              0x41
#define VK_F8              0x42
#define VK_F9              0x43
#define VK_F10             0x44
#define VK_F11             0x57
#define VK_F12             0x58

//
//Control bits.
//
#define KEY_SHIFT_STATUS       0x00000100
#define KEY_ALT_STATUS         0x00000200
#define KEY_CTRL_STATUS        0x00000400
#define KEY_CAPS_LOCK_STATUS   0x00000800

#define KEY_EVENT_UP           0x00004000
#define KEY_EVENT_EXTEND_KEY   0x00008000

//
//Macros to simple the programming.
//
#define ShiftKeyDown(para)     (para & KEY_SHIFT_STATUS)
#define AltKeyDown(para)       (para & KEY_ALT_STATUS)
#define CtrlKeyDown(para)      (para & KEY_CTRL_STATUS)
#define CapsLockKeyDown(para)  (para & KEY_CAPS_LOCK_STATUS)

#define KeyUpEvent(para)       (para & KEY_EVENT_UP)
#define IsExtendKey(para)      (para & KEY_EVENT_EXTEND_KEY)

//
//Interrupt flags control functions.
//
VOID DisableInterrupt();

VOID EnableInterrupt();

//
//The following macros are used to synchronize shared resource accessing.
//

/*#define ENTER_CRITICAL_SECTION()  \
	DisableInterrupt()

#define LEAVE_CRITICAL_SECTION()  \
	EnableInterrupt()*/

//
//Error handling routines or macros definition.
//
#define __ERROR_HANDLER(level,reason,pszmsg) \
	ErrorHandler(level,reason,pszmsg)

VOID ErrorHandler(DWORD dwLevel,DWORD dwReason,LPSTR pszMsg);

VOID __BUG(LPSTR,DWORD);
#define BUG() \
	__BUG(__FILE__,__LINE__)

#define ERROR_LEVEL_FATAL       0x00000001
#define ERROR_LEVEL_CRITICAL    0x00000002
#define ERROR_LEVEL_IMPORTANT   0x00000004
#define ERROR_LEVEL_ALARM       0x00000008
#define ERROR_LEVEL_INFORM      0x00000010

//
//Some kernal mode procedures,implemented in Mini-kernal,the Master and
//device drivers can call these procedures.
//

//Print out a string at current position.
void PrintStr(const char* pszMsg);
//Clear the whole screen.
void ClearScreen();
//Print out a color character at current position.
void PrintCh(unsigned short ch);
//Goto the current line's home.
void GotoHome();
//Change to the next line.
void ChangeLine();
//Goto previous position.
void GotoPrev();

//Set the key board handler,and returns the old one.
KEY_HANDLER SetKeyHandler(KEY_HANDLER pKeyHandler);

//Set the timer interrupt handler.
typedef VOID (*__GENERAL_INTERRUPT_HANDLER)(DWORD,LPVOID);

INT_HANDLER SetTimerHandler(__GENERAL_INTERRUPT_HANDLER);

//This function find first empty GDT entry,and set it,returns the index value
//of the entry.
WORD SetGdtEntry(DWORD,DWORD);

//
//Zero a block of memory.
//
VOID MemZero(LPVOID,DWORD);

//
//Block memory copying operation.
//
VOID MemCpy(LPVOID,LPVOID,DWORD);

//
//Read a byte from port.
//
VOID ReadByteFromPort(UCHAR* pByte,    //Buffer.
					  WORD   wPort);   //I/O port.

//
//Write a byte to port.
//
VOID WriteByteToPort(UCHAR,            //The byte to write out.
					 WORD);            //I/O port.

//
//Read string of data from port.
//
VOID ReadByteStringFromPort(LPVOID,        //The local buffer to store string data.
			    			DWORD,         //How many bytes to be read.
				    		WORD);         //I/O port.

//
//Write string data to port.
//
VOID WriteByteStringToPort(LPVOID,         //The data buffer to be write out.
			    		   DWORD,          //How many byte to write out.
				    	   WORD);          //I/O port.

VOID ReadWordFromPort(WORD* pWord,
					  WORD  wPort);

VOID WriteWordToPort(WORD,
					 WORD);

VOID ReadWordStringFromPort(LPVOID,
							DWORD,
							WORD);

VOID WriteWordStringToPort(LPVOID,
						   DWORD,
						   WORD);


#endif //hellocn.h
