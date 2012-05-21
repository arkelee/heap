/* 
 * File:   heap_test.c
 * Author: arkelee@gmail.com
 *
 * Created on 2010-12-4, 22:34
 */

#include <stdio.h>
#include <stdlib.h>

#include "heap.h"
#include "heap_debug.h"

/*
 * 
 */
#define BUFLEN 65535
char buf[BUFLEN];

void heap_test(void)
{
    void *p_heap, *p1, *p2 ,*p3, *p4;

    p_heap = Heap_Create(NULL, (void *)buf, BUFLEN);

    p1 = Heap_Malloc(p_heap, 0x100);
    p2 = Heap_Malloc(p_heap, 0x1000);
    p3 = Heap_Malloc(p_heap, 0x2000);
    p4 = Heap_Malloc(p_heap, 0x1000);
    Heap_Malloc(p_heap, 0x100);
    Heap_Malloc(p_heap, 0x1200);
    Heap_Malloc(p_heap, 0x100);
    Heap_Malloc(p_heap, 0x1200);
    Heap_Malloc(p_heap, 0x100);
    Heap_Malloc(p_heap, 0x1200);
    Heap_Malloc(p_heap, 0x100);
    Heap_Malloc(p_heap, 0x1200);
    Heap_Malloc(p_heap, 0x100);
    Heap_Malloc(p_heap, 0x1200);
    Heap_Free(p_heap, p2);
    Heap_Free(p_heap, p3);
    Heap_Free(p_heap, p4);

    Heap_Print(p_heap);

    Heap_Free(p_heap, p1);

}

int main(int argc, char** argv)
{
    heap_test();
    
    return (0);
}