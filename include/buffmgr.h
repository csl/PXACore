/***********************************************************************/
//    Author                    : Garry
//    Original Date             : Aug,25 2004
//    Module Name               : buffmgr.h
//    Module Funciton           : 
//                                This module countains the buffer manager data
//                                structures and data types definition.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __BUFFMGR_H__
#define __BUFFMGR_H__

//
//Basic concepts:
//1. Buffer block : A buffer object,used by application to store data temporarily,
//                  when the temporarily data is freed,the buffer block can be
//                  freed too.
//
//2. Buffer pool : A big block of memory,the buffer blocks are allocated from
//                 this big block memory.

#define MIN_BUFFER_SIZE  16  //The min buffer can be allocated.

//
//The following are some control structure's and operation functions' definition.
//

struct __BUFFER_CONTROL_BLOCK;
struct __FREE_BUFFER_HEADER;
struct __USED_BUFFER_HEADER;

typedef BOOL     (*__CREATE_BUFFER_1)(struct __BUFFER_CONTROL_BLOCK*,DWORD);
typedef BOOL     (*__CREATE_BUFFER_2)(struct __BUFFER_CONTROL_BLOCK*,LPVOID,DWORD);
typedef LPVOID   (*__ALLOCATE)(struct __BUFFER_CONTROL_BLOCK*,DWORD);
typedef VOID     (*__FREE)(struct __BUFFER_CONTROL_BLOCK*,LPVOID);
typedef DWORD    (*__GET_BUFFER_FLAG)(struct __BUFFER_CONTROL_BLOCK*,LPVOID);
typedef BOOL     (*__SET_BUFFER_FLAG)(struct __BUFFER_CONTROL_BLOCK*,LPVOID,DWORD);
typedef VOID     (*__DESTROY_BUFFER)(struct __BUFFER_CONTROL_BLOCK*);

//Buffer operation function.

struct __BUFFER_OPERATIONS
{
	__CREATE_BUFFER_1                 lpCreateBuffer1;
	__CREATE_BUFFER_2                 lpCreateBuffer2;
	__ALLOCATE                        lpAllocate;
	__FREE                            lpFree;
	__GET_BUFFER_FLAG                 lpGetBufferFlag;
	__SET_BUFFER_FLAG                 lpSetBufferFlag;
	__DESTROY_BUFFER                  lpDestroyBuffer;
};


//Buffer control block.

#define CREATED_BY_SELF           0x00000001  //When the buffer pool is created by lpCreateBuffer1 function,
                                              //set this bit.
#define CREATED_BY_CLIENT         0x00000002  //When the buffer pool is created by lpCreateBuffer2 function,
                                              //set this bit.
#define POOL_INITIALIZED          0x00000004  //If set,the buffer's pool is created.
                                              //This bit must be set before the buffer manager
                                              //can be used.
#define OPERATIONS_INITIALIZED    0x00000008  //If set,the operations is initialized.
                                              //This bit must be set before the buffer manager
                                              //can be used.

struct __BUFFER_CONTROL_BLOCK
{
	DWORD                         dwFlags;
	LPVOID                        lpPoolStartAddress;
	DWORD                         dwPoolSize;
	DWORD                         dwFreeSize;
	DWORD                         (*GetControlBlockFlag)(struct __BUFFER_CONTROL_BLOCK*);
	struct __FREE_BUFFER_HEADER*  lpFreeBufferHeader;
	struct __BUFFER_OPERATIONS    BufferOperations;
};

//
//Free buffer's control header.
//All of the free buffers are linked into a bi-direct list.
//

#define BUFFER_STATUS_FREE        0x00000001  //If this bit set,the buffer block is free.
#define BUFFER_STATUS_USED        0x00000002  //If this bit set,the buffer block is used.
#define BUFFER_STATUS_MODIFIED    0x00000004  //If this buffer's content is modified,
                                              //set this bit.

struct __FREE_BUFFER_HEADER
{
	DWORD                  dwFlags;
	DWORD                  dwBlockSize;
	struct __FREE_BUFFER_HEADER*  lpNextBlock;
	struct __FREE_BUFFER_HEADER*  lpPrevBlock;
};

struct __USED_BUFFER_HEADER
{
	DWORD                  dwFlags;
	DWORD                  dwBlockSize;
	LPVOID                 lpReserved1;            //Make sure the size of __FREE_BUFFER_HEADER
	                                               //is the same as size of __USED_BUFFER_HEADER.
	LPVOID                 lpReserved2;
};


//
//If a client want to use the buffer manager,it first suplly a __BUFFER_CONTROL_BLOCK object,
//and call the following function to initialize it.
//This function fills the control block operations field to proper functions,so,
//the client can call to request service.
//
BOOL InitBufferMgr(struct __BUFFER_CONTROL_BLOCK*);

#endif //End of buffmgr.h
