/*
 * File:   heap_debug.c
 * Author: arkelee@gmail.com
 *
 * Created on 2010-12-4, 22:34
 */
#include <stdio.h>
#include "heap.h"
#include "heap_in.h"

void Heap_Print(void *p_heap)
{
    tBlockHdr *p_hdr;
    tBlockFtr *p_ftr;
    UINT blockLen, pool, seg, pos;
    UINT total_user = 0;
    UINT total_free = 0;
    tHeapInfo *p_inf = (tHeapInfo *)p_heap;

 
    /* Print Heap Header*/
    printf("Heap Size: %d(0x%X)\n", p_inf->m_heapSize, p_inf->m_heapSize);
    printf("Heap MaxSeg: %d\n", p_inf->m_maxSeg);

    printf("Blocks in free-chain:\n");
    for (tHeapLen i = 0u; i <= p_inf->m_maxSeg; ++i)
    {
        if (p_inf->m_free[i] != BLK_NULL) {
            printf(" |-->[%02d]:%04X\n", i, p_inf->m_free[i]);
            
            pos = p_inf->m_free[i];
            while (pos != BLK_NULL)
            {
                p_hdr = GET_BLOCK_HEADER(p_heap, pos);
                p_ftr = GET_BLOCK_FOOTER(p_heap, pos, p_hdr->m_blockLen);

                printf("  |-->Offset:0x%04X, BlockLen:0x%04X, UserLen:0x%04X\n",
                        pos, p_hdr->m_blockLen, p_hdr->m_userLen);
                pos = p_ftr->m_nxt;
                total_free += p_hdr->m_blockLen - sizeof(tBlockHdr) - sizeof(tBlockFtr);
            }
        }
    }

    printf("Blocks consecutively:\n");
    pos = sizeof(tHeapInfo);
    while (pos < p_inf->m_heapSize - sizeof(tHeapInfo))
    {
        p_hdr = GET_BLOCK_HEADER(p_heap, pos);
        p_ftr = GET_BLOCK_FOOTER(p_heap, pos, p_hdr->m_blockLen);

        printf(" |-->Offset:0x%04X, BlockLen:0x%04X, UserLen:0x%04X\n",
                pos, p_hdr->m_blockLen, p_hdr->m_userLen);
        pos += p_hdr->m_blockLen;
        total_user +=  p_hdr->m_userLen;
    }

    printf("Heap Free:%d(0x%04X) %2.1f%%, User:%d(0x%04X) %2.1f%%\n",
            total_free, total_free, ((float)total_free * 100.0f) / ((float)p_inf->m_heapSize),
            total_user, total_user, ((float)total_user * 100.0f) / ((float)p_inf->m_heapSize));
}