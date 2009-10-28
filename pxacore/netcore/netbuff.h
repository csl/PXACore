//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Aug,04 2005
//    Module Name               : NETBUFF.H
//    Module Funciton           : 
//                                This module countains net buffer's
//                                definition.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __NETBUFF_H__
#define __NETBUFF_H__
#endif

DECLARE_PREDEFINED_OBJECT(__NET_BUFFER)
DECLARE_PREDEFINED_OBJECT(__SOCKET)
DECLARE_PREDEFINED_OBJECT(__TIME)
DECLARE_PREDEFINED_OBJECT(__NET_DEVICE)
DECLARE_PREDEFINED_OBJECT(__IP_HDR)
DECLARE_PREDEFINED_OBJECT(__TCP_HDR)
DECLARE_PREDEFINED_OBJECT(__UDP_HDR)
DECLARE_PREDEFINED_OBJECT(__ICMP_HDR)
DECLARE_PREDEFINED_OBJECT(__IGMP_HDR)
DECLARE_PREDEFINED_OBJECT(__OSPF_HDR)
DECLARE_PREDEFINED_OBJECT(__IPIP_HDR)
DECLARE_PREDEFINED_OBJECT(__GRE_HDR)
DECLARE_PREDEFINED_OBJECT(__IPV6_HDR)
DECLARE_PREDEFINED_OBJECT(__ARP_HDR)
DECLARE_PREDEFINED_OBJECT(__ISIS_HDR)


//
//The definition of net buffer queue.
//

BEGIN_DEFINE_OBJECT(__NET_BUFFER_QUEUE	)
    __NET_BUFFER*             lpQueueHdr;    //Header pointer of the queue.
    __NET_BUFFER*             lpQueueTail;   //Tail pointer of the queue.
	DWORD                     dwQueueNum;    //How many element in the queue currently.
	DWORD                     dwMaxNum;      //Maximal element number in the queue.

	BOOL                      (*InsertIntoQueue)(__NET_BUFFER_QUEUE*,__NET_BUFFER*);
	BOOL                      (*DeleteFromQueue)(__NET_BUFFER_QUEUE*,__NET_BUFFER*);
	__NET_BUFFER*             (*GetFromQueue)(__NET_BUFFER_QUEUE*);
	DWORD                     (*GetQueueLen)(__NET_BUFFER_QUEUE*);    //Returns the length.
	VOID                      (*SetMaxLen)(__NET_BUFFER_QUEUE*,DWORD);
	VOID                      (*Initialize)(__NET_BUFFER_QUEUE*);     //Initialize routine.
END_DEFINE_OBJECT()

BOOL NetBufferQueueInit(__NET_BUFFER_QUEUE*);    //Initialize routine of net buffer queue.

//
//The definition of net buffer header.
//This data structure is used by net buffer manager to link all free net buffers together.
//

BEGIN_DEFINE_OBJECT(__NET_BUFFER_HEADER)
    __NET_BUFFER*              lpNext;
    DWORD                      dwLinkLen;    //How many free net buffers in this link.
END_DEFINE_OBJECT()

//
//The definition of net buffer.
//

BEGIN_DEFINE_OBJECT(__NET_BUFFER)
    __NET_BUFFER*                       lpPrev;
    __NET_BUFFER*                       lpNext;

	__SOCKET*                           lpSocket;  //Socket this buffer belongs to.
	__TIME*                             Time;      //Time when is buffer is allocated.
	__NET_DEVICE*                       lpNetDevice;  //Net device this packet is received.

	union{
		__TCP_HDR*                      lpTcpHdr;
		__UDP_HDR*                      lpUdpHdr;
		__IGMP_HDR*                     lpIgmpHdr;
		__ICMP_HDR*                     lpIcmpHdr;
		__OSPF_HDR*                     lpOspfHdr;
		__IPIP_HDR*                     lpIpipHdr;
		__GRE_HDR*                      lpGreHdr;
		UCHAR*                          lpTransLayerData;
	}TransLayerHdr;  //Transparent layer header.

	union{
		__IP_HDR*                       lpIpHdr;
		__ARP_HDR*                      lpArpHdr;
		__IPV6_HDR*                     lpIpv6Hdr;
		__ISIS_HDR*                     lpIsisHdr;
		UCHAR*                          lpNetLayerData;
	}NetLayerHdr;   //Net layer header.

	union{
		UCHAR*                          lpLinkLayerData;
	}LinkLayerHdr;  //Link layer header.

	UCHAR                               ucParam[32];    //Private buffer.

	DWORD                               dwDataLen;      //Data length in this buffer.
	DWORD                               dwTotalLen;     //Total length,including fragment.

	DWORD                               dwCheckSum;     //Check sum of this packet.
	DWORD                               dwPktType;      //Packet type.Can be the following
	                                                    //values.
#define PACKET_TYPE_UNKNOWN             0x00000000
#define PACKET_TYPE_BROADCAST           0x00000001
#define PACKET_TYPE_MULTICAST           0x00000002
#define PACKET_TYPE_THISHOST            0x00000003
#define PACKET_TYPE_OTHERHOST           0x00000004
#define PACKET_TYPE_LOOPBACK            0x00000005
#define PACKET_TYPE_OUTGOING            0x00000006

	BOOL                                bCloned;        //If a cloned object.

	UCHAR                               ucProtocol;     //Protocol this packet belongs to.
	                                                    //Can be the following values.
#define PACKET_PROTOCOL_UNKNOWN         0x00
#define PACKET_PROTOCOL_IP              0x01
#define PACKET_PROTOCOL_ARP             0x02
#define PACKET_PROTOCOL_IPV6            0x03
#define PACKET_PROTOCOL_CLNS            0x04
#define PACKET_PROTOCOL_UDP             0x05
#define PACKET_PROTOCOL_TCP             0x06
#define PACKET_PROTOCOL_ICMP            0x07
#define PACKET_PROTOCOL_IGMP            0x08
#define PACKET_PROTOCOL_OSPF            0x09
#define PACKET_PROTOCOL_IPIP            0x0A
#define PACKET_PROTOCOL_GRE             0x0B
#define PACKET_PROTOCOL_RAWIP           0x0C

	UCHAR                               ucReserved1;    //Aligment.
	UCHAR                               ucReserved2;
	UCHAR                               ucReserved3;

	__ATOMIC_T                          Users;          //Reference counter of this buffer.
	UCHAR*                              lpBufferHdr;    //Pointing to data buffer header.
	UCHAR*                              lpBufferEnd;    //Pointing to data buffer tail.
	UCHAR*                              lpDataHdr;      //Pointing to data header.
	UCHAR*                              lpDataEnd;      //Pointing to data end.
END_DEFINE_OBJECT()    //End of net buffer.

//
//The following defines some macros to operate the net buffer object.
//

#define NET_BUFFER_DATA_LEN(netb)        ((netb)->dwDataLen)    //Get data length.
#define NET_BUFFER_BUFF_LEN(netb)        ((DWORD)((netb)->lpBufferEnd - (netb)->lpBufferHdr))
#define NET_BUFFER_TOTAL_LEN(netb)       ((netb)->dwTotalLen)   //Total length,including fragment.
#define NET_BUFFER_SHARED(netb)          (1 != AtomicGet(&(netb)->Users))
#define NET_BUFFER_HEADROOM(netb)        ((DWORD)((netb)->lpDataHdr - (netb)->lpBufferHdr))
#define NET_BUFFER_TAILROOM(netb)        ((DWORD)((netb)->lpBufferEnd - (netb)->lpDataEnd))

//
//The definiton of fragment data.
//This structure is used to manage fragment data.It resides the end of data buffer.
//

BEGIN_DEFINE_OBJECT(__FRAG_DATA)
    __NET_BUFFER_QUEUE           FragQueue;
    __ATOMIC_T                   SharedCounter;
END_DEFINE_OBJECT()

#define FRAG_DATA_SIZE                sizeof(__FRAG_DATA)     //Get frag data's size.
#define NET_BUFFER_FRAG_DATA(netb)    ((__FRAG_DATA*)(netb)->lpBufferEnd)  //Get the pointer.
#define NET_BUFFER_DATA_SHARED(netb)  \
	(1 != AtomicGet(&NET_BUFFER_FRAG_DATA(netb)->SharedCounter))  //If the data is shared.

//
//The definition of net buffer manager.
//This object is used to manage net buffer,such as allocating,freeing,etc.
//

BEGIN_DEFINE_OBJECT(__NET_BUFFER_MANAGER)
    __NET_BUFFER_HEADER                 FreeBufferLink;    //Used to manage the free net buff.
    BOOL                                (*Initialize)(__NET_BUFFER_MANAGER*);  //Initialize.
	__NET_BUFFER*                       (*NetBufferAlloc)(__NET_BUFFER_MANAGER*,
		                                                  DWORD);
	VOID                                (*NetBufferFreeData)(__NET_BUFFER_MANAGER*,
		                                                     __NET_BUFFER*);
	VOID                                (*NetBufferFree)(__NET_BUFFER_MANAGER*,
		                                                 __NET_BUFFER*);
	__NET_BUFFER*                       (*NetBufferGet)(__NET_BUFFER*);  //Increment reference.

	//
	//Data buffer operations.
	//
	UCHAR*                              (*NetBufferPull)(__NET_BUFFER*,INT);
	UCHAR*                              (*NetBufferPush)(__NET_BUFFER*,INT);
	UCHAR*                              (*NetBufferPut)(__NET_BUFFER*,INT);
	BOOL                                (*NetBufferReserve)(__NET_BUFFER*,INT);
	BOOL                                (*NetBufferStore)(__NET_BUFFER*,
		                                                  INT,
														  UCHAR*,
														  DWORD);
	BOOL                                (*NetBufferSave)(__NET_BUFFER*,
		                                                 INT,
														 UCHAR*,
														 DWORD);
	BOOL                                (*NetBufferExpand)(__NET_BUFFER*,
		                                                   DWORD,
														   DWORD);
	BOOL                                (*NetBufferAppendFrag)(__NET_BUFFER*,
		                                                       __NET_BUFFER*);
	__NET_BUFFER*                       (*NetBufferGetFrag)(__NET_BUFFER*);

	//
	//Copy operations.
	//

	__NET_BUFFER*                       (*NetBufferCopy)(__NET_BUFFER*);
	__NET_BUFFER*                       (*NetBufferSharedCopy)(__NET_BUFFER*);
	__NET_BUFFER*                       (*NetBufferPackCopy)(__NET_BUFFER_MANAGER*,
		                                                     __NET_BUFFER*);
	__NET_BUFFER*                       (*NetBufferClone)(__NET_BUFFER*);
END_DEFINE_OBJECT()

BOOL NetBufferManagerInit(__NET_BUFFER_MANAGER*);    //Initialize routine.

//
//The following macros are used by net buffer manager to allocate memory.
//Currently,we allocate data buffer and net buffer's memory from same memory pool,
//but in the future,we will create another memory pool,to be used by data buffer.
//

#define ALLOC_DATA_MEMORY(size)          (UCHAR*)KMemAlloc(size + FRAG_DATA_SIZE,KMEM_SIZE_TYPE_ANY)
#define FREE_DATA_MEMORY(ptr)            KMemFree((LPVOID)ptr,KMEM_SIZE_TYPE_ANY,0L)

#define ALLOC_BUFF_MEMORY(size)          (__NET_BUFFER*)KMemAlloc(size,KMEM_SIZE_TYPE_ANY)
#define FREE_BUFF_MEMORY(ptr)            KMemFree((LPVOID)ptr,KMEM_SIZE_TYPE_ANY,0L)

/***********************************************************************************
************************************************************************************
************************************************************************************
************************************************************************************
***********************************************************************************/

//
//The definition of global object : NetBufferManager.
//

extern __NET_BUFFER_MANAGER NetBufferManager;
