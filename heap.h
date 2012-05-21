/* 
 * File:   heap.h
 * Author: arkelee@gmail.com
 *
 * Created on 2010-12-4, 22:40
 */

#ifndef HEAP_H
#define	HEAP_H

#ifdef	__cplusplus
extern "C" {
#endif
    
typedef enum {
    HE_NOERROR,
    HE_BadRequest,
    HE_OutofMemory,
    HE_BadFree,
    HE_BlockCorrupted
} HE_MSG;

typedef unsigned int UINT;
typedef unsigned short UINT16;
//typedef unsigned short size_t;

#ifndef    NULL
#define    NULL               ((void *)0)
#endif

typedef void (*tHeapCallback)(void *, HE_MSG, UINT);

extern void *Heap_Create(tHeapCallback p_callback, void *ptr, UINT len);
extern void *Heap_Malloc(void *p_heap, UINT userLen);
extern void Heap_Free(void *p_heap, void *ptr);


#ifdef	__cplusplus
}
#endif

#endif	/* HEAP_H */

