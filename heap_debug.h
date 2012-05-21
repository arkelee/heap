/* 
 * File:   heap_debug.h
 * Author: arkelee@gmail.com
 *
 * Created on 2010-12-5, 2:22
 */

#ifndef HEAP_DEBUG_H
#define	HEAP_DEBUG_H

#ifdef	__cplusplus
extern "C" {
#endif

#ifndef printf
#include <stdio.h>
#endif
    
//#define PRINTF printf
extern void Heap_Print(void *p_heap);

#ifdef	__cplusplus
}
#endif

#endif	/* HEAP_DEBUG_H */

