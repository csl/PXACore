//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Sep,02 2005
//    Module Name               : NET_COMM.H
//    Module Funciton           : 
//                                This file contains pre-definitions of network module.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __NET_COMM_H__
#define __NET_COMM_H__

u16_t htons(u16_t);                    //Convert a 16 bit word in host sequence to network
                                       //sequence.
u16_t ntohs(u16_t);                    //Convert a 16 bit word in network sequence to host
                                       //sequence.
u32_t htonl(u32_t);                    //32 bit convert.
u32_t ntohl(u32_t);                    //32 bit convert.

//
//The following routines used to calculate checksum.
//

u16_t checksum(void* dataptr,int len);

//
//The definition of common usage structures and types.
//

//
//This structure is used to store link layer statistics data.
//
BEGIN_DEFINE_OBJECT(__LINK_LAYER_STAT)
    u32_t           ulTxFrames;
    u32_t           ulRxFrames;
	u32_t           ulTxBytes;
	u32_t           ulRxBytes;
	u32_t           ulRxCrcErrors;
	u32_t           ulTxNobuffers;
END_DEFINE_OBJECT()  //__LINK_LAYER_STAT.

//
//The following structure is used to store network layer statistics.
//
BEGIN_DEFINE_OBJECT(__NET_LAYER_STAT)
    u32_t           ulTxPackets;
    u32_t           ulRxPackets;
	u32_t           ulTxBytes;
	u32_t           ulRxBytes;

	u32_t           ulRxCrcErrors;
	u32_t           ulRxNoBuffers;
	u32_t           ulSendErrors;

	u32_t           ulRxUnicasts;    //Unicast packets received.
	u32_t           ulTxUnicasts;    //Unicast packets send.
	u32_t           ulRxMulticasts;  //Multicast packets received.
	u32_t           ulTxMulticasts;  //Multicast packets send.
	u32_t           ulRxBroadcasts;  //Broadcast packet received.
	u32_t           ulTxBroadcasts;  //Broadcast packet send.
END_DEFINE_OBJECT()   //__NET_LAYER_STAT.

//
//Constants definition.
//

//
//Protocol family's definiton.
//
#define PROTOCOL_FAMILY_IPV4       0x0001    //Protocol family for IPv4.
#define PROTOCOL_FAMILY_IPV6       0x0002    //IPv6
#define PROTOCOL_FAMILY_CLNS       0x0003    //ISO CLNS.
#define PROTOCOL_FAMILY_IPX        0x0004    //IPX.
#define PROTOCOL_FAMILY_APPLE      0x0005    //Apple talk.
#define PROTOCOL_FAMILY_DCENET     0x0006    //DEC Net.
#define PROTOCOL_FAMILY_ATM        0x0007    //ATM.

//
//Address type's defintion.
//
#define ADDRESS_TYPE_UNICAST       0x0001    //Address is unicast.
#define ADDRESS_TYPE_BROADCAST     0x0002    //Broadcast.
#define ADDRESS_TYPE_MULTICAST     0x0004    //Multicast.
#define ADDRESS_TYPE_NETWORK       0x0008    //Network layer address.
#define ADDRESS_TYPE_LINK          0x0010    //Link layer address,such as MAC.

//
//Common network interface's address.
//Any kind of address,no matter IP,IPv6,or IPX,all use this structure to
//save address information.
//For MAC layer address,this structure also be used to save address information.
//
BEGIN_DEFINE_OBJECT(__COMMON_ADDRESS)
    __COMMON_ADDRESS*        lpNext;        //Pointing to next address.
    __COMMON_ADDRESS*        lpPrev;        //Pointing to previous addr.

    u16_t           usProtocolFamily;       //Protocol family.
    u16_t           usAddressType;          //Which kind of address.
	u16_t           usAddrressLen;          //The length of address.
	u16_t           usReserved;             //Aligment.
	union{
		struct{
			u32_t   Address;
			u32_t   Mask;
		}Ipv4Addr;  //Ipv4 address.
		struct{
			u32_t   Address[4];
			u32_t   Mask   [4];
		}Ipv6Addr;  //Ipv6 address.
		struct{
			u8_t    Address[6];
			u8_t    Reserved[2];
		}MacAddr;   //Ethernet MAC address.
		struct{
			u32_t   Address;
			u32_t   Mask;
		}IpxAddr;   //IPX address.
		u32_t       Address[8];
	}Address;
END_DEFINE_OBJECT()  //__COMMON_ADDRESS.

#endif  //NET_COMM.H

