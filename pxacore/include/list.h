//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Aug,18 2005
//    Module Name               : LIST.H
//    Module Funciton           : 
//                                This module countains bi-direction list's definition and
//                                implementation code.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __LIST_H__
#define __LIST_H__

BEGIN_DEFINE_OBJECT(__LIST_NODE)
    __LIST_NODE*    lpPrev;
    __LIST_NODE*    lpNext;
END_DEFINE_OBJECT()

//
//Initializes a list node,to form a ring list.
//
#define LIST_INIT(node) (node).lpPrev = &(node); \
    (node).lpNext = &(node);

//
//Check if the list node is an orphan,e.g.,it is the only
//element in the list.
//
#define LIST_NULL_NODE(nodeptr) \
    (((nodeptr) == (nodeptr)->lpNext) && ((nodeptr) == (nodeptr)->lpPrev))

//
//Add a list node in front of the list,as a stack.
//
__INLINE VOID ListAddHeader(__LIST_NODE* lpHdr,__LIST_NODE* lpEntry)
{
    if((NULL == lpHdr) || (NULL == lpEntry))  //Parameters check.
        return;
    lpEntry->lpNext = lpHdr->lpNext;
    lpEntry->lpPrev = lpHdr;
    lpHdr->lpNext->lpPrev = lpEntry;
    lpHdr->lpNext         = lpEntry;
}

//
//Add a list node at the tail of the list,like a queue.
//
__INLINE VOID ListAddTail(__LIST_NODE* lpHdr,__LIST_NODE* lpEntry)
{
    if((NULL == lpHdr) || (NULL == lpEntry)) //Parameters check.
        return;
    lpEntry->lpNext = lpHdr;
    lpEntry->lpPrev = lpHdr->lpPrev;
    lpHdr->lpPrev->lpNext = lpEntry;
    lpHdr->lpPrev         = lpEntry;
}

//
//Delete the node designated by lpEntry from list.
//
__INLINE VOID ListDelete(__LIST_NODE* lpEntry)
{
    if(NULL == lpEntry)  //Parameter check.
        return;
    if(LIST_NULL_NODE(lpEntry))  //This is a orphan node,e.g,this is the list header node.
        return;
    lpEntry->lpPrev->lpNext = lpEntry->lpNext;
    lpEntry->lpNext->lpPrev = lpEntry->lpPrev;
    LIST_INIT(*lpEntry);  //Initializes the deleted node to orphan node.
}

//
//Replace the old node with the new one.
//
__INLINE VOID ListReplace(__LIST_NODE* lpOld,__LIST_NODE* lpNew)
{
    if((NULL == lpOld) || (NULL == lpNew))  //Parameters check.
        return;
    lpNew->lpPrev = lpOld->lpPrev;
    lpNew->lpNext = lpOld->lpNext;
    lpOld->lpNext->lpPrev = lpNew;
    lpOld->lpPrev->lpNext = lpNew;
    LIST_INIT(*lpOld);    //Initializes the old one to an orphan.
}

//
//Get the first node,assume the lpHdr is the list's header.
//
__INLINE __LIST_NODE* ListGetHeader(__LIST_NODE* lpHdr)
{
    __LIST_NODE*    lpFirst = NULL;

    if(NULL == lpHdr)    //Parameter check.
        return NULL;
    if(LIST_NULL_NODE(lpHdr)) //The list is NULL.
        return NULL;
    lpFirst = lpHdr->lpNext;
    ListDelete(lpFirst);     //Delete this node from list.
    return lpFirst;
}

//
//Get the tail node,assume the lpHdr is the list's header.
//
__INLINE __LIST_NODE* ListGetTail(__LIST_NODE* lpHdr)
{
    __LIST_NODE*    lpLast  = NULL;

    if(NULL == lpHdr)    //Parameter check.
        return NULL;
    if(LIST_NULL_NODE(lpHdr)) //The list is NULL.
        return NULL;
    lpLast = lpHdr->lpPrev;
    ListDelete(lpLast);     //Delete this node from list.
    return lpLast;
}

//
//Get a member's offset of a object type.
//
#ifdef __I386
#define OFFSET_OF(type,member) \
    (DWORD)((DWORD)(&((type*)0x80000000)->(member)) - (DWORD)((type*)0x80000000))
#else
#define OFFSET_OF(type,member)
#endif

//
//The LIST_ENTRY macro gets the object associated with a list node.
//
#define LIST_ENTRY(nodeptr,type,member) \
    (type*)((DWORD)(nodeptr) - OFFSET_OF(type,member))

//
//This macro is used to iterate a list forward.
// pos   : __LIST_NODE* structure used as loop counter.
// lphdr : The target list's header element.
//
#define LIST_FOR_EACH(pos,lphdr) \
    for(pos = (lphdr)->lpNext;pos != (lphdr);pos = pos->lpNext)

//
//The macro is used to iterate a list backword.
// pos   : __LIST_NODE* structure used as loop counter.
// lphdr : List's header element,a pointer.
//
#define LIST_FOR_EACH_PREV(pos,lphdr) \
    for(pos = (lphdr)->lpPrev;pos != (lpHdr);pop = pos->lpPrev)

#endif    //End of LIST.H
