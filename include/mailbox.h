#ifndef __MAILBOX_H__
#define __MAILBOX_H__
#endif

BEGIN_DEFINE_OBJECT(__MAILBOX)
    INHERIT_FROM_COMMON_OBJECT
	INHERIT_FROM_COMMON_SYNCHRONIZATION_OBJECT
	struct __PRIORITY_QUEUE*                  lpGettingQueue;
    	struct __PRIORITY_QUEUE*                  lpSendingQueue;
    	LPVOID                             lpMsg;
	DWORD                              (*GetMail)(struct __COMMON_OBJECT*,LPVOID*,DWORD);
	DWORD                              (*SendMail)(struct __COMMON_OBJECT*,LPVOID,DWORD);
END_DEFINE_OBJECT()

#define MAILBOX_SUCCESS         0xFFFFFFFF
#define MAILBOX_FAILED          0x00000000
#define MAILBOX_FULL            0x00000001
#define MAILBOX_TIMEOUT         0x00000002
#define MAILBOX_NO_RESOURCE     0x00000003

BOOL MailBoxInitialize(struct __COMMON_OBJECT*);
VOID MailBoxUninitialize(struct __COMMON_OBJECT*);
