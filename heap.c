/*
 * File:   heap.c
 * Author: arkelee@gmail.com
 *
 * Created on 2010-12-4, 22:34
 */
#include "heap.h"
#include "heap_in.h"

/*
 * in       size(32 - 65535)
 * return   0...43
 * Two-level segregate value
 */
static tHeapLen Segregate(UINT size)
{
    static const unsigned char s_logTable[16] =
    {   0u, 1u, 2u, 2u, 3u, 3u, 3u, 3u,
        4u, 4u, 4u, 4u, 4u, 4u, 4u, 4u,    };

    UINT  size_l1, size_l2;

    /* first level */
    if (size > 0xFFFu)
        size_l1 = s_logTable[size >> 12] + 11u - HEAP_MIN_L2;
    else if (size > 0xFFu)
        size_l1 = s_logTable[size >>  8] +  7u - HEAP_MIN_L2;
    else
        size_l1 = s_logTable[size >>  4] +  3u - HEAP_MIN_L2;

    /* second level */
    {
        UINT shift = size_l1 + HEAP_MIN_L2 - HEAP_DIVS_L2;
        size_l2 = (size >> shift) & ((1u << HEAP_DIVS_L2) - 1u);
    }

    return (tHeapLen)((size_l1 << HEAP_DIVS_L2) + size_l2);
}


/*
 *
 *
 */
static void Block_Init(void *p_heap, tHeapPos pos, UINT blockLen)
{
    tBlockHdr *p_hdr = GET_BLOCK_HEADER(p_heap, pos);
    tBlockFtr *p_ftr = GET_BLOCK_FOOTER(p_heap, pos, blockLen);

    /* Get Header */
    p_hdr->m_blockLen = (tHeapLen)blockLen;
    p_hdr->m_userLen  = 0u;
    p_hdr->m_seg      = Segregate(blockLen);

    p_ftr->m_prv      = BLK_NULL;
    p_ftr->m_nxt      = BLK_NULL;
    p_ftr->m_blockLen = (tHeapLen)blockLen;
}

/*
 *
 *
 */
static void Block_Attach(void *p_heap, tHeapPos pos)
{
    tHeapInfo *p_inf = (tHeapInfo *)p_heap;
    tBlockHdr *p_hdr = GET_BLOCK_HEADER(p_heap, pos);
    tBlockFtr *p_ftr = GET_BLOCK_FOOTER(p_heap, pos, p_hdr->m_blockLen);

    UINT   seg = p_hdr->m_seg;
    tHeapPos old = p_inf->m_free[seg];

    /* Attach to the beginning of the free list */
    if (old != BLK_NULL)
    {
        tBlockHdr *p_old_hdr = GET_BLOCK_HEADER(p_heap, old);
        tBlockFtr *p_old_ftr = GET_BLOCK_FOOTER(p_heap, old, p_old_hdr->m_blockLen);

        p_old_ftr->m_prv = (tHeapPos)pos;
    }

    p_ftr->m_nxt = (tHeapPos)old;
    p_ftr->m_prv = BLK_NULL;
    p_inf->m_free[seg] = (tHeapPos)pos;
    p_hdr->m_userLen = 0u;
}

/*
 *
 *
 */
static void Block_Detach(void *p_heap, tHeapPos pos)
{
    tBlockHdr *p_hdr = GET_BLOCK_HEADER(p_heap, pos);
    tBlockFtr *p_ftr = GET_BLOCK_FOOTER(p_heap, pos, p_hdr->m_blockLen);

    tHeapPos prv = p_ftr->m_prv;
    tHeapPos nxt = p_ftr->m_nxt;

    if (prv == BLK_NULL)
    {
        tHeapInfo *p_inf = (tHeapInfo *)p_heap;
        p_inf->m_free[p_hdr->m_seg] = nxt;
    } else {
        tBlockHdr *p_prv_hdr = GET_BLOCK_HEADER(p_heap, prv);
        tBlockFtr *p_prv_ftr = GET_BLOCK_FOOTER(p_heap, prv, p_prv_hdr->m_blockLen);
        p_prv_ftr ->m_nxt = nxt;
    }

    if (nxt != BLK_NULL)
    {
        tBlockHdr *p_nxt_hdr = GET_BLOCK_HEADER(p_heap, nxt);
        tBlockFtr *p_nxt_ftr = GET_BLOCK_FOOTER(p_heap, nxt, p_nxt_hdr->m_blockLen);
        p_nxt_ftr ->m_prv = prv;
    }

    p_ftr->m_prv = BLK_NULL;
    p_ftr->m_nxt = BLK_NULL;
}

/*
 *
 *
 */
static void Block_WriteGuard(void *p_heap, tHeapPos pos)
{
    tBlockHdr *p_hdr = GET_BLOCK_HEADER(p_heap, pos);
    tBlockFtr *p_ftr = GET_BLOCK_FOOTER(p_heap, pos, p_hdr->m_blockLen);

    UINT guard_end = pos + sizeof(tBlockHdr) + p_hdr->m_userLen;

    p_hdr->m_guard = 0xDEAD;
    ((char *)p_heap)[guard_end] = (char)0xDE;
    ((char *)p_heap)[guard_end + 1u] = (char)0xAD;
}

/*
 *
 *
 */
int Block_TestGuard(void *p_heap, tHeapPos pos)
{
    tBlockHdr *p_hdr = GET_BLOCK_HEADER(p_heap, pos);
    tBlockFtr *p_ftr = GET_BLOCK_FOOTER(p_heap, pos, p_hdr->m_blockLen);

    UINT guard_end = pos + sizeof(tBlockHdr) + p_hdr->m_userLen;

    return (p_hdr->m_guard == 0xDEAD &&
            ((char *)p_heap)[guard_end] == 0xDE &&
            ((char *)p_heap)[guard_end + 1u] == 0xAD);
}

/*
 *
 *
 */
void *Heap_Create(tHeapCallback p_callback, void *ptr, UINT len)
{
    tHeapInfo *p_inf = ptr;
    void *p_heap = ptr;
    UINT area = len - sizeof(tHeapInfo);

    //assart(area >  HEAP_MIN);
    //assart(area <= HEAP_MAX);

    CS_ENTER(g_heap_cs);
    
    /* init heap header */
    p_inf->m_callback = p_callback;
    p_inf->m_heapSize = len;
    p_inf->m_maxSeg   = Segregate(area);

    for (UINT i = 0u; i < SEG_POOLS; ++i)
        p_inf->m_free[i] = BLK_NULL;

    /* init heap blokcs */
    Block_Init(p_heap, sizeof(tHeapInfo), area);
    Block_Attach(p_heap, sizeof(tHeapInfo));

    CS_EXIT(g_heap_cs);
    
#ifdef HEAP_WALK
    /* Verify heap intergrity */
    if (Heap_Walk(p_heap) != HE_NOERROR)
        p_callback();
#endif

    return p_heap;
}


/*
 *
 *
 */
void Heap_Open()
{
    CS_INIT(g_heap_cs);
}

/*
 *
 *
 */
void Heap_Close()
{
    CS_DELETE(g_heap_cs);
}


/*
 *
 *
 */
void *Heap_Malloc(void *p_heap, UINT userLen)
{
    tBlockHdr *p_hdr;
    UINT  blockLen, pool, seg, pos;
    tHeapInfo *p_inf = (tHeapInfo *)p_heap;

    if(userLen == 0u)
        userLen = 1u;

    blockLen = GET_BLOCK_LEN(userLen);

    /* Heap_Walk_Heap */

    if (blockLen > HEAP_MAX)
    {
        if (p_inf->m_callback)
            p_inf->m_callback(p_heap, HE_BadRequest, blockLen);
        
        return NULL;
    }

    if (blockLen < HEAP_MIN)
    {
        blockLen = HEAP_MIN;
    }

    seg = Segregate(blockLen);

    CS_ENTER(g_heap_cs);

    pos = p_inf->m_free[seg];
    while (pos != BLK_NULL)
    {
        tBlockFtr *p_ftr;
        p_hdr = GET_BLOCK_HEADER(p_heap, pos);
        if (IS_BLK_FREE(p_hdr) && p_hdr->m_blockLen >= blockLen)
            break;
        p_ftr = GET_BLOCK_FOOTER(p_heap, pos, p_hdr->m_blockLen);
        pos = p_ftr->m_nxt;
    }

    if (pos == BLK_NULL)
    {
        pool = (seg == p_inf->m_maxSeg) ? seg : seg + 1u;
        for (; pool < SEG_POOLS; pool++)
        {
            if (p_inf->m_free[pool] != BLK_NULL)
                break;
        }
        if (pool == SEG_POOLS)
        {
            CS_ABORT(g_heap_cs);
            if (p_inf->m_callback)
                p_inf->m_callback(p_heap, HE_OutofMemory, blockLen);

            return NULL;
        }
        pos = p_inf->m_free[pool];
    }

    if (pos == BLK_NULL)
    {
        CS_ABORT(g_heap_cs);
        if (p_inf->m_callback)
            p_inf->m_callback(p_heap, HE_OutofMemory, blockLen);

        return NULL;
    }

    p_hdr = GET_BLOCK_HEADER(p_heap, pos);
    if (p_hdr->m_blockLen < blockLen)
    {
        CS_ABORT(g_heap_cs);
        if (p_inf->m_callback)
            p_inf->m_callback(p_heap, HE_OutofMemory, blockLen);

        return NULL;
    }

    Block_Detach(p_heap, pos);

    if (p_hdr->m_blockLen > blockLen + SPLIT_THRESH)
    {
        UINT pos2 = pos + blockLen;
        UINT len2 = p_hdr->m_blockLen - blockLen;

        /* Init the two blocks and free the second */
        Block_Init(p_heap, pos, blockLen);
        Block_Init(p_heap, pos2, len2);
        Block_Attach(p_heap, pos2);
    }

    p_hdr->m_userLen = userLen;
    
#if HEAP_USE_GUARDWORDS
    Block_WriteGuard(p_heap, pos);
#endif

    CS_EXIT(g_heap_cs);

#ifdef HEAP_WALK
/* Verify heap intergrity */
if (Heap_Walk(p_heap) != HE_NOERROR)
    p_callback();
#endif

    return p_heap + pos + sizeof(tBlockHdr);
    
}


/*
 *
 *
 */
void Heap_Free(void *p_heap, void *ptr)
{
    tHeapInfo *p_inf = (tHeapInfo *)p_heap;
    UINT pos = (UINT)(((void *)ptr - p_heap) - sizeof(tBlockHdr));
    tBlockHdr *p_hdr = GET_BLOCK_HEADER(p_heap, pos);
    UINT blockLen = p_hdr->m_blockLen;

    if (pos >= p_inf->m_heapSize || IS_BLK_FREE(p_hdr))
    {
        if (p_inf->m_callback)
            p_inf->m_callback(p_heap, HE_BadFree, (UINT)ptr);

        return;
    }

#ifdef HEAP_WALK
/* Verify heap intergrity */
if (Heap_Walk(p_heap) != HE_NOERROR)
{
    p_callback();
    return;
}
#endif
    
#if HEAP_USE_GUARDWORDS
    if (Block_TestGuard(p_heap, pos) == 0)
    {
        if (p_inf->m_callback)
            p_inf->m_callback(p_heap, HE_BlockCorrupted, pos);

        return;
    }
#endif

    CS_ENTER(g_heap_cs);

    /* Merge previous */
    if (pos > sizeof(tHeapInfo))
    {
        tBlockFtr *p_prv_ftr = GET_BLOCK_FOOTER(p_heap, pos, 0u);
        UINT prv_pos = pos - p_prv_ftr->m_blockLen;
        tBlockHdr *p_prv_hdr = GET_BLOCK_HEADER(p_heap, prv_pos);
        if (IS_BLK_FREE(p_prv_hdr))
        {
            Block_Detach(p_heap, prv_pos);
            pos = prv_pos;
            blockLen += p_prv_ftr->m_blockLen;
        }
    }

    /* Merge next */
    if (pos + blockLen < p_inf->m_heapSize)
    {
        UINT nxt_pos = pos + blockLen;
        tBlockHdr *p_nxt_hdr = GET_BLOCK_HEADER(p_heap, nxt_pos);
        if (IS_BLK_FREE(p_nxt_hdr))
        {
            Block_Detach(p_heap, nxt_pos);
            blockLen += p_nxt_hdr->m_blockLen;
        }
    }

    /* Reinit new block */
    if (blockLen != p_hdr->m_blockLen)
        Block_Init(p_heap, pos, blockLen);

    Block_Attach(p_heap, pos);

    CS_EXIT(g_heap_cs);
    
#ifdef HEAP_WALK
/* Verify heap intergrity */
if (Heap_Walk(p_heap) != HE_NOERROR)
{
    p_callback();
    return;
}
#endif

}
