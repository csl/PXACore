//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Mar,09 2005
//    Module Name               : memmgr.h
//    Module Function           : 
//                                This module countains memory manager pro-type's
//                                definition.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __MEMMGR_H__
#define __MEMMGR_H__
#endif

//------------------------------------------------------------------------
//
//    The definition of Page Frame Manager.
//
//------------------------------------------------------------------------

//
//The definition of __PAGE_FRAME object.
//This object is used to describe a page frame.
//

BEGIN_DEFINE_OBJECT(__PAGE_FRAME)
    __PAGE_FRAME*         lpNextFrame;
    __PAGE_FRAME*         lpPrevFrame;
	DWORD                 dwKernelThreadNum;
	DWORD                 dwFrameFlag;
	LPVOID                lpReserved;
END_DEFINE_OBJECT()

//
//Constant definition.
//

#define PAGE_FRAME_SIZE     4096              //One page frame's size,in current version,
                                              //this value is 4K.

#define PAGE_FRAME_FLAG_READONLY  0x00000001  //Read only page.
#define PAGE_FRAME_FLAG_WRITE     0x00000002  //Write page.
#define PAGE_FRAME_READWRITE      0x00000003
#define PAGE_FRAME_EXECUTABLE     0x00000004


#define PAGE_FRAME_BLOCK_SIZE_4K    1024*4
#define PAGE_FRAME_BLOCK_SIZE_8K    1024*8

#define PAGE_FRAME_BLOCK_SIZE_16K   1024*16
#define PAGE_FRAME_BLOCK_SIZE_32K   1024*32
#define PAGE_FRAME_BLOCK_SIZE_64K   1024*64

#define PAGE_FRAME_BLOCK_SIZE_128K  1024*128
#define PAGE_FRAME_BLOCK_SIZE_256K  1024*256
#define PAGE_FRAME_BLOCK_SIZE_512K  1024*512

#define PAGE_FRAME_BLOCK_SIZE_1024K 1024*1024
#define PAGE_FRAME_BLOCK_SIZE_2048K 1024*2048
#define PAGE_FRAME_BLOCK_SIZE_4096K 1024*4096
#define PAGE_FRAME_BLOCK_SIZE_8192K 1024*8192


#define PAGE_FRAME_BLOCK_NUM  12

#define DEFAULT_PAGE_ALIGMENT 4096    //Aligment at 4k boundary.



//
//The definition of Page Frame Block object.
//This object is used to describe one page frame block.
//

BEGIN_DEFINE_OBJECT(__PAGE_FRAME_BLOCK)
    __PAGE_FRAME*         lpNextBlock;
    __PAGE_FRAME*         lpPrevBlock;
	DWORD*                lpdwBitmap;
END_DEFINE_OBJECT()

//
//The definition of object Page Frame Manager.
//


BEGIN_DEFINE_OBJECT(__PAGE_FRAME_MANAGER)
    __PAGE_FRAME*          lpPageFrameArray;
    __PAGE_FRAME_BLOCK     FrameBlockArray[PAGE_FRAME_BLOCK_NUM];
	DWORD                  dwTotalFrameNum;
	DWORD                  dwFreeFrameNum;
	LPVOID                 lpStartAddress;

	BOOL                   (*Initialize)(__COMMON_OBJECT*    lpThis,
		                                 LPVOID              lpStartAddr,
										 LPVOID              lpEndAddr);

	LPVOID                 (*FrameAlloc)(__COMMON_OBJECT*    lpThis,
		                                 DWORD               dwSize,
										 DWORD               dwFrameFlag);

	VOID                   (*FrameFree)(__COMMON_OBJECT*     lpThis,
		                                LPVOID               lpStartAddr,
										DWORD                dwSize);

END_DEFINE_OBJECT()


//------------------------------------------------------------------------
//
//        The pro-type definition of Chunk Manager.
//
//------------------------------------------------------------------------

//
//Chunk control block.
//This structure is used to trace the chunk,one this structure corresponding one chunk.
//

BEGIN_DEFINE_OBJECT(__CHUNK_CONTROL_BLOCK)
    LPVOID                  lpStartAddr;
    LPVOID                  lpEndAddr;             //The lpEndAddr minis lpStartAddr,then
	                                               //add 1,is the chunk's size.
	                                               //For example,if the lpStartAddr is
	                                               //0x01000000,and the chunk's size is 256K,
	                                               //then the lpEndAddr must be 0x01000000 + 
	                                               //256k - 1 = 0x0103FFFF,not
	                                               //0x01040000.
	__CHUNK_CONTROL_BLOCK*  lpNextChunk;
	__CHUNK_CONTROL_BLOCK*  lpPrevChunk;
	DWORD                   dwChunkFlag;
	WORD                    wReserved;             //Aligment with 4.
	WORD                    wAvlHeight;            //Releated with AVL tree.
	__CHUNK_CONTROL_BLOCK*  lpAvlLeft;             //Point to left tree branch.
	__CHUNK_CONTROL_BLOCK*  lpAvlRight;            //Point to right tree branch.
	__CHUNK_CONTROL_BLOCK*  lpSharedChunk;         //Point to shared chunk.
END_DEFINE_OBJECT()


//
//Chunk flag definition.
//

#define CHUNK_FLAG_CODE         0x00000001    //Code chunk.
#define CHUNK_FLAG_DATA         0x00000002    //Data chunk.
#define CHUNK_FLAG_STACK        0x00000004    //Stack chunk.
#define CHUNK_FLAG_HEAP         0x00000008    //Heap chunk.
#define CHUNK_FLAG_RDATA        0x00000010    //Readonly data.

#define MAX_HEAP_CHUNK_NUM      0x00000008    //Maximal heap chunk number for a kernel
                                              //thread.

#define AVL_SWITCH_CHUNK_NUM    0x00000010    //If one kernel thread creates more chunks
											  //than this number,the chunk by the kernel
											  //thread created will keep in an AVL tree
											  //instead of bidirect link list.

#define MAX_CHUNK_SIZE  PAGE_FRAME_BLOCK_SIZE_8192K //The maximal chunk size.

//------------------------------------------------------------------------
//
//        The pro-type definition for routine MemoryAlloc and MemoryFree.
//
//------------------------------------------------------------------------

/*BEGIN_DEFINE_OBJECT(__FREE_BLOCK_HEADER)    //This structure is used to control a block of
                                            //free memory.
    DWORD                  dwBlockFlag;     //Flag,such as FREE or OCCUPIED.
    DWORD                  dwBlockSize;     //The free memory block's size.
	__FREE_BLOCK_HEADER*   lpNextBlock;     //Pointes to next free block.
	__FREE_BLOCK_HEADER*   lpPrevBlock;     //Pointes to previous block.
END_DEFINE_OBJECT()*/

#define MEMORY_BLOCK_FLAG_FREE      0x00000001
#define MEMORY_BLOCK_FLAG_OCCUPIED  0x00000002
#define MEMORY_BLOCK_FLAG_MODIFIED  0x00000004

//
//The memory block list control block.
//

BEGIN_DEFINE_OBJECT(__FREE_BLOCK_CONTROL_BLOCK)
    DWORD                        dwFlag;
    LPVOID                       lpStartAddr;
	DWORD                        dwTotalSize;
	DWORD                        dwFreeSize;
	//__FREE_BLOCK_HEADER*         lpFreeList;
END_DEFINE_OBJECT()

//
//The definition of Virtual Memory Descriptor.
//One kernel thread has a virtual memory descriptor,the chunk(s) created by the kernel
//thread will keep in the virtual memory descriptor.
//If the chunk(s) owned by a kernel thread exceed AVL_SWITCH_CHUNK_NUM,then the chunk 
//will be maintenanced in an AVL tree,otherwise,the chunk(s) will be keep in a bi-direct
//link list.
//

BEGIN_DEFINE_OBJECT(__VIRTUAL_MEMORY_DESCRIPTOR)
    __CHUNK_CONTROL_BLOCK*          lpChunkListRoot;
    __CHUNK_CONTROL_BLOCK*          lpChunkTreeRoot;
	WORD                            wChunkNum;
	WORD                            wHeapNum;
	DWORD                           dwDescriptorFlag;
	__CHUNK_CONTROL_BLOCK*          HeapChunkArray[MAX_HEAP_CHUNK_NUM];
END_DEFINE_OBJECT()

#define VM_DESCRIPTOR_FLAG_FREE     0x00000001    //The descriptor is free.

//
//The definition of Memory Manager(Chunk Manager).
//

#define MAX_KERNEL_THREAD_NUM  128    //The maximal kernel thread number in current version.
#define INVALID_DESC_INDEX     128

BEGIN_DEFINE_OBJECT(__MEMORY_MANAGER)
    __VIRTUAL_MEMORY_DESCRIPTOR     MemDescArray[MAX_KERNEL_THREAD_NUM];  //One for one kernel thread.
    
    BOOL                            (*Initialize)(__COMMON_OBJECT*     lpThis);
	DWORD                           (*ReserveDescIndex)(__COMMON_OBJECT* lpThis);
	LPVOID                          (*ReserveChunk)(__COMMON_OBJECT*   lpThis,
		                                            DWORD              dwIndex,
													DWORD              dwChunkType,
													DWORD              dwChunkSize);
	VOID                            (*ReleaseChunk)(__COMMON_OBJECT*   lpThis,
		                                            DWORD              dwIndex,
		                                            LPVOID             lpStartAddr);
	VOID                            (*ReleaseDescIndex)(__COMMON_OBJECT* lpThis,
		                                                DWORD            dwIndex);
	LPVOID                          (*MemoryAlloc)(__COMMON_OBJECT*    lpThis,
		                                           DWORD               dwDescIndex,
		                                           DWORD               dwSize);
	VOID                            (*MemoryFree)(__COMMON_OBJECT*     lpThis,
		                                          DWORD                dwDescIndex,
		                                          LPVOID               lpStartAddr,
												  DWORD                dwSize);

END_DEFINE_OBJECT()

/*************************************************************************
**************************************************************************
**************************************************************************
**************************************************************************
*************************************************************************/

//
//Declare the global object MemoryManager.
//

extern __MEMORY_MANAGER MemoryManager;

//
//Declare the global object PageFrameManager.
//

extern __PAGE_FRAME_MANAGER PageFrameManager;
