/***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct,22 2006
//    Module Name               : L_STDIO.H
//    Module Funciton           : 
//                                Standard I/O libary header file.
//                                Please note it's name is a prefix "L_".
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#define NUL	0x00 
#define SOH	0x01 
#define STX	0x02 
#define ETX	0x03 
#define EOT	0x04 
#define ENQ	0x05 
#define ACK	0x06 
#define BEL	0x07 
#define BS	0x08 
#define HT	0x09 
#define LF	0x0a 
#define VT	0x0b 
#define FF	0x0c 
#define CR	0x0d 
#define SO	0x0e 
#define SI	0x0f 
#define DLE	0x10 
#define DC1	0x11 
#define DC2	0x12 
#define DC3	0x13 
#define DC4	0x14 
#define NAK	0x15 
#define SYN	0x16 
#define ETB	0x17 
#define CAN	0x18 
#define EM	0x19 
#define SUB	0x1a 
#define ESC	0x1b 
#define FS	0x1c 
#define GS	0x1d 
#define RS	0x1e 
#define US	0x1f 
#define DEL	0x7f

typedef char *  va_list;

#define _INTSIZEOF(n)   ( (sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1) )

#define va_start(ap,v)  ( ap = (va_list)&v + _INTSIZEOF(v) )
#define va_arg(ap,t)    ( *(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)) )
#define va_end(ap)      ( ap = (va_list)0 )

#define MAX_BUFFER_SIZE 512

void *memcpy(void *dest, const void *src, int len);
void *memset(void *dest, int c, int len);

int printf(const char *fmt, ...);
int getc(void);
int gets(char *s);

int putchar(int c);
int puts(const char *s);

int strcmp(char *s1, char *s2);
static int strlen(const char* s);

char *strtok(char * s,const char * ct);
char *strpbrk(const char * cs,const char * ct);
unsigned int strspn(const char *s, const char *accept);
unsigned long strtoul(const char *str, char **endptr, int requestedbase);

/* Byte swapping */

#define SWAP8(X) 	(X)
#define SWAP16(X) 	((((X) & 0x00ff) << 8) | \
			 ((X) >> 8))
#define SWAP32(X) 	((((X) & 0x000000ff) << 24) | \
			 (((X) & 0x0000ff00) << 8) | \
			 (((X) & 0x00ff0000) >> 8) | \
			 (((X) & 0xff000000) >> 24))
