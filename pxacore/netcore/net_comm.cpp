/***********************************************************************/
//    Author                    : Garry
//    Original Date             : Sep,06 2005
//    Module Name               : NET_COMM.CPP
//    Module Funciton           : 
//                                This module countains common function implementation code
//                                for network module.
//                                Some code in this module is copied from Light Weight IP
//                                stack source code,which is a simple free source IP stack.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __NETSTD_H__
#include "NetStd.h"
#endif

//
//The implementation of convert routines.
//These routines are used to convert a word,or a double word in host sequence into
//network sequence,or reverse.
//

#ifdef LITTLE_ENDIAN    //The endian machine's implementation.

u16_t htons(u16_t n)
{
	return ((n & 0xff) << 8) | ((n & 0xff00) >> 8);
}

u16_t ntohs(u16_t n)
{
	return htons(n);
}

u32_t htonl(u32_t n)
{
	return ((n & 0xff) << 24) |
    ((n & 0xff00) << 8) |
    ((n & 0xff0000) >> 8) |
    ((n & 0xff000000) >> 24);
}

u32_t ntohl(u32_t n)
{
	return ((n & 0xff) << 24) |
    ((n & 0xff00) << 8) |
    ((n & 0xff0000) >> 8) |
    ((n & 0xff000000) >> 24);
}

#else  //BIG_ENDIAN,should do nothing.

#define htons(n)
#define ntohs(n)
#define htonl(n)
#define ntohl(n)

#endif

//
//The implementation of check routine.
//

u16_t checksum(void* dataptr,int len)
{
	u32_t acc;

	for(acc = 0; len > 1; len -= 2) {
		/*    acc = acc + *((u16_t *)dataptr)++;*/
		acc += *(u16_t *)dataptr;
		dataptr = (void *)((u16_t *)dataptr + 1);
	}

  /* add up any odd byte */
	if (len == 1) {
		acc += htons((u16_t)((*(u8_t *)dataptr) & 0xff) << 8);
	} else {
	}
	acc = (acc >> 16) + (acc & 0xffffUL);
	if ((acc & 0xffff0000) != 0) {
		acc = (acc >> 16) + (acc & 0xffff);
	}

	return (u16_t)acc;
}

//
//The following routine is used to calculate IP header's check sum,or ICMP header's check
//sum.
//
u16_t    inet_chksum(void *dataptr, u16_t len)
{
  u32_t acc;
  acc = checksum(dataptr, len);
  while (acc >> 16) {
    acc = (acc & 0xffff) + (acc >> 16);
  }
  return (u16_t)~(acc & 0xffff);
}