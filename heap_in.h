/* 
 * File:   heap_in.h
 * Author: arkelee@gmail.com
 *
 * Created on 2010-12-4, 22:38
 */

#ifndef HEAP_IN_H
#define	HEAP_IN_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "heap.h"

    
#define    HEAP_MIN_L2        (5u)
#define    HEAP_MAX_L2       (15u)
#define    HEAP_DIVS_L2       (2u)

#define    SEG_POOLS         ((HEAP_MAX_L2 - HEAP_MIN_L2 + 1u) << HEAP_DIVS_L2)
#define    HEAP_MIN          (1u << HEAP_MIN_L2)
#define    HEAP_MAX          ((1u << HEAP_MAX_L2 + 1u) - 1u)

#define    SPLIT_THRESH      (64u)

#define    GET_BLOCK_HEADER(p_heap, pos)    ((tBlockHdr *)(p_heap + pos))
#define    GET_BLOCK_FOOTER(p_heap, pos, blockLen)  \
                    ((tBlockFtr *)(p_heap + pos + blockLen - sizeof(tBlockFtr)))

#define    GET_BLOCK_LEN(userLen) \
                (((userLen) + sizeof(tBlockHdr) + sizeof(tBlockFtr) + 7u) & ~7u)

#define    BLK_NULL          ((tHeapPos)(0u))
#define    IS_BLK_FREE(hdr)  ((hdr)->m_userLen == 0u)

#define    CS_INIT(g_semphore)    {}
#define    CS_ENTER(g_semphore)   {}
#define    CS_EXIT(g_semphore)    {}
#define    CS_DELETE(g_semphore)  {}
#define    CS_ABORT(g_semphore)   {}

typedef UINT16  tHeapPos;
typedef UINT16  tHeapLen;

typedef struct {
    tHeapCallback    m_callback;
    tHeapLen         m_heapSize;
    tHeapLen         m_maxSeg;
    tHeapPos         m_free[SEG_POOLS];
} tHeapInfo;

typedef struct {
    tHeapLen        m_blockLen;
    tHeapLen        m_userLen;
    tHeapLen        m_seg;
    UINT16          m_guard;
} tBlockHdr;

typedef struct {
    UINT16          m_guard;
    tHeapPos        m_prv;
    tHeapPos        m_nxt;
    tHeapPos        m_blockLen;
} tBlockFtr;



// typedef void (*tHeapCallback)(void *, HE_ERROR, UINT);

#ifdef	__cplusplus
}
#endif

#endif	/* HEAP_IN_H */

